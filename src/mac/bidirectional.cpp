#include "aether/mac/bidirectional.hpp"

#include <map>
#include <ostream>
#include <stdexcept>

namespace aether::mac {
namespace {
void hash(std::uint64_t& digest, std::uint64_t value) {
  // FNV-1a over a defined little-endian integer encoding.
  for (unsigned i = 0; i < 8; ++i) {
    digest ^= value & 255;
    digest *= 1099511628211ULL;
    value >>= 8;
  }
}
}  // namespace
BidirectionalResult run_bidirectional(BidirectionalConfig config, std::ostream* trace) {
  if (!config.ues || config.ues > 100 || !config.traffic_slots || config.traffic_slots > 1000 ||
      config.phy.delay_slots > 100)
    throw std::invalid_argument("scenario bounds: UEs 1..100, traffic slots 1..1000, delay <=100");
  MacController controller(config.policy);
  phy::FeedbackModel model(config.phy);
  for (unsigned id = 1; id <= config.ues; ++id) controller.add({id});
  std::vector<std::uint64_t> ul_arrivals(config.ues + 1);
  std::map<std::uint64_t, std::vector<phy::FeedbackEvent>> events;
  BidirectionalResult result;
  if (trace) *trace << "slot,direction,ue,first_prb,prbs,bytes,process,generation,attempt\n";
  // Finite traffic is followed by drain. No assumption that ACK will eventually arrive:
  // max attempts retires data to failed, including when all feedback is silent.
  for (std::uint64_t slot = 0; slot < config.traffic_slots + 20000ULL; ++slot) {
    controller.begin_slot(slot);
    auto due = events.begin();
    while (due != events.end() && due->first <= slot) {
      for (const auto& event : due->second) {
        const auto outcome = controller.feedback(event.feedback);
        hash(result.trace_hash, static_cast<unsigned>(outcome));
      }
      due = events.erase(due);
    }
    for (unsigned id = 1; id <= config.ues; ++id) {
      if (slot < config.traffic_slots) {
        controller.enqueue_downlink({id}, 48 + (id % 5) * 8);
        ul_arrivals[id] += 32 + (id % 7) * 8;
      }
      const bool outage =
          id % 10 == 0 && slot >= config.traffic_slots / 3 && slot < (2 * config.traffic_slots) / 3;
      controller.configure(Direction::downlink, {id}, outage ? 0 : 7, 1 + id % 4, true);
      controller.configure(Direction::uplink, {id}, outage ? 0 : 7, 1 + id % 4, true);
      const auto a = controller.accounting(Direction::uplink, {id});
      const auto retired = a.delivered + a.failed;
      controller.report_uplink(
          {id}, {slot + 1, slot, ul_arrivals[id], retired, ul_arrivals[id] - retired});
    }
    for (auto direction : {Direction::downlink, Direction::uplink}) {
      for (const auto& tx : controller.schedule(direction, 100)) {
        ++result.transmissions;
        if (tx.token.attempt > 1) ++result.retries;
        for (auto value : {slot, std::uint64_t(direction), std::uint64_t(tx.grant.id.value),
                           std::uint64_t(tx.grant.first_prb), std::uint64_t(tx.grant.prbs),
                           tx.grant.bytes, std::uint64_t(tx.token.process), tx.token.generation,
                           std::uint64_t(tx.token.attempt)})
          hash(result.trace_hash, value);
        if (trace)
          *trace << slot << ',' << (direction == Direction::downlink ? "DL" : "UL") << ','
                 << tx.grant.id.value << ',' << tx.grant.first_prb << ',' << tx.grant.prbs << ','
                 << tx.grant.bytes << ',' << tx.token.process << ',' << tx.token.generation << ','
                 << tx.token.attempt << '\n';
        if (auto feedback = model.evaluate(tx)) events[feedback->due_slot].push_back(*feedback);
      }
    }
    if (!controller.invariants_hold()) throw std::logic_error("byte conservation violated");
    result.totals = {};
    for (auto direction : {Direction::downlink, Direction::uplink}) {
      auto& total = result.totals[static_cast<unsigned>(direction)];
      for (unsigned id = 1; id <= config.ues; ++id) {
        const auto a = controller.accounting(direction, {id});
        total.admitted += a.admitted;
        total.queued += a.queued;
        total.in_flight += a.in_flight;
        total.delivered += a.delivered;
        total.failed += a.failed;
      }
    }
    result.slots = slot + 1;
    if (slot + 1 >= config.traffic_slots && !result.totals[0].queued &&
        !result.totals[0].in_flight && !result.totals[1].queued && !result.totals[1].in_flight &&
        events.empty())
      return result;
  }
  throw std::runtime_error("bidirectional scenario did not drain within bound");
}
}  // namespace aether::mac
