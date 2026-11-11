# Verification
Baseline before edits: GCC 14.2.0 Debug build succeeded; 1/1 original CTest passed. Legacy demo output preserved. Audit found a stale cursor after shrinking a UE vector and Release assertions compiled out. Both fixed with regression coverage and runtime checks.

Current tests: GoogleTest 1.15.2, 17 discovered cases (including three policy instances), legacy test, and Python scenario integration test. Parameterized invariant tests each run 1000 slots with 100 UEs using seed 42; iterations are not counted as individual tests. Check PRB bounds/contiguity, unique grants, active state, capacity/backlog limits, byte conservation and replay. Integration test covers four deterministic JSON scenarios and invalid CLI inputs. GoogleTest assertions remain active in Release.

Raw executed results: debug-tests.txt, release-tests.txt and sanitizer-tests.txt. Full-suite target of 150 tests and 85% protocol coverage remains future work. No coverage percentage claimed. TSan, Valgrind, fuzzing and long stress run not executed for this slice. GitHub Actions is supplied but not remotely executed here.

Sanitizer host limitation: default LeakSanitizer failed during test discovery with "LeakSanitizer does not work under ptrace". Rerun uses ASAN_OPTIONS=detect_leaks=0 for both build and CTest. This disables leak detection only; address and undefined-behavior instrumentation remain enabled. Leak checking must be repeated on an unrestricted Linux host.
