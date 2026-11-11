#include "aether/mac/controller.hpp"

#include <algorithm>
#include <limits>
#include <set>
#include <stdexcept>

namespace aether::mac {
unsigned MacController::index(Direction direction) {
  switch (direction) {
    case Direction::downlink:
      return 0;
    case Direction::uplink:
      return 1;
  }
  throw std::invalid_argument("invalid direction");
}
MacController::MacController(Policy policy, ControllerConfig config)
    : config_(config), planes_{Plane(policy), Plane(policy)} {
  if (!config.max_outstanding_bytes) throw std::invalid_argument("zero queue bound");
  const HarqManager validate(config.harq);
}
void MacController::add(UeId id) {
  // Stage both namespaces so invalid IDs or allocation errors cannot split registration.
  auto next = planes_;
  for (auto& plane : next) {
    plane.scheduler.add({id});
    plane.links.try_emplace(id, config_.harq);
  }
  planes_.swap(next);
}
void MacController::configure(Direction direction, UeId id, unsigned quality, unsigned priority,
                              bool active) {
  auto& plane = planes_[index(direction)];
  auto& link = plane.links.at(id);
  plane.scheduler.configure(id, quality, priority, active);
  link.quality = quality;
  link.priority = priority;
  link.active = active;
}
std::uint64_t MacController::slot() const {
  if (!slot_) throw std::logic_error("begin_slot must be called first");
  return *slot_;
}
void MacController::begin_slot(std::uint64_t next) {
  if (slot_ && next <= *slot_) throw std::invalid_argument("slot must increase");
  if (next > std::numeric_limits<std::uint64_t>::max() - config_.harq.timeout_slots)
    throw std::overflow_error("slot leaves no room for HARQ deadline");
  slot_ = next;
  for (auto& plane : planes_) {
    plane.scheduled = false;
    for (auto& [id, link] : plane.links) link.failed += link.harq.expire(next);
  }
}
void MacController::enqueue_downlink(UeId id, std::uint64_t bytes) {
  auto& plane = planes_[0];
  auto& link = plane.links.at(id);
  const auto outstanding = link.admitted - link.delivered - link.failed;
  if (bytes > config_.max_outstanding_bytes - outstanding)
    throw std::overflow_error("DL outstanding byte bound");
  plane.scheduler.enqueue(id, bytes);  // Checks cumulative uint64 overflow.
  link.admitted += bytes;
}
bool MacController::fresh(const Link& link) const {
  return link.report && slot() - link.report->generated_slot <= config_.bsr_max_age;
}
void MacController::report_uplink(UeId id, BufferStatusReport report) {
  const auto now = slot();
  auto& plane = planes_[1];
  auto& link = plane.links.at(id);
  const auto retired = link.delivered + link.failed;
  if (!report.sequence || report.generated_slot > now ||
      now - report.generated_slot > config_.bsr_max_age)
    throw std::invalid_argument("invalid BSR timestamp or sequence");
  if (link.report && (report.sequence <= link.report->sequence ||
                      report.generated_slot < link.report->generated_slot ||
                      report.retired < link.report->retired))
    throw std::invalid_argument("stale BSR");
  if (report.admitted < link.admitted || report.retired > retired ||
      report.retired > report.admitted || report.outstanding != report.admitted - report.retired)
    throw std::invalid_argument("inconsistent BSR counters");
  if (report.outstanding > config_.max_outstanding_bytes ||
      report.admitted - retired > config_.max_outstanding_bytes)
    throw std::overflow_error("UL outstanding byte bound");
  // Only arrivals not seen before enter the queue. Pending grants remain reserved,
  // even when this report was generated before those grants or their feedback.
  plane.scheduler.enqueue(id, report.admitted - link.admitted);
  link.admitted = report.admitted;
  link.report = report;
}
std::vector<Transmission> MacController::schedule(Direction direction, unsigned prbs) {
  const auto now = slot();
  const auto d = index(direction);
  if (prbs > 275) throw std::invalid_argument("invalid PRB budget");
  if (planes_[d].scheduled) throw std::logic_error("direction already scheduled this slot");
  auto next = planes_[d];  // Transactional commit, including allocation-failure safety.
  std::vector<Transmission> result;
  result.reserve(next.links.size());
  std::set<UeId> retry_users;
  std::vector<UeId> order;
  for (const auto& [id, link] : next.links)
    if (link.harq.ready()) order.push_back(id);
  std::rotate(order.begin(), std::upper_bound(order.begin(), order.end(), next.retry_cursor),
              order.end());
  unsigned used = 0;
  for (auto id : order) {
    auto& link = next.links.at(id);
    retry_users.insert(id);  // Never bypass an unfittable retry with new data for this UE.
    if (!link.active || !link.quality) continue;
    const auto process = *link.harq.ready();
    const auto& pending = link.harq.processes().at(process);
    if (now <= pending.sent_slot || capacity(link.quality, prbs - used) < pending.bytes) continue;
    unsigned blocks = 1;
    while (capacity(link.quality, blocks) < pending.bytes) ++blocks;
    const auto bytes = pending.bytes;
    const auto token = link.harq.retransmit(process, now);
    result.push_back({direction, {id, used, blocks, bytes}, token, now});
    used += blocks;
    next.retry_cursor = id;
  }
  for (auto& [id, link] : next.links) {
    const bool eligible = link.active && link.harq.available() && !retry_users.contains(id) &&
                          (direction == Direction::downlink || fresh(link));
    next.scheduler.configure(id, link.quality, link.priority, eligible);
  }
  for (auto grant : next.scheduler.schedule(prbs - used)) {
    grant.first_prb += used;
    auto token = next.links.at(grant.id).harq.start(grant.bytes, now);
    result.push_back({direction, grant, token, now});
  }
  next.scheduled = true;
  std::swap(planes_[d], next);
  return result;
}
FeedbackResult MacController::feedback(const Feedback& feedback) {
  const auto now = slot();
  auto& plane = planes_[index(feedback.direction)];
  const auto it = plane.links.find(feedback.ue);
  if (it == plane.links.end()) return FeedbackResult::ignored;
  auto& link = it->second;
  const auto completion = link.harq.feedback(feedback.token, feedback.ack, now);
  if (completion.result == FeedbackResult::acknowledged) link.delivered += completion.bytes;
  if (completion.result == FeedbackResult::failed) link.failed += completion.bytes;
  return completion.result;
}
Accounting MacController::accounting(Direction direction, UeId id) const {
  const auto& plane = planes_[index(direction)];
  const auto& link = plane.links.at(id);
  return {link.admitted, plane.scheduler.state().at(id).queued, link.harq.pending(), link.delivered,
          link.failed};
}
const HarqManager& MacController::harq(Direction direction, UeId id) const {
  return planes_[index(direction)].links.at(id).harq;
}
bool MacController::invariants_hold() const {
  for (unsigned d = 0; d < planes_.size(); ++d)
    for (const auto& [id, link] : planes_[d].links) {
      const auto a = accounting(static_cast<Direction>(d), id);
      // Subtract rather than summing: detects overflow as well as accounting drift.
      auto remaining = a.admitted;
      for (auto part : {a.queued, a.in_flight, a.delivered, a.failed}) {
        if (part > remaining) return false;
        remaining -= part;
      }
      if (remaining || a.queued > config_.max_outstanding_bytes ||
          a.in_flight > config_.max_outstanding_bytes - a.queued)
        return false;
      if (planes_[d].scheduler.state().at(id).served != a.admitted - a.queued) return false;
    }
  return true;
}
}  // namespace aether::mac
