# Implementation status: Sprint 1 checkpoint

## Sprint 0 summary
Audited every starter source, test, CMake and CI file. Baseline GCC Debug build and original test passed (1/1). Original simulator output: slot 0 UE1=1000; slot 1 UE2=1000; slot 2 UE3=900, UE1=100; slot 3 UE2=200, UE1=700; slot 4 empty. No RAN protocol implementation existed. Architecture, risk register and reference scope now documented.

## Sprint 1 summary
Implemented explicit PRB grants, configurable UE state, RR/PF/custom priority policies, slot clock, synthetic channel-sensitive capacity, deterministic simulation CLI, four JSON workloads, fairness and latency measurement. Preserved byte-budget demo. Fixed legacy vector-shrink cursor and Release-disabled assertions.

Files changed: CMakeLists.txt, README.md, src/scheduler.cpp, tests/scheduler_test.cpp and CI workflow. New modules: include/aether/mac/scheduler.hpp, src/mac/scheduler.cpp, simulator/main.cpp, tests/mac_test.cpp, tests/scenario_test.py, benchmarks/scheduler_bench.cpp, tools/run_scenario.py, scenarios/*.json, Dockerfile and docs/*.

Decisions: single owner, stable UE ID ordering, shared accounting for policy variants, custom capacity model explicitly separated from normative TBS. Test and performance evidence in adjacent logs and reports. 17 GoogleTest cases plus two CTest entries; randomized iterations are not additional tests.

Sprint 1 functional foundation is implemented. Full acceptance is incomplete: no Google Benchmark integration, coverage measurement or verified Clang/container runs. This checkpoint is not the multi-sprint final platform.

## Prioritized backlog
1. Sprint 2: distinguish scheduled, in-flight, ACKed and failed bytes; DL/UL queue ownership, BSR abstraction, seeded PHY outcomes and bounded HARQ processes. Test timeout/retry/drop conservation before upper-layer integration.
2. Sprint 3: pin TS 38.322 edition; TM then UM parsing, segmentation/reassembly and wraparound; AM status/retransmission/timers.
3. Sprint 4: pin PDCP/SDAP editions; sequence state, reorder timers and QFI-to-DRB configuration.
4. Sprint 5: bounded queues, jthread lifecycle, backpressure, reference-output oracle and TSan stress.
5. Sprint 6: fuzz parsers, error injection, coverage and full protocol regression suite.
6. Sprint 7: Google Benchmark, perf, allocation/lock profiling, CPU/RSS, multicore scaling and Prometheus/Grafana.
7. Sprint 8: verify Docker/CI/compiler matrix, long stress and release packaging; tag only once acceptance passes.

Optional AI triage, DPDK and NUMA are deferred until core correctness is established.
