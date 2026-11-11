#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

#include "aether/phy/feedback.hpp"
using namespace aether;
namespace {
mac::Transmission tx() { return {mac::Direction::uplink, {{1}, 0, 4, 100}, {0, 1, 1}, 0}; }
}  // namespace
TEST(Phy, CertainAckNackAndSilence) {
  EXPECT_TRUE(phy::FeedbackModel({42, 0, 0, 1}).evaluate(tx())->feedback.ack);
  EXPECT_FALSE(phy::FeedbackModel({42, 1000, 0, 1}).evaluate(tx())->feedback.ack);
  EXPECT_FALSE(phy::FeedbackModel({42, 0, 1000, 1}).evaluate(tx()));
}
TEST(Phy, DeterministicAndIdentityPreserved) {
  phy::FeedbackModel model({42, 300, 0, 2});
  for (unsigned i = 1; i < 100; ++i) {
    auto t = tx();
    t.grant.id = {i};
    t.slot = i;
    auto first = model.evaluate(t);
    auto second = model.evaluate(t);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    EXPECT_EQ(first->due_slot, i + 2);
    EXPECT_EQ(first->feedback.ack, second->feedback.ack);
    EXPECT_EQ(first->feedback.ue, t.grant.id);
    EXPECT_EQ(first->feedback.direction, t.direction);
    EXPECT_EQ(first->feedback.token, t.token);
  }
}
TEST(Phy, RejectsInvalidConfigurationAndOverflow) {
  EXPECT_THROW(phy::FeedbackModel(phy::FeedbackConfig{42, 1001, 0, 1}), std::invalid_argument);
  EXPECT_THROW(phy::FeedbackModel(phy::FeedbackConfig{42, 0, 1001, 1}), std::invalid_argument);
  EXPECT_THROW(phy::FeedbackModel(phy::FeedbackConfig{42, 0, 0, 0}), std::invalid_argument);
  auto t = tx();
  t.slot = std::numeric_limits<std::uint64_t>::max();
  EXPECT_THROW(phy::FeedbackModel().evaluate(t), std::overflow_error);
}
