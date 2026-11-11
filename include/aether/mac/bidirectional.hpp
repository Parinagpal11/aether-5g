#pragma once
#include <iosfwd>

#include "aether/mac/controller.hpp"
#include "aether/phy/feedback.hpp"

namespace aether::mac {
struct BidirectionalConfig {
  unsigned ues = 100, traffic_slots = 200;
  Policy policy = Policy::round_robin;
  phy::FeedbackConfig phy{};
};
struct BidirectionalResult {
  std::array<Accounting, 2> totals{};
  std::uint64_t slots = 0, transmissions = 0, retries = 0;
  std::uint64_t trace_hash = 14695981039346656037ULL;
  bool operator==(const BidirectionalResult&) const = default;
};
// Fixed 100-PRB independent direction budgets, with a bounded drain phase.
BidirectionalResult run_bidirectional(BidirectionalConfig config, std::ostream* trace = nullptr);
}  // namespace aether::mac
