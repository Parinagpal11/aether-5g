# Limitations and risk register
- Only Sprint 1 scheduling abstraction exists. No packet-level Layer 2 pipeline.
- Scheduling counters are not delivery counters; redesign accounting before loss injection.
- Greedy PF/QoS allocations can concentrate resources; priority starvation is possible.
- Synthetic quality/capacity must not be presented as NR TBS or CQI conformance.
- Single-owner execution; no multicore results, race-freedom claims, queue bounds or stress evidence.
- Allocation failure can throw; no hard-real-time allocator or bad_alloc recovery guarantee.
- Scenarios expose constant DL arrivals; bursty/UL/loss workloads pending.
- No security, RF, interoperability, Prometheus server or Grafana integration.
- Initial dependency download needs network; offline source override documented.
