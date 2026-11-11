# Sprint 2 implementation and release report

Date: 2026-11-11. Base: `a27e313` from `Parinagpal11/aether-5g`. Local branch: `feat/sprint2-ul-dl-harq`. Changes have not been pushed to GitHub.

## Release readiness

**All nine requested Sprint 2 acceptance criteria pass locally within the documented simulation scope.** ASan/UBSan pass with leak detection disabled because this environment prevents LeakSanitizer from running. Leak detection, remote CI, alternate compilers and deployment validation are not claimed. This completes the scoped functional gate, not the full multi-sprint platform or production/interoperability validation.

| # | Acceptance criterion | Status | Evidence |
|---|---|---|---|
| 1 | Existing Sprint 1 regression suite passes | PASS | baseline 19/19; all original entries also pass in final 50-entry suites; legacy source/tests unchanged |
| 2 | UL and DL schedule independently | PASS | Controller.DirectionsAndProcessNamespacesAreIndependent; separate scheduler/PRB/process namespaces |
| 3 | BSR validation and accounting | PASS | Bsr tests cover ordering, freshness, pending grants, delayed reports, zero reports and retirement watermarks |
| 4 | HARQ ACK/NACK/retransmission transitions | PASS | Harq tests and Controller tests cover retries, terminal failure, process reuse and timeout |
| 5 | No duplicate delivery or accounting violations | PASS under tested model | ACK-once and stale-token tests; independent full ledger; per-slot conservation checks |
| 6 | Deterministic 100-UE bidirectional scenario | PASS | All three policies replay; CLI full traces and summaries compare exactly |
| 7 | Debug and Release builds | PASS | 50/50 entries in each final build |
| 8 | AddressSanitizer and UBSan | PASS, leak detection excluded | 50/50 entries, no address/UB finding; default LeakSanitizer discovery failure recorded separately |
| 9 | Accurate supported/unsupported documentation | PASS | README, scope, architecture, standards mapping, limitations and contracts updated; relative links checked |

## Executed tests and failures

- Baseline: 19 passed, 0 failed.
- HARQ vertical slice: 8 selected tests passed, 0 failed.
- Controller/BSR/HARQ slice: 19 selected tests passed, 0 failed.
- Initial full integration: 46 passed, 0 failed.
- Final Debug: 50 passed, 0 failed.
- Final Release: 50 passed, 0 failed.
- Final ASan/UBSan with leak detection disabled: 50 passed, 0 failed.
- Default sanitizer build: **failed** during test discovery because LeakSanitizer cannot operate under ptrace. This is a failed verification attempt, not a passing leak test. CTest did not run on that attempt.

Every individual test name and result is retained in [Debug](debug-tests.txt), [Release](release-tests.txt) and [sanitizer](sanitize-tests.txt) logs. See [default sanitizer failure](sanitize-build-default.txt), [baseline](baseline.txt), [initial integration](debug-initial.txt), and [executed commands](commands.md). The suite has 47 GoogleTest cases plus three CTest wrapper entries. Replays and loop iterations are not inflated into extra test counts.

## Deterministic demonstration

Command: `python3 tools/run_bidirectional.py scenarios/bidirectional.json ./build-release/aether_bidirectional`.

100 UEs, RR, 200 arrival slots, seed 42, 100 PRBs per direction, NACK threshold 250/1000 and silence threshold 30/1000, with temporary outages. Debug and Release summaries match byte-for-byte.

Completed in 535 simulated slots; 1913 transmissions including 498 retries. Trace digest: `12120825357111578152`. These are simulation counters, not performance benchmark results.

| Direction | Admitted bytes | Queued | In flight | Delivered | Failed |
|---|---:|---:|---:|---:|---:|
| DL | 1,280,000 | 0 | 0 | 1,245,633 | 34,367 |
| UL | 1,115,200 | 0 | 0 | 1,083,599 | 31,601 |

Raw result: [scenario-result.json](scenario-result.json).

## Accounting invariants

