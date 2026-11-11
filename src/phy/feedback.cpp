#include "aether/phy/feedback.hpp"

#include <limits>
#include <stdexcept>

namespace aether::phy {
namespace {
std::uint64_t mix(std::uint64_t x) {
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}
}  // namespace
FeedbackModel::FeedbackModel(FeedbackConfig config) : config_(config) {
  if (config.nack_per_mille > 1000 || config.silent_per_mille > 1000 || !config.delay_slots)
    throw std::invalid_argument("invalid PHY feedback configuration");
}
std::optional<FeedbackEvent> FeedbackModel::evaluate(const mac::Transmission& t) const {
  if (t.slot > std::numeric_limits<std::uint64_t>::max() - config_.delay_slots)
    throw std::overflow_error("feedback slot overflow");
  if (t.direction != mac::Direction::downlink && t.direction != mac::Direction::uplink)
    throw std::invalid_argument("invalid PHY direction");
  auto key = mix(config_.seed ^ t.grant.id.value);
  key = mix(key ^ static_cast<unsigned>(t.direction));
  key = mix(key ^ t.token.process);
  key = mix(key ^ t.token.generation);
  key = mix(key ^ t.token.attempt);
  key = mix(key ^ t.slot);
  if (key % 1000 < config_.silent_per_mille) return std::nullopt;
  const bool ack = mix(key) % 1000 >= config_.nack_per_mille;
  return FeedbackEvent{t.slot + config_.delay_slots, {t.direction, t.grant.id, t.token, ack}};
}
}  // namespace aether::phy
