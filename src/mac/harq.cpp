#include "aether/mac/harq.hpp"

#include <limits>
#include <stdexcept>

namespace aether::mac {
HarqManager::HarqManager(HarqConfig config) : config_(config) {
  if (!config.processes || config.processes > 16 || !config.max_attempts ||
      config.max_attempts > 32 || !config.timeout_slots)
    throw std::invalid_argument("invalid HARQ configuration");
  processes_.resize(config.processes);
}
void HarqManager::validate_slot(std::uint64_t slot) const {
  if (slot > std::numeric_limits<std::uint64_t>::max() - config_.timeout_slots)
    throw std::overflow_error("HARQ deadline overflow");
}
bool HarqManager::available() const {
  for (const auto& p : processes_)
    if ((p.state == HarqState::idle || p.state == HarqState::failed) &&
        p.generation < std::numeric_limits<std::uint64_t>::max())
      return true;
  return false;
}
std::optional<unsigned> HarqManager::ready() const {
  for (unsigned i = 0; i < processes_.size(); ++i)
    if (processes_[i].state == HarqState::retransmission_ready) return i;
  return std::nullopt;
}
AttemptToken HarqManager::start(std::uint64_t bytes, std::uint64_t slot) {
  validate_slot(slot);
  if (!bytes) throw std::invalid_argument("zero-byte HARQ transmission");
  if (bytes > std::numeric_limits<std::uint64_t>::max() - pending())
    throw std::overflow_error("HARQ pending overflow");
  for (unsigned i = 0; i < processes_.size(); ++i) {
    auto& p = processes_[i];
    if ((p.state != HarqState::idle && p.state != HarqState::failed) ||
        p.generation == std::numeric_limits<std::uint64_t>::max())
      continue;
    p = {HarqState::waiting_ack, p.generation + 1, bytes, slot, slot + config_.timeout_slots, 1};
    return {i, p.generation, p.attempts};
  }
  throw std::runtime_error("HARQ processes exhausted");
}
AttemptToken HarqManager::retransmit(unsigned process, std::uint64_t slot) {
  validate_slot(slot);
  auto& p = processes_.at(process);
  if (p.state != HarqState::retransmission_ready || slot <= p.sent_slot)
    throw std::logic_error("HARQ retransmission not eligible");
  p.state = HarqState::waiting_ack;
  p.sent_slot = slot;
  p.deadline = slot + config_.timeout_slots;
  ++p.attempts;
  return {process, p.generation, p.attempts};
}
Completion HarqManager::negative(HarqProcess& p) {
  if (p.attempts >= config_.max_attempts) {
    p.state = HarqState::failed;
    return {FeedbackResult::failed, p.bytes};
  }
  p.state = HarqState::retransmission_ready;
  return {FeedbackResult::retry_ready, 0};
}
Completion HarqManager::feedback(AttemptToken token, bool ack, std::uint64_t slot) {
  if (token.process >= processes_.size()) return {};
  auto& p = processes_[token.process];
  if (p.state != HarqState::waiting_ack || p.generation != token.generation ||
      p.attempts != token.attempt || slot <= p.sent_slot || slot >= p.deadline)
    return {};
  if (!ack) return negative(p);
  p.state = HarqState::idle;
  return {FeedbackResult::acknowledged, p.bytes};
}
std::uint64_t HarqManager::expire(std::uint64_t slot) {
  std::uint64_t failed = 0;
  for (auto& p : processes_)
    if (p.state == HarqState::waiting_ack && slot >= p.deadline) failed += negative(p).bytes;
  return failed;
}
std::uint64_t HarqManager::pending() const {
  std::uint64_t bytes = 0;
  for (const auto& p : processes_)
    if (p.state == HarqState::waiting_ack || p.state == HarqState::retransmission_ready)
      bytes += p.bytes;
  return bytes;
}
}  // namespace aether::mac
