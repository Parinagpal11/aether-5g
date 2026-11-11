#include "aether/mac/scheduler.hpp"
#include "aether/scheduler.hpp"
#include <gtest/gtest.h>
#include <limits>
#include <random>
#include <set>
using namespace aether::mac;
TEST(Capacity, ZeroAndBounds) {
  EXPECT_EQ(capacity(0, 10), 0); EXPECT_EQ(capacity(15, 0), 0);
  EXPECT_THROW(capacity(16, 1), std::invalid_argument);
  EXPECT_THROW(capacity(7, 276), std::invalid_argument);
}
TEST(Capacity, KnownCalculation) { EXPECT_EQ(capacity(7, 10), 326); }
TEST(Capacity, Monotonic) {
  for (unsigned q=1;q<=15;++q) for(unsigned n=1;n<=275;++n) {
    EXPECT_GE(capacity(q,n),capacity(q,n-1));
    EXPECT_GE(capacity(q,n),capacity(q-1,n));
  }
}
TEST(Clock, DurationsAndProgress) {
  for(unsigned mu=0;mu<=4;++mu) { SlotClock c(mu); EXPECT_EQ(c.duration_ns(),1000000U/(1U<<mu)); EXPECT_EQ(c.tick(),0); EXPECT_EQ(c.tick(),1); EXPECT_EQ(c.slot(),2); }
  EXPECT_THROW(SlotClock(5),std::invalid_argument);
}
TEST(Registry, Validation) {
  Scheduler s(Policy::round_robin); s.add({{1}});
  EXPECT_THROW(s.add({{1}}),std::invalid_argument);
  EXPECT_THROW(s.add({{0}}),std::invalid_argument);
  EXPECT_THROW(s.add({{2},1}),std::invalid_argument);
  EXPECT_THROW(s.configure({1},16,1,true),std::invalid_argument);
  EXPECT_THROW(s.configure({1},7,0,true),std::invalid_argument);
  EXPECT_THROW(s.configure({2},7,1,true),std::out_of_range);
  EXPECT_THROW(s.enqueue({2},1),std::out_of_range);
  EXPECT_THROW(s.remove({2}),std::out_of_range);
}
TEST(Registry, OverflowIsAtomic) {
  Scheduler s(Policy::round_robin); s.add({{1}});
  auto max=std::numeric_limits<std::uint64_t>::max(); s.enqueue({1},max);
  EXPECT_THROW(s.enqueue({1},1),std::overflow_error);
  s.schedule(1); EXPECT_THROW(s.enqueue({1},1),std::overflow_error);
  EXPECT_EQ(s.state().at({1}).queued+s.state().at({1}).served,max);
}
TEST(RoundRobin, RotationAndRemoval) {
  Scheduler s(Policy::round_robin);
  for(unsigned id=1;id<=3;++id){s.add({{id}});s.enqueue({id},10000);}
  EXPECT_EQ(s.schedule(1).at(0).id.value,1); s.remove({1});
  EXPECT_EQ(s.schedule(1).at(0).id.value,2);
  EXPECT_EQ(s.schedule(1).at(0).id.value,3);
  EXPECT_EQ(s.schedule(1).at(0).id.value,2);
}
TEST(RoundRobin, InactiveAndOutage) {
  Scheduler s(Policy::round_robin); s.add({{1}});s.add({{2}});
  s.enqueue({1},100);s.enqueue({2},100);
  s.configure({1},7,1,false);s.configure({2},0,1,true);
  EXPECT_TRUE(s.schedule(10).empty());
  s.configure({2},7,1,true);EXPECT_EQ(s.schedule(10).at(0).id.value,2);
}
TEST(RoundRobin, PartialAndContiguous) {
  Scheduler s(Policy::round_robin);s.add({{1}});s.add({{2}});
  s.enqueue({1},1);s.enqueue({2},1000);auto g=s.schedule(3);
  ASSERT_EQ(g.size(),2);EXPECT_EQ(g[0].bytes,1);EXPECT_EQ(g[0].prbs,1);
  EXPECT_EQ(g[1].first_prb,1);EXPECT_EQ(g[1].prbs,2);
}
TEST(Scheduler, EmptyZeroAndInvalidResource) {
  Scheduler s(Policy::round_robin);EXPECT_TRUE(s.schedule(10).empty());
  s.add({{1}});s.enqueue({1},10);EXPECT_TRUE(s.schedule(0).empty());
  EXPECT_THROW(s.schedule(276),std::invalid_argument);EXPECT_EQ(s.state().at({1}).queued,10);
  EXPECT_THROW(Scheduler(static_cast<Policy>(99)),std::invalid_argument);
}
TEST(ProportionalFair, ChannelAndHistory) {
  Scheduler s(Policy::proportional_fair);s.add({{1}});s.add({{2}});
  s.configure({1},1,1,true);s.configure({2},15,1,true);
  s.enqueue({1},1000000);s.enqueue({2},1000000);
  EXPECT_EQ(s.schedule(10).at(0).id.value,2);
  for(int i=0;i<200;++i)s.schedule(10);
  EXPECT_GT(s.state().at({1}).served,0);EXPECT_GT(s.state().at({2}).served,s.state().at({1}).served);
}
TEST(Qos, PriorityOverridesChannel) {
  Scheduler s(Policy::qos);s.add({{1}});s.add({{2}});
  s.configure({1},1,1,true);s.configure({2},15,16,true);
  s.enqueue({1},1000);s.enqueue({2},1000);EXPECT_EQ(s.schedule(1).at(0).id.value,1);
}
TEST(Fairness, KnownValues) {
  EXPECT_DOUBLE_EQ(fairness({}),0);EXPECT_DOUBLE_EQ(fairness({0,0}),0);
  EXPECT_DOUBLE_EQ(fairness({10,10}),1);EXPECT_DOUBLE_EQ(fairness({10,0}),0.5);
}
TEST(Regression, LegacyShrinkingVector) {
  std::vector<aether::UE> u{{1,100},{2,100},{3,100}};
  aether::RoundRobinScheduler s(10);s.schedule(u);s.schedule(u);u.resize(1);
  EXPECT_EQ(s.schedule(u).at(0).ue_id,1);
}
class Invariants : public testing::TestWithParam<Policy> {};
TEST_P(Invariants, SeededReplayAndConservation) {
  Scheduler a(GetParam()),b(GetParam());std::mt19937 rng(42);
  for(unsigned id=1;id<=100;++id){a.add({{id}});b.add({{id}});}
  for(int slot=0;slot<1000;++slot){
    for(unsigned id=1;id<=100;++id){auto bytes=rng()%1000;auto q=rng()%16;auto p=1+rng()%16;bool active=rng()%5!=0;
      for(auto* s:{&a,&b}){s->enqueue({id},bytes);s->configure({id},q,p,active);}}
    auto before=a.state();auto budget=rng()%276;auto g=a.schedule(budget);EXPECT_EQ(g,b.schedule(budget));
    unsigned used=0;std::set<unsigned> seen;
    for(auto x:g){EXPECT_TRUE(seen.insert(x.id.value).second);EXPECT_EQ(x.first_prb,used);used+=x.prbs;
      const auto& ue=before.at(x.id);EXPECT_TRUE(ue.active);EXPECT_GT(x.bytes,0);EXPECT_LE(x.bytes,ue.queued);EXPECT_LE(x.bytes,capacity(ue.quality,x.prbs));}
    EXPECT_LE(used,budget);
    for(const auto& [id,ue]:a.state())EXPECT_EQ(ue.queued+ue.served,before.at(id).queued+before.at(id).served);
  }
}
INSTANTIATE_TEST_SUITE_P(Policies,Invariants,testing::Values(Policy::round_robin,Policy::proportional_fair,Policy::qos));
