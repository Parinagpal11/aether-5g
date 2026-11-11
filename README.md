# AETHER-5G

Standards-informed C++20 Layer 2 research platform. **Sprint 1 MAC foundation**, not a complete NR MAC, interoperable base station, or certified implementation.

## Build and run
Requires Linux, GCC 12+ or recent Clang, CMake 3.20+, Ninja and Python 3. GoogleTest 1.15.2 downloads at configure time with a SHA-256 pin; for offline builds provide `-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/path/to/googletest`.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j 4
ctest --test-dir build --output-on-failure
./build/aether_slots rr 10 100 50 500
python3 tools/run_scenario.py scenarios/saturation.json ./build/aether_slots
./build/aether_sim # preserved original byte-budget demo
```

`aether_slots POLICY UEs slots PRBs bytes_per_UE_per_slot` emits deterministic grant CSV and a JSON fairness summary to stderr. Policies: `rr`, `pf`, `qos`. CLI uses equal channel quality and priority; heterogeneous channels and priorities are exercised through the library API and tests. Resource budget is shared among DL queues. Slot duration is 1 ms in the CLI; the clock API supports numerologies 0..4.

## Implemented
UE registration/removal, active state, bounded integer accounting, explicit contiguous PRB grants, channel-dependent approximate capacity, round-robin, proportional-fair and priority-first scheduling, slot clock, Jain fairness, four JSON scenarios, randomized invariant tests and scheduler latency harness.

## Scope
No UL, BSR encoding, HARQ, RLC, PDCP, SDAP, real PHY, RRC, packet parsers or concurrent workers yet. Queue occupancy represents bytes, not packet storage. Served bytes mean scheduler allocation, not successful air-interface delivery. Full requirements are preserved in `docs/master-specification.md`; status and backlog are in `docs/STATUS.md`.

## Benchmarks and sanitizers
```sh
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j 4
./build-release/scheduler_bench
cmake -S . -B build-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DAETHER_SANITIZER=address,undefined
cmake --build build-sanitize -j 4
ctest --test-dir build-sanitize --output-on-failure
```
See `docs/performance-results.md` for measured results and limitations. No real-time guarantees.
