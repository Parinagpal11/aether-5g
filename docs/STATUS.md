# Implementation status: Sprint 2

Sprint 2 implements independent UL/DL scheduling, validated exact simulation BSRs, bounded HARQ, deterministic feedback, and bidirectional verification. The runtime remains single-threaded. The 19-entry Sprint 1 regression suite and its implementation remain unchanged.

See [Sprint 2 report](sprint2/report.md) for the nine acceptance gates, all modified files, executed verification commands, raw passing/failing logs and local release readiness. The full multi-sprint platform is not complete.

## Implemented
- Separate UL/DL scheduler, accounting and HARQ namespaces.
- Cumulative UL reports with sequence/timestamp/counter/bound validation and pending-grant reservation.
- Idle/waiting/retry-ready/failed HARQ state, ACK/NACK, timeout, bounded attempts and token reuse protection.
- Queued/in-flight/delivered/failed partitions checked by internal invariants and an independent test ledger.
- Deterministic feedback model, temporary outages and finite-traffic 100-UE scenarios with drain.
- New API, CLI, JSON runner and regression/integration tests.

## Verification boundaries
GCC Debug, Release, ASan and UBSan are exercised locally. LeakSanitizer is blocked by tracing; sanitizer rerun disables leak detection only. No remote CI, Docker, Clang, macOS, TSan, coverage percentage, Sprint 2 benchmark or long stress claim.

## Next priorities
Sprint 3: RLC TM, then UM, then AM, with pinned standards editions and parser/segmentation/reassembly tests. Later: PDCP/SDAP, packet payloads, concurrent bounded workers, observability, profiling and full release acceptance. Preserve Sprint 2 accounting contracts when introducing real packet delivery and RLC retransmission ownership.
