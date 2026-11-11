> Historical Sprint 1 evidence. For current Sprint 2 behavior and verification, see [Sprint 2 report](sprint2/report.md). These timings do not measure Sprint 2.

# Verified performance results

Measured 2026-11-11 on shared virtual Linux 6.18.57 x86_64; /proc/cpuinfo reports AMD EPYC 9V74. CPU isolation and physical topology unavailable (lscpu could not read sysfs). GCC 14.2.0, C++20, Release -O3 -DNDEBUG. One warmup/measurement run per case; 1000 warmup and 10000 measured calls. See benchmark-methodology.md and raw benchmark.csv.

| Policy | UEs | P50 µs | P95 µs | P99 µs |
|---|---:|---:|---:|---:|
| RR | 1 | 0.06 | 0.07 | 0.09 |
| RR | 10 | 0.21 | 0.231 | 0.35 |
| RR | 100 | 1.041 | 1.082 | 1.532 |
| PF | 1 | 0.08 | 0.09 | 0.14 |
| PF | 10 | 0.38 | 0.4 | 0.421 |
| PF | 100 | 6.37 | 7.852 | 50.165 |
| QoS | 1 | 0.08 | 0.081 | 0.13 |
| QoS | 10 | 0.371 | 0.411 | 0.65 |
| QoS | 100 | 6.36 | 7.291 | 26.079 |

All measured P99 values in this one run were below 100 µs. This does not establish a real-time guarantee, repeated-run stability, full-pipeline latency or multicore scalability. PF/QoS sort and repeated map lookups are candidates for later profiling; no bottleneck or speedup is claimed without profiling. CPU utilization, RSS, lock contention, coverage and 60-minute stress were not measured.
