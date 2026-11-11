#include "aether/mac/harq.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>
using namespace aether::mac;

TEST(Harq, ConfigurationValidation) {
  EXPECT_THROW(HarqManager(HarqConfig{0, 3, 4}), std::invalid_argument);
  EXPECT_THROW(HarqManager(HarqConfig{17, 3, 4}), std::invalid_argument);
  EXPECT_THROW(HarqManager(HarqConfig{1, 0, 4}), std::invalid_argument);
  EXPECT_THROW(HarqManager(HarqConfig{1, 33, 4}), std::invalid_argument);
  EXPECT_THROW(HarqManager(HarqConfig{1, 3, 0}), std::invalid_argument);
}
TEST(Harq, AckDeliversOnce) {
  HarqManager h;
  auto t = h.start(100, 0);
  EXPECT_EQ(h.pending(), 100);
  EXPECT_EQ(h.processes()[0].state, HarqState::waiting_ack);
  auto result = h.feedback(t, true, 1);
  EXPECT_EQ(result.result, FeedbackResult::acknowledged);
  EXPECT_EQ(result.bytes, 100);
  EXPECT_EQ(h.pending(), 0);
  EXPECT_EQ(h.processes()[0].state, HarqState::idle);
  EXPECT_EQ(h.feedback(t, true, 1).result, FeedbackResult::ignored);
}
TEST(Harq, NackRetryKeepsBytesAndRejectsOldAttempt) {
  HarqManager h;
  auto first = h.start(100, 0);
  EXPECT_EQ(h.feedback(first, false, 1).result, FeedbackResult::retry_ready);
  EXPECT_EQ(h.pending(), 100);
  ASSERT_TRUE(h.ready());
  auto second = h.retransmit(*h.ready(), 1);
  EXPECT_EQ(second.generation, first.generation);
  EXPECT_EQ(second.attempt, 2);
  EXPECT_EQ(h.pending(), 100);
  EXPECT_EQ(h.feedback(first, true, 2).result, FeedbackResult::ignored);
  EXPECT_EQ(h.feedback(second, true, 2).bytes, 100);
}
TEST(Harq, AttemptLimitAndFailedReuse) {
  HarqManager h({1, 2, 4});
  auto first = h.start(100, 0);
  h.feedback(first, false, 1);
  auto second = h.retransmit(0, 1);
  EXPECT_EQ(h.feedback(second, false, 2).bytes, 100);
  EXPECT_EQ(h.processes()[0].state, HarqState::failed);
  EXPECT_EQ(h.pending(), 0);
  EXPECT_FALSE(h.ready());
  auto next = h.start(50, 2);
  EXPECT_GT(next.generation, second.generation);
  EXPECT_EQ(h.feedback(second, true, 3).result, FeedbackResult::ignored);
  EXPECT_EQ(h.feedback(next, true, 3).bytes, 50);
}
TEST(Harq, DeadlineIsExclusiveAndExpiryIsIdempotent) {
  HarqManager h({1, 1, 3});
  auto t = h.start(100, 5);
  EXPECT_EQ(h.feedback(t, true, 5).result, FeedbackResult::ignored);
  EXPECT_EQ(h.feedback(t, true, 8).result, FeedbackResult::ignored);
  EXPECT_EQ(h.expire(7), 0);
  EXPECT_EQ(h.expire(8), 100);
  EXPECT_EQ(h.expire(9), 0);
}
TEST(Harq, TimeoutMakesRetryReady) {
  HarqManager h;
  h.start(100, 0);
  EXPECT_EQ(h.expire(4), 0);
  EXPECT_EQ(h.processes()[0].state, HarqState::retransmission_ready);
  EXPECT_EQ(h.pending(), 100);
}
TEST(Harq, ExhaustionAndInvalidOperationsAreAtomic) {
  HarqManager h({1, 3, 4});
  EXPECT_THROW(h.start(0, 0), std::invalid_argument);
  EXPECT_THROW(h.retransmit(0, 0), std::logic_error);
  EXPECT_THROW(h.retransmit(1, 0), std::out_of_range);
  auto t = h.start(100, 0);
  EXPECT_THROW(h.start(50, 0), std::runtime_error);
  EXPECT_FALSE(h.available());
  EXPECT_EQ(h.feedback({99, 1, 1}, true, 1).result, FeedbackResult::ignored);
  EXPECT_EQ(h.feedback({0, 9, 1}, true, 1).result, FeedbackResult::ignored);
  EXPECT_EQ(h.pending(), 100);
  EXPECT_EQ(h.feedback(t, true, 1).bytes, 100);
}
TEST(Harq, OverflowRejected) {
  HarqManager h;
  const auto max = std::numeric_limits<std::uint64_t>::max();
  EXPECT_THROW(h.start(1, max), std::overflow_error);
  h.start(max, 0);
  EXPECT_THROW(h.start(1, 0), std::overflow_error);
  EXPECT_EQ(h.pending(), max);
}
