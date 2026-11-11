#include "aether/mac/scheduler.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace aether::mac {
namespace {
void validate(unsigned q, unsigned p) {
  if (q > 15 || p < 1 || p > 16) throw std::invalid_argument("quality/priority out of range");
}
}
std::uint64_t capacity(unsigned quality, unsigned prbs) {
  validate(quality, 1);
  if (prbs > 275) throw std::invalid_argument("PRBs exceed simulation limit");
  if (!quality) return 0;
  // 12 subcarriers, 12 data symbols, single layer; synthetic Qm and rate.
  const unsigned qm = quality <= 5 ? 2 : quality <= 10 ? 4 : 6;
  const unsigned rate1024 = 128 + quality * 48;
  return std::uint64_t{prbs} * 144 * qm * rate1024 / (1024 * 8);
}
double fairness(const std::vector<std::uint64_t>& values) {
  long double sum = 0, squares = 0;
  for (auto x : values) { sum += x; squares += static_cast<long double>(x) * x; }
  return squares == 0 ? 0 : static_cast<double>(sum * sum / (values.size() * squares));
}
SlotClock::SlotClock(unsigned numerology) {
  if (numerology > 4) throw std::invalid_argument("numerology must be 0..4");
  duration_ = 1000000U >> numerology;
}
std::uint64_t SlotClock::tick() {
  if (slot_ == std::numeric_limits<std::uint64_t>::max()) throw std::overflow_error("slot overflow");
  return slot_++;
}
Scheduler::Scheduler(Policy policy) : policy_(policy) {
  if (policy != Policy::round_robin && policy != Policy::proportional_fair && policy != Policy::qos)
    throw std::invalid_argument("invalid policy");
}
void Scheduler::add(Ue ue) {
  validate(ue.quality, ue.priority);
  if (!ue.id.value || ue.queued || ue.served) throw std::invalid_argument("new UE must have zero counters and nonzero ID");
  ue.average = 1;
  if (!ues_.emplace(ue.id, ue).second) throw std::invalid_argument("duplicate UE");
}
void Scheduler::remove(UeId id) {
  if (!ues_.erase(id)) throw std::out_of_range("unknown UE");
}
void Scheduler::enqueue(UeId id, std::uint64_t bytes) {
  auto& ue = ues_.at(id);
  // Conservatively bound lifetime admitted bytes, so served counters cannot overflow.
  if (bytes > std::numeric_limits<std::uint64_t>::max() - ue.queued - ue.served)
    throw std::overflow_error("byte accounting overflow");
  ue.queued += bytes;
}
void Scheduler::configure(UeId id, unsigned quality, unsigned priority, bool active) {
  validate(quality, priority);
  auto& ue = ues_.at(id);
  ue.quality = quality; ue.priority = priority; ue.active = active;
}
std::vector<Grant> Scheduler::schedule(unsigned available_prbs) {
  if (available_prbs > 275) throw std::invalid_argument("PRBs exceed simulation limit");
  std::vector<UeId> order;
  for (const auto& [id, ue] : ues_)
    if (ue.active && ue.queued && ue.quality) order.push_back(id);
  if (policy_ == Policy::round_robin) {
    auto start = std::upper_bound(order.begin(), order.end(), cursor_);
    std::rotate(order.begin(), start, order.end());
  } else {
    std::stable_sort(order.begin(), order.end(), [&](auto a, auto b) {
      const auto& x = ues_.at(a); const auto& y = ues_.at(b);
      if (policy_ == Policy::qos && x.priority != y.priority) return x.priority < y.priority;
      return capacity(x.quality, 1) / x.average > capacity(y.quality, 1) / y.average;
    });
  }
  std::vector<Grant> grants;
  grants.reserve(order.size());
  unsigned used = 0;
  for (auto id : order) {
    if (used == available_prbs) break;
    auto& ue = ues_.at(id);
    unsigned blocks = 1;
    while (blocks < available_prbs - used && capacity(ue.quality, blocks) < ue.queued) ++blocks;
    const auto bytes = std::min(ue.queued, capacity(ue.quality, blocks));
    grants.push_back({id, used, blocks, bytes});
    ue.queued -= bytes; ue.served += bytes;
    used += blocks; cursor_ = id;
  }
  for (auto& [id, ue] : ues_) {
    std::uint64_t bytes = 0;
    for (const auto& grant : grants) if (grant.id == id) bytes = grant.bytes;
    ue.average = std::max(1e-9, 0.99 * ue.average + 0.01 * static_cast<double>(bytes));
  }
  return grants;
}
}
