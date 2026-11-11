#pragma once

#include <optional>

#include "aether/mac/controller.hpp"

namespace aether::phy {
struct FeedbackConfig {
  std::uint64_t seed = 42;
  unsigned nack_per_mille = 200, silent_per_mille = 20;
  std::uint64_t delay_slots = 1;
};
struct FeedbackEvent {
  std::uint64_t due_slot;
  mac::Feedback feedback;
};
// Stateless integer hashing gives deterministic outcomes independent of call order.
// ACK is the simulation delivery event, not a separately simulated receiver action.
class FeedbackModel {
 public:
  explicit FeedbackModel(FeedbackConfig config = {});
  std::optional<FeedbackEvent> evaluate(const mac::Transmission& transmission) const;

 private:
  FeedbackConfig config_;
};
}  // namespace aether::phy
