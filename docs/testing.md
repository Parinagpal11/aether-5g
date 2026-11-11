# Verification

The Sprint 2 audit checked remote commit a27e313. All 19 existing CTest entries passed before implementation. Those source files and tests were preserved. Historical Sprint 1 logs remain in this directory; current evidence is under sprint2/.

The current suite has 50 CTest entries: 47 GoogleTest cases (17 original plus 30 new, including policy instances), the original legacy test, the original Python scenario test, and the new bidirectional CLI replay test. Loop iterations and repeated runs are not counted as distinct tests.

New tests cover HARQ transitions, process exhaustion/reuse, stale and duplicate feedback, timeout boundaries, configuration/overflow rejection, BSR ordering/freshness/retirement, independent UL/DL namespaces, pending reservations, full byte accounting against an independent ledger, and deterministic PHY outcomes. Three 100-UE scenarios are repeated for RR/PF/QoS. The CLI test compares complete CSV traces and summaries and independently checks per-slot resource bounds and terminal conservation.

Build/test commands and results: [Sprint 2 report](sprint2/report.md). Normal ASan/UBSan build failed at test discovery because LeakSanitizer cannot operate under host tracing. The recorded passing run uses ASAN_OPTIONS=detect_leaks=0 and UBSAN_OPTIONS=halt_on_error=1. CMake also sets -fno-sanitize-recover=all. This verifies address and undefined-behavior checks, not leak freedom.

No TSan, coverage, fuzzing, Valgrind, 60-minute stress, remote CI or Clang result is claimed. The full-project targets of 150 meaningful tests and 85% core line coverage remain future milestones.
