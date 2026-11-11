#include "aether/mac/bidirectional.hpp"

#include <gtest/gtest.h>
using namespace aether::mac;
class Bidirectional : public testing::TestWithParam<Policy> {};
TEST_P(Bidirectional, HundredUesReplayAndDrain) {
  BidirectionalConfig config{100, 100, GetParam(), {42, 250, 30, 1}};
  const auto first = run_bidirectional(config);
  EXPECT_EQ(first, run_bidirectional(config));
  EXPECT_GT(first.retries, 0);
  for (const auto& a : first.totals) {
    EXPECT_GT(a.delivered, 0);
    EXPECT_GT(a.failed, 0);
    EXPECT_EQ(a.queued, 0);
    EXPECT_EQ(a.in_flight, 0);
    EXPECT_EQ(a.admitted, a.delivered + a.failed);
  }
}
INSTANTIATE_TEST_SUITE_P(AllPolicies, Bidirectional,
                         testing::Values(Policy::round_robin, Policy::proportional_fair,
                                         Policy::qos));
TEST(BidirectionalEdges, AllFeedbackSilentTerminatesWithFailure) {
  auto result = run_bidirectional({10, 10, Policy::round_robin, {42, 0, 1000, 1}});
  for (const auto& a : result.totals) {
    EXPECT_EQ(a.delivered, 0);
    EXPECT_EQ(a.failed, a.admitted);
    EXPECT_EQ(a.queued, 0);
    EXPECT_EQ(a.in_flight, 0);
  }
}
TEST(BidirectionalEdges, PerfectChannelDeliversEverything) {
  auto result = run_bidirectional({10, 10, Policy::round_robin, {42, 0, 0, 1}});
  EXPECT_EQ(result.retries, 0);
  for (const auto& a : result.totals) {
    EXPECT_EQ(a.failed, 0);
    EXPECT_EQ(a.delivered, a.admitted);
  }
}
