# Decisions
Preserve legacy demo/API, add a namespaced PRB scheduler rather than silently changing byte-budget semantics. Fix legacy cursor normalization and assertions because audit exposed real correctness weaknesses.

Use ordered maps for stable ID lookup and replay. Choose clear ownership and checked accounting before optimizing. Use uint64 counters with checked admission so queued+served never overflows. New UEs cannot import counters; removal deliberately discards that UE's state. Lifetime totals must be captured externally before removal if required.

One scheduling interface with three policy choices keeps allocation/accounting shared. Priority-based policy is explicitly custom, not standards LCP. Latency harness uses steady_clock and raw CSV so results can be independently inspected. Dependencies are versioned; GoogleTest archive is hash-pinned.
