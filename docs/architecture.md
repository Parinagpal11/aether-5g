# Architecture and boundaries

The current executable composes constant traffic generation, a single-owner scheduler, slot clock and CSV output. The new domain API is `include/aether/mac/scheduler.hpp`; implementation lives in `src/mac/scheduler.cpp`. Original API and demo remain separate for compatibility.

Scheduler owns the UE map and both queue/throughput state. Only validated operations mutate it. Ordered IDs produce stable ties and replay; callers cannot mutate exposed const state. A schedule call computes a UE order, assigns each UE at most one contiguous grant, drains its backlog, and updates historical service. No resources overlap. Empty, inactive and outage UEs are excluded.

Policy is injected at construction through an enum on one common interface. Round-robin remembers the last served identifier, avoiding dangling indices after removal. PF orders by one-PRB capacity divided by an exponentially averaged bytes/slot service (alpha=0.01). QoS orders lower numeric priority first, then PF. Both are greedy per slot; strict QoS can starve low-priority traffic. RR provides turns, not equal byte throughput under unequal channels.

Future boundaries: traffic -> SDAP -> PDCP -> RLC -> MAC -> simulated PHY, with a receive path for UL. Configuration events will be serialized by the simulation controller. HARQ completion must be separated from current scheduled-byte accounting before PHY losses are introduced.
