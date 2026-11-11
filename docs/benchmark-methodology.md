> Historical Sprint 1 evidence. For current Sprint 2 behavior and verification, see [Sprint 2 report](sprint2/report.md). These timings do not measure Sprint 2.

# Measurement procedure
Build Release using GCC with CMake defaults (-O3 -DNDEBUG). Run scheduler_bench. Each of three policies and UE counts 1/10/100 uses 100 PRBs, quality 7, priority 1, 1000 bytes arrival per UE per slot. First 1000 slots warm up; 10000 measured calls follow. steady_clock brackets schedule(), including grant allocation and EWMA update, excluding traffic generation and CSV output. Sorted samples use nearest-rank P50/P95/P99. Bytes count scheduled payload during measured slots; not RF throughput. Policy codes: 0 RR, 1 PF, 2 QoS.

Shared virtual Linux host, no CPU pinning, no isolation and no frequency control. Host noise can dominate tails. This is one microbenchmark run, not evidence of real-time guarantees or multicore speedup. Harness is currently standalone C++, not Google Benchmark; Google Benchmark integration, repeated-run statistics, CPU/RSS measurements and perf analysis are pending.