1. Per UE/direction: admitted = queued + in_flight + delivered + failed.
2. In flight equals waiting-ACK plus retry-ready process bytes only.
3. New transmission reserves bytes once; retransmission never readmits or dequeues them again.
4. ACK or terminal failure settles a block once. Wrong state/generation/attempt/time-window feedback changes no counter.
5. Scheduler served is unique scheduled bytes, never successful delivery; served = admitted - queued.
6. Cumulative UL reports add only previously unseen arrivals. Pending and already completed bytes remain excluded from new backlog.
7. Outstanding queued plus pending stays within configured bounds; lifetime counters and deadlines reject overflow.
8. Each direction independently stays within its PRB budget, with non-overlapping contiguous grants and at most one grant per UE per slot.

## Main design decisions

Keep both legacy schedulers intact. Use two PRB scheduler instances for new traffic and a controller for feedback/accounting. Stage scheduling state before commit for exception safety. Use exact cumulative reports to make delayed-report accounting explicit. Scope HARQ to per-UE/per-direction process namespaces with generation and attempt tokens. ACK is the single modeled delivery event. Use integer-derived deterministic feedback and finite traffic plus bounded drain.

## Remaining limitations

No packet payload, separate receiver delivery/ACK-loss model, wire-format BSR, LCGs, real TBS, NDI/RV/soft combining, actual NR timing, shared TDD grid, UE removal/re-registration in the new controller, RLC/PDCP/SDAP, RF, security or concurrent runtime. A retry can remain pending indefinitely under permanent outage or insufficient resources; the supplied scenario restores service and fails if drain exceeds its bound. Strict priority and retry traffic may starve other data. No Sprint 2 performance or coverage claim. See [design](design.md) and [limitations](../limitations.md).

## Every added or modified file

Paths below are relative to the repository root. Original scheduling sources and original test files are unchanged.

| File | Change |
|---|---|
| `.clang-format` | Added |
| `CMakeLists.txt` | Modified |
| `README.md` | Modified |
| `docs/STATUS.md` | Modified |
| `docs/architecture.md` | Modified |
| `docs/benchmark-methodology.md` | Modified |
| `docs/concurrency.md` | Modified |
| `docs/engineering-decisions.md` | Modified |
| `docs/limitations.md` | Modified |
| `docs/performance-results.md` | Modified |
| `docs/protocol-scope.md` | Modified |
| `docs/release-checklist.md` | Modified |
| `docs/release-notes.md` | Modified |
| `docs/sprint2/baseline.txt` | Added |
| `docs/sprint2/commands.md` | Added |
| `docs/sprint2/debug-build.txt` | Added |
| `docs/sprint2/debug-initial.txt` | Added |
| `docs/sprint2/debug-tests.txt` | Added |
| `docs/sprint2/design.md` | Added |
| `docs/sprint2/release-build.txt` | Added |
| `docs/sprint2/release-configure.txt` | Added |
| `docs/sprint2/release-tests.txt` | Added |
| `docs/sprint2/report.md` | Added |
| `docs/sprint2/sanitize-build-default.txt` | Added |
| `docs/sprint2/sanitize-build.txt` | Added |
| `docs/sprint2/sanitize-configure.txt` | Added |
| `docs/sprint2/sanitize-tests.txt` | Added |
| `docs/sprint2/scenario-debug.json` | Added |
| `docs/sprint2/scenario-result.json` | Added |
| `docs/standards-traceability.md` | Modified |
| `docs/testing.md` | Modified |
| `include/aether/mac/bidirectional.hpp` | Added |
| `include/aether/mac/controller.hpp` | Added |
| `include/aether/mac/harq.hpp` | Added |
| `include/aether/phy/feedback.hpp` | Added |
| `scenarios/bidirectional.json` | Added |
| `simulator/bidirectional.cpp` | Added |
| `src/mac/bidirectional.cpp` | Added |
| `src/mac/controller.cpp` | Added |
| `src/mac/harq.cpp` | Added |
| `src/phy/feedback.cpp` | Added |
| `tests/bidirectional_cli_test.py` | Added |
| `tests/bidirectional_test.cpp` | Added |
| `tests/controller_test.cpp` | Added |
| `tests/harq_test.cpp` | Added |
| `tests/phy_test.cpp` | Added |
| `tools/run_bidirectional.py` | Added |
