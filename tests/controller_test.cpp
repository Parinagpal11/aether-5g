#include "aether/mac/controller.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <tuple>
using namespace aether::mac;
constexpr auto DL = Direction::downlink;
constexpr auto UL = Direction::uplink;
namespace {
Feedback reply(const Transmission& t, bool ack = true) {
  return {t.direction, t.grant.id, t.token, ack};
}
}  // namespace
TEST(Controller, DlVerticalSliceCountsOnlyAcknowledgedBytes) {
  MacController c;
  c.add({1});
  c.enqueue_downlink({1}, 100);
  c.begin_slot(0);
  auto tx = c.schedule(DL, 10);
  ASSERT_EQ(tx.size(), 1);
  EXPECT_EQ(c.accounting(DL, {1}), (Accounting{100, 0, 100, 0, 0}));
  c.begin_slot(1);
  EXPECT_EQ(c.feedback(reply(tx[0])), FeedbackResult::acknowledged);
  EXPECT_EQ(c.feedback(reply(tx[0])), FeedbackResult::ignored);
  EXPECT_EQ(c.accounting(DL, {1}), (Accounting{100, 0, 0, 100, 0}));
  EXPECT_TRUE(c.invariants_hold());
}
TEST(Controller, DirectionsAndProcessNamespacesAreIndependent) {
  MacController c;
  c.add({1});
  c.begin_slot(0);
  c.enqueue_downlink({1}, 100);
  c.report_uplink({1}, {1, 0, 200, 0, 200});
  auto dl = c.schedule(DL, 10);
  auto ul = c.schedule(UL, 0);
  ASSERT_EQ(dl.size(), 1);
  EXPECT_TRUE(ul.empty());
  EXPECT_EQ(c.accounting(UL, {1}), (Accounting{200, 200, 0, 0, 0}));
  c.begin_slot(1);
  ul = c.schedule(UL, 10);
  ASSERT_EQ(ul.size(), 1);
  EXPECT_EQ(dl[0].token.process, ul[0].token.process);
  c.feedback(reply(dl[0]));
  EXPECT_EQ(c.accounting(UL, {1}).delivered, 0);
  c.begin_slot(2);
  c.feedback(reply(ul[0]));
  EXPECT_EQ(c.accounting(DL, {1}).delivered, 100);
  EXPECT_EQ(c.accounting(UL, {1}).delivered, 200);
  EXPECT_TRUE(c.invariants_hold());
}
TEST(Bsr, DelayedReportDoesNotReadmitPendingOrDeliveredBytes) {
  MacController c;
  c.add({1});
  c.begin_slot(0);
  c.report_uplink({1}, {1, 0, 100, 0, 100});
  auto t = c.schedule(UL, 10).at(0);
  c.begin_slot(1);
  c.report_uplink({1}, {2, 0, 100, 0, 100});
  EXPECT_TRUE(c.schedule(UL, 10).empty());
  c.feedback(reply(t));
  c.begin_slot(2);
  c.report_uplink({1}, {3, 0, 150, 0, 150});
  EXPECT_EQ(c.accounting(UL, {1}), (Accounting{150, 50, 0, 100, 0}));
  EXPECT_EQ(c.schedule(UL, 10).at(0).grant.bytes, 50);
  EXPECT_TRUE(c.invariants_hold());
}
TEST(Bsr, InvalidReportsAreAtomic) {
  MacController c;
  c.add({1});
  c.begin_slot(5);
  c.report_uplink({1}, {1, 5, 100, 0, 100});
  auto before = c.accounting(UL, {1});
  for (auto bad : {BufferStatusReport{0, 5, 100, 0, 100},
                   {1, 5, 100, 0, 100},
                   {2, 6, 100, 0, 100},
                   {2, 4, 100, 0, 100},
                   {2, 5, 99, 0, 99},
                   {2, 5, 100, 1, 99},
                   {2, 5, 100, 0, 99}}) {
    EXPECT_THROW(c.report_uplink({1}, bad), std::invalid_argument);
    EXPECT_EQ(c.accounting(UL, {1}), before);
  }
  EXPECT_THROW(c.report_uplink({2}, {2, 5, 100, 0, 100}), std::out_of_range);
}
TEST(Bsr, FreshnessStopsNewGrantsButNotRetries) {
  MacController c(Policy::round_robin, {{1, 3, 4}, 1, 1000});
  c.add({1});
  c.begin_slot(0);
  c.report_uplink({1}, {1, 0, 200, 0, 200});
  auto first = c.schedule(UL, 1).at(0);
  c.begin_slot(1);
  c.feedback(reply(first, false));
  c.begin_slot(2);
  EXPECT_THROW(c.report_uplink({1}, {2, 0, 200, 0, 200}), std::invalid_argument);
  auto retry = c.schedule(UL, 1).at(0);
  EXPECT_EQ(retry.token.attempt, 2);
  c.begin_slot(3);
  c.feedback(reply(retry));
  EXPECT_TRUE(c.schedule(UL, 10).empty());
  c.begin_slot(4);
  c.report_uplink({1}, {2, 4, 200, 32, 168});
  EXPECT_FALSE(c.schedule(UL, 10).empty());
}
TEST(Controller, RetriesDoNotConsumeUniqueBytesTwice) {
  MacController c;
  c.add({1});
  c.enqueue_downlink({1}, 100);
  c.begin_slot(0);
  auto first = c.schedule(DL, 10).at(0);
  c.begin_slot(1);
  c.feedback(reply(first, false));
  auto retry = c.schedule(DL, 10).at(0);
  EXPECT_EQ(c.accounting(DL, {1}), (Accounting{100, 0, 100, 0, 0}));
  c.begin_slot(2);
  EXPECT_EQ(c.feedback(reply(first)), FeedbackResult::ignored);
  c.feedback(reply(retry));
  EXPECT_EQ(c.accounting(DL, {1}), (Accounting{100, 0, 0, 100, 0}));
}
TEST(Controller, ExhaustedAttemptsRetireExactlyOnce) {
  MacController c(Policy::round_robin, {{1, 1, 2}, 20, 1000});
  c.add({1});
  c.enqueue_downlink({1}, 100);
  c.begin_slot(0);
  auto first = c.schedule(DL, 10).at(0);
  c.begin_slot(2);  // No feedback: terminal timeout.
  EXPECT_EQ(c.accounting(DL, {1}), (Accounting{100, 0, 0, 0, 100}));
  EXPECT_EQ(c.feedback(reply(first)), FeedbackResult::ignored);
  c.begin_slot(3);
  EXPECT_EQ(c.accounting(DL, {1}).failed, 100);
  EXPECT_TRUE(c.invariants_hold());
}
TEST(Controller, OutstandingBoundsApplyToInflightAndQueued) {
  MacController c(Policy::round_robin, {{1, 3, 4}, 20, 100});
  c.add({1});
  c.begin_slot(0);
  c.enqueue_downlink({1}, 100);
  c.schedule(DL, 10);
  EXPECT_THROW(c.enqueue_downlink({1}, 1), std::overflow_error);
  EXPECT_THROW(c.report_uplink({1}, {1, 0, 101, 0, 101}), std::overflow_error);
  EXPECT_EQ(c.accounting(UL, {1}).admitted, 0);
  EXPECT_TRUE(c.invariants_hold());
}
TEST(Controller, NoFreeProcessPreservesQueuedData) {
  MacController c(Policy::round_robin, {{1, 3, 4}, 20, 1000});
  c.add({1});
  c.enqueue_downlink({1}, 100);
  c.begin_slot(0);
  c.schedule(DL, 1);
  c.begin_slot(1);
  EXPECT_TRUE(c.schedule(DL, 10).empty());
  EXPECT_EQ(c.accounting(DL, {1}), (Accounting{100, 68, 32, 0, 0}));
}
TEST(Controller, OutageAndInsufficientRetryBudgetPreservePending) {
  MacController c;
  c.add({1});
  c.enqueue_downlink({1}, 100);
  c.begin_slot(0);
  auto first = c.schedule(DL, 10).at(0);
  c.begin_slot(1);
  c.feedback(reply(first, false));
  EXPECT_TRUE(c.schedule(DL, 1).empty());
  c.begin_slot(2);
  c.configure(DL, {1}, 0, 1, true);
  EXPECT_TRUE(c.schedule(DL, 10).empty());
  c.begin_slot(3);
  c.configure(DL, {1}, 7, 1, true);
  EXPECT_EQ(c.schedule(DL, 10).at(0).grant.bytes, 100);
  EXPECT_TRUE(c.invariants_hold());
}
TEST(Controller, BadClockBudgetAndRegistrationLeaveStateIntact) {
  MacController c;
  EXPECT_THROW(c.schedule(DL, 1), std::logic_error);
  EXPECT_THROW(c.report_uplink({1}, {1, 0, 1, 0, 1}), std::logic_error);
  EXPECT_THROW(c.add({0}), std::invalid_argument);
  c.add({1});
  EXPECT_THROW(c.add({1}), std::invalid_argument);
  c.begin_slot(0);
  EXPECT_THROW(c.begin_slot(0), std::invalid_argument);
  EXPECT_THROW(c.schedule(DL, 276), std::invalid_argument);
  EXPECT_TRUE(c.schedule(DL, 0).empty());
  EXPECT_THROW(c.schedule(DL, 0), std::logic_error);
  EXPECT_THROW(c.schedule(static_cast<Direction>(9), 1), std::invalid_argument);
  EXPECT_THROW(c.begin_slot(std::numeric_limits<std::uint64_t>::max()), std::overflow_error);
  EXPECT_EQ(c.slot(), 0);
  EXPECT_TRUE(c.invariants_hold());
}
TEST(Bsr, RetirementWatermarksAndZeroReports) {
  MacController c(Policy::round_robin, {{1, 1, 4}, 20, 1000});
  c.add({1});
  c.begin_slot(0);
  c.report_uplink({1}, {1, 0, 0, 0, 0});
  EXPECT_TRUE(c.schedule(UL, 10).empty());
  c.begin_slot(1);
  c.report_uplink({1}, {2, 1, 100, 0, 100});
  auto t = c.schedule(UL, 10).at(0);
  c.begin_slot(2);
  c.feedback(reply(t, false));
  c.report_uplink({1}, {3, 2, 100, 100, 0});
  EXPECT_TRUE(c.schedule(UL, 10).empty());
  EXPECT_THROW(c.report_uplink({1}, {4, 2, 100, 99, 1}), std::invalid_argument);
  EXPECT_EQ(c.accounting(UL, {1}), (Accounting{100, 0, 0, 0, 100}));
}
TEST(Controller, CumulativeCounterOverflowRejectedWithoutMutation) {
  constexpr auto max = std::numeric_limits<std::uint64_t>::max();
  MacController c(Policy::round_robin, {{1, 1, 4}, 20, max});
  c.add({1});
  c.begin_slot(0);
  c.enqueue_downlink({1}, max);
  auto t = c.schedule(DL, 1).at(0);
  c.begin_slot(1);
  c.feedback(reply(t));
  const auto before = c.accounting(DL, {1});
  EXPECT_THROW(c.enqueue_downlink({1}, 1), std::overflow_error);
  EXPECT_EQ(c.accounting(DL, {1}), before);
  EXPECT_TRUE(c.invariants_hold());
}
TEST(Controller, FullAccountingAgainstIndependentLedger) {
  using Key = std::tuple<Direction, unsigned, unsigned, std::uint64_t>;
  struct Expected {
    std::uint64_t admitted = 0, queued = 0, pending = 0, delivered = 0, failed = 0;
  };
  MacController c;
  std::map<std::pair<Direction, unsigned>, Expected> ledger;
  std::map<Key, std::uint64_t> blocks;
  std::set<Key> settled;
  std::vector<Transmission> previous;
  for (unsigned id = 1; id <= 10; ++id) c.add({id});
  for (std::uint64_t slot = 0; slot < 300; ++slot) {
    c.begin_slot(slot);
    for (const auto& t : previous) {
      auto& e = ledger[{t.direction, t.grant.id.value}];
      Key key{t.direction, t.grant.id.value, t.token.process, t.token.generation};
      bool ack = t.grant.id.value % 3 != 0 && t.token.attempt == 2;
      auto result = c.feedback(reply(t, ack));
      if (ack || t.token.attempt == 3) {
        EXPECT_TRUE(settled.insert(key).second);
        e.pending -= blocks.at(key);
        if (ack) {
          e.delivered += blocks.at(key);
          EXPECT_EQ(result, FeedbackResult::acknowledged);
        } else {
          e.failed += blocks.at(key);
          EXPECT_EQ(result, FeedbackResult::failed);
        }
      } else
        EXPECT_EQ(result, FeedbackResult::retry_ready);
      EXPECT_EQ(c.feedback(reply(t, ack)), FeedbackResult::ignored);
    }
    previous.clear();
    for (auto direction : {DL, UL}) {
      for (unsigned id = 1; id <= 10; ++id) {
        auto& e = ledger[{direction, id}];
        if (slot < 30) {
          const auto bytes = 20 + id * 3;
          e.admitted += bytes;
          e.queued += bytes;
          if (direction == DL) c.enqueue_downlink({id}, bytes);
        }
        if (direction == UL)
          c.report_uplink({id}, {slot + 1, slot, e.admitted, e.delivered + e.failed,
                                 e.admitted - e.delivered - e.failed});
      }
      const unsigned budget = 5 + slot % 13;
      auto transmissions = c.schedule(direction, budget);
      unsigned used = 0;
      std::set<unsigned> ids;
      for (const auto& t : transmissions) {
        EXPECT_TRUE(ids.insert(t.grant.id.value).second);
        EXPECT_EQ(t.grant.first_prb, used);
        used += t.grant.prbs;
        EXPECT_LE(t.grant.bytes, capacity(7, t.grant.prbs));
        auto& e = ledger[{direction, t.grant.id.value}];
        Key key{direction, t.grant.id.value, t.token.process, t.token.generation};
        if (t.token.attempt == 1) {
          EXPECT_TRUE(blocks.emplace(key, t.grant.bytes).second);
          ASSERT_LE(t.grant.bytes, e.queued);
          e.queued -= t.grant.bytes;
          e.pending += t.grant.bytes;
        } else {
          EXPECT_FALSE(settled.contains(key));
          EXPECT_EQ(blocks.at(key), t.grant.bytes);
        }
        previous.push_back(t);
      }
      EXPECT_LE(used, budget);
      for (unsigned id = 1; id <= 10; ++id) {
        const auto& e = ledger.at({direction, id});
        EXPECT_EQ(c.accounting(direction, {id}),
                  (Accounting{e.admitted, e.queued, e.pending, e.delivered, e.failed}));
      }
    }
    EXPECT_TRUE(c.invariants_hold());
  }
  for (const auto& [key, e] : ledger) {
    EXPECT_EQ(e.pending, 0);
    EXPECT_EQ(e.queued, 0);
    EXPECT_EQ(e.admitted, e.delivered + e.failed);
  }
}
