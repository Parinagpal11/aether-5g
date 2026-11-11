# Decisions
Preserve legacy demo/API, add a namespaced PRB scheduler rather than silently changing byte-budget semantics. Fix legacy cursor normalization and assertions because audit exposed real correctness weaknesses.

Use ordered maps for stable ID lookup and replay. Choose clear ownership and checked accounting before optimizing. Use uint64 counters with checked admission so queued+served never overflows. New UEs cannot import counters; removal deliberately discards that UE's state. Lifetime totals must be captured externally before removal if required.

One scheduling interface with three policy choices keeps allocation/accounting shared. Priority-based policy is explicitly custom, not standards LCP. Latency harness uses steady_clock and raw CSV so results can be independently inspected. Dependencies are versioned; GoogleTest archive is hash-pinned.

## Sprint 2
Preserve Scheduler as a legacy reference and new-data allocator. Keep one instance per direction; do not reinterpret served as delivery. Add a transactional controller, per-link HARQ and explicit accounting partitions. Prefer cumulative simulation BSR watermarks over ambiguous stale backlog snapshots. Use generation/attempt tokens rather than process ID alone. ACK is atomic delivery; do not imply support for a separate receiver whose ACK can be lost. See sprint2/design.md for exact contracts.
