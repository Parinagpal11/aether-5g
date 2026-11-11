# MASTER ENGINEERING SPECIFICATION: AETHER-5G

## Role and mission

You are acting as a Principal Wireless Systems Software Engineer, 5G RAN Protocol Architect, Senior C++20 Engineer, and Software Verification Lead.

Your responsibility is to design, implement, test, benchmark, document, and deliver an enterprise-grade open-source engineering project named:

**AETHER-5G: A Multithreaded 5G NR Layer 2 Software and Verification Platform**

This project is being developed as a serious engineering portfolio project for a Qualcomm Wireless R&D Software Engineer position focused on Test Base Station software, 5G/6G Radio Access Networks, Layer 2 protocols, real-time-sensitive C++ software, Linux, multicore processing, and performance optimization.

The project must demonstrate the ability to translate selected 3GPP specifications into working software, implement concurrent data-plane processing, debug and optimize Linux applications, and maintain rigorous software verification practices.

Do not build a superficial demonstration, frontend-heavy application, or collection of disconnected toy algorithms.

Build an integrated, testable, maintainable systems software platform.

## 1. Existing repository and starting instructions

An existing starter repository named `aether-5g-day1.zip` may be provided.

Before making changes:

1. Extract and inspect the repository.
2. Read its README, CMake configuration, source files, tests, and CI workflows.
3. Identify existing functionality and limitations.
4. Execute its current build and test commands.
5. Record the baseline results.
6. Preserve working functionality.
7. Refactor existing code only when necessary.
8. Never claim existing functionality without verification.

The initial repository contains a simplified byte-budget scheduling simulator. Treat it as an architectural starting point, not a standards-compliant MAC implementation.

If no repository is available, initialize a new C++20 project with equivalent foundations.

## 2. Product vision

AETHER-5G must simulate a configurable 5G NR gNodeB Layer 2 processing environment.

The platform should support:

- Multiple simulated User Equipments (UEs).
- Uplink and downlink traffic generation.
- Configurable radio resource availability.
- 5G NR MAC scheduling.
- Selected RLC functionality.
- Selected PDCP functionality.
- SDAP QoS flow mapping.
- A simulated PHY interface.
- A simplified RRC configuration interface.
- Concurrent packet processing.
- Deterministic scenario execution.
- Protocol verification.
- Performance profiling.
- Automated regression testing.
- Failure injection.
- Operational metrics and observability.

The platform does not need real RF hardware.

It must not falsely claim to be a complete commercial gNodeB, 3GPP-certified implementation, or interoperable base station.

The focus is a standards-informed, executable Layer 2 research and verification platform.

## 3. Technical stack

### Core implementation

- C++20
- Linux as the primary deployment and benchmarking platform
- CMake
- Ninja
- GCC and Clang
- Standard Template Library
- RAII and modern memory management
- `std::jthread`
- `std::mutex`
- `std::condition_variable`
- `std::atomic`
- Bounded producer-consumer queues
- Thread-safe resource ownership

### Testing

- GoogleTest
- Google Benchmark
- AddressSanitizer
- UndefinedBehaviorSanitizer
- ThreadSanitizer
- Valgrind where appropriate
- Deterministic integration tests
- Property-based or randomized invariant testing
- Fuzz testing for PDU parsers

### Performance engineering

- Linux perf
- gdb
- Flamegraphs
- CPU affinity experiments
- Cache behavior analysis
- Memory allocation profiling
- Contention profiling
- Throughput benchmarking
- P50/P95/P99 latency analysis

### Infrastructure

- Docker
- GitHub Actions
- Python 3.12
- Prometheus
- Grafana
- Structured logging
- Versioned configuration files

### Optional advanced capabilities

- DPDK
- NUMA-aware execution
- Hardware performance counters
- AI-assisted failure triage

Optional capabilities must never block the core release.

## 4. Standards and protocol scope

Use the following specifications as primary references:

- 3GPP TS 38.321: NR MAC
- 3GPP TS 38.322: NR RLC
- 3GPP TS 38.323: NR PDCP
- 3GPP TS 37.324: SDAP
- 3GPP TS 38.331: RRC
- 3GPP TS 38.214: Physical-layer data procedures

Use official specifications and clearly identify the release and version consulted.

Create `docs/standards-traceability.md` containing:

- Specification identifier.
- Version.
- Relevant clause.
- Requirement interpretation.
- Implementing source module.
- Corresponding test.
- Implementation status.
- Known deviations and limitations.

Do not claim compliance with a complete specification when only selected features are implemented.

For every protocol component, explicitly distinguish:

1. Specification-derived behavior.
2. Simplified simulation assumptions.
3. Unsupported functionality.
4. Future extensions.

## 5. High-level architecture

Implement the following logical pipeline:

Traffic Generator / UE Simulator

↓

SDAP: QoS Flow Mapping

↓

PDCP: Sequence Numbering and Reordering

↓

RLC: Segmentation and Reassembly

↓

MAC: Scheduling and Resource Allocation

↓

Simulated PHY: Slots, Channel Quality, Resource Grid, Transmission Outcomes

For uplink traffic, implement the corresponding receive-side processing path.

The platform should also contain:

- RRC configuration adapter.
- Scenario controller.
- Simulation clock.
- Multithreaded runtime.
- Metrics subsystem.
- Verification engine.
- Fault-injection subsystem.
- Benchmark runner.

Use explicit interfaces between layers.

Avoid circular dependencies and unnecessary shared mutable state.

Separate the protocol domain model from the simulation infrastructure.

## 6. MAC subsystem

Implement the MAC scheduler as a central engineering component.

### Required functionality

- UE registration and removal.
- Per-UE scheduling state.
- Downlink buffer occupancy.
- Uplink buffer-status reports.
- Logical channel priority.
- Resource block allocation.
- Slot-based scheduling.
- Configurable channel quality.
- Configurable resource availability.
- Scheduling grants.
- Throughput accounting.
- Scheduling fairness measurements.
- Resource exhaustion handling.

### Scheduling algorithms

Implement:

1. Round-robin.
2. Proportional-fair.
3. QoS-aware scheduling.

Use a common scheduler interface so policies can be swapped without modifying the simulator.

### Design requirements

The scheduler must:

- Never allocate more resources than available.
- Never schedule an invalid UE.
- Never allocate a negative resource quantity.
- Handle empty queues.
- Handle inactive UEs.
- Handle traffic bursts.
- Handle resource exhaustion.
- Preserve accounting invariants.
- Support deterministic replay.

Document the relationship between allocated PRBs, assumed modulation/coding parameters, and simulated transport-block capacity.

Do not treat a fixed number of bytes per PRB as a complete NR transport-block calculation.

Start with a documented abstraction, then introduce more standards-informed behavior.

### HARQ

Implement a simplified HARQ state model supporting:

- Process identifiers.
- Pending transmissions.
- ACK/NACK outcomes.
- Retransmission eligibility.
- Configurable maximum attempts.
- Timeout and failure states.

Explicitly document which HARQ timing and PHY interactions are simulated rather than standards-complete.

## 7. RLC subsystem

Implement selected RLC functionality with separate transmit and receive entities.

### Required modes

- Transparent Mode (TM).
- Unacknowledged Mode (UM).
- Acknowledged Mode (AM).

### Required functionality

- PDU construction and parsing.
- Sequence number handling.
- Segmentation.
- Reassembly.
- Transmit and receive buffers.
- Reordering behavior.
- Duplicate handling where applicable.
- AM retransmission buffers.
- Status reporting.
- Relevant timers.
- Error handling.

### Verification scenarios

- Single SDU transmission.
- Segmented SDU transmission.
- Out-of-order arrival.
- Missing segments.
- Duplicate PDUs.
- Retransmission recovery.
- Sequence number wraparound.
- Buffer exhaustion.
- Malformed PDU handling.

Implement incrementally: TM first, then UM, then AM.

Every supported behavior must have deterministic tests.

## 8. PDCP subsystem

Implement selected PDCP user-plane functionality.

### Required features

- PDCP entity configuration.
- Sequence numbering.
- COUNT/state handling where applicable.
- PDU construction and parsing.
- Reordering.
- Duplicate detection.
- Delivery to upper layers.
- Configurable reordering timers.
- State reset and cleanup.

Security-related behavior must be accurately scoped.

Do not claim production-grade ciphering or integrity protection unless actual algorithms, key handling, and corresponding tests have been implemented.

If security is not implemented, document the omission clearly.

## 9. SDAP subsystem

Implement:

- QoS Flow Identifier representation.
- Data Radio Bearer representation.
- QFI-to-DRB mapping.
- Configurable mapping rules.
- QoS flow classification.
- Mapping updates.
- Invalid mapping handling.
- Per-flow traffic accounting.

Provide test cases with multiple QoS flows mapped to different bearers.

Show how QoS policy influences scheduling decisions through the simulated data plane.

## 10. PHY simulation

Build a deterministic simulated physical layer.

The PHY emulator must support:

- Configurable NR numerology.
- Slot timing.
- Configurable bandwidth and resource-grid assumptions.
- Per-UE channel quality.
- Resource availability.
- Simulated transmission success and failure.
- Configurable error probability.
- Channel degradation scenarios.
- Throughput accounting.
- HARQ feedback generation.

The PHY emulator must not pretend to implement full modulation, coding, synchronization, or RF transmission.

Use a seeded random generator for reproducible stochastic behavior.

Keep the channel model configurable and document its limitations.

## 11. RRC configuration interface

Implement a simplified configuration adapter rather than a complete RRC protocol stack.

It should allow the simulation controller to:

- Register and remove UEs.
- Configure bearers.
- Configure QoS flows.
- Configure scheduling policies.
- Update selected radio parameters.
- Trigger simplified connection lifecycle scenarios.

Do not claim full TS 38.331 support.

Separate configuration events from data-plane packet processing.

## 12. Multithreaded execution engine

This is a critical requirement.

Build a concurrent Linux runtime using modern C++.

### Execution components

- Traffic generation worker.
- Scheduling worker.
- Protocol processing workers.
- Simulated PHY worker.
- Metrics collection worker.

### Requirements

- Bounded queues.
- Explicit ownership of packet buffers.
- Backpressure.
- Graceful shutdown.
- No uncontrolled detached threads.
- No data races.
- No use-after-free.
- No unbounded memory growth.
- Deterministic single-threaded reference mode.
- Multithreaded performance mode.

Use the deterministic reference mode as the correctness oracle.

When comparing concurrent and reference execution, compare protocol invariants and defined outputs, not nondeterministic timing or event ordering.

### Performance experiments

Benchmark:

- One worker.
- Two workers.
- Four workers.
- Eight workers where hardware permits.

Measure:

- Throughput.
- CPU utilization.
- Queue wait time.
- Processing latency.
- Lock contention.
- Memory usage.
- Scheduling overhead.

Investigate false sharing, unnecessary allocations, and synchronization bottlenecks.

Do not introduce lock-free structures unless profiling demonstrates a measurable benefit.

## 13. Traffic and workload generation

Implement reproducible workloads.

### Required scenarios

1. Idle network.
2. Single UE, constant traffic.
3. Ten UEs, balanced traffic.
4. One hundred UEs, mixed traffic.
5. Bursty downlink traffic.
6. Uplink-heavy traffic.
7. Mixed uplink/downlink traffic.
8. High-priority QoS flow.
9. Poor-channel UE.
10. Resource saturation.
11. Packet loss and retransmissions.
12. Long-running stress scenario.

Each scenario must define:

- UE count.
- Traffic rate.
- Packet size distribution.
- Simulation duration.
- Scheduling policy.
- Channel configuration.
- Random seed.
- Expected invariants.

Store scenarios in version-controlled YAML or JSON files.

## 14. Testing strategy

Testing is a first-class engineering deliverable.

### Unit testing

Test individual components, including:

- Scheduling algorithms.
- Resource allocation.
- Packet queues.
- RLC segmentation.
- RLC reassembly.
- PDCP ordering.
- SDAP mapping.
- HARQ state transitions.
- Configuration validation.

### Integration testing

Test complete packet paths:

- Downlink SDAP to PHY.
- Uplink PHY to SDAP.
- Multiple concurrent UEs.
- QoS scheduling.
- Retransmission recovery.
- Shutdown with queued traffic.

### Concurrency testing

Run ThreadSanitizer configurations and targeted stress tests.

Verify:

- No data races.
- No deadlocks under tested conditions.
- Correct shutdown.
- Bounded memory usage.
- Valid queue accounting.

### Fault injection

Inject:

- Packet drops.
- Duplicate packets.
- Out-of-order delivery.
- Malformed PDUs.
- Worker delays.
- Queue saturation.
- Invalid configuration.
- Simulated channel failures.

### Test targets

Aim for:

- At least 150 meaningful tests across the full project.
- At least 85% line coverage for core protocol and scheduling logic.
- Passing sanitizer configurations.
- Deterministic regression scenarios.
- Reproducible benchmark results.

These are engineering targets, not existing results.

Do not inflate test counts with trivial or duplicate tests.

## 15. Benchmarking and performance engineering

Implement a repeatable benchmark suite.

### Required metrics

- Aggregate throughput.
- Per-UE throughput.
- Scheduler execution time.
- Packet processing latency.
- P50 latency.
- P95 latency.
- P99 latency.
- Packet drop rate.
- Retransmission rate.
- Jain's fairness index.
- CPU utilization.
- Memory consumption.
- Queue occupancy.
- Worker utilization.

### Benchmark dimensions

Compare:

- Round-robin vs proportional-fair vs QoS-aware.
- One UE vs ten UEs vs one hundred UEs.
- Single-threaded vs multithreaded.
- Low traffic vs saturation.
- Good channel vs degraded channel.
- Low packet loss vs high packet loss.

### Performance targets

Establish initial targets:

- Support 100 simulated UEs.
- Demonstrate bounded resource usage.
- Complete a 60-minute stress run.
- Investigate P99 scheduling latency below 100 microseconds under a defined configuration.
- Demonstrate and explain multicore scaling behavior.

Do not fabricate performance improvements.

Do not claim real-time guarantees based only on average latency.

Record benchmark hardware, compiler, optimization flags, operating system, workload, and measurement methodology.

If performance targets are missed, explain why and profile the bottleneck.

## 16. Observability

Expose runtime metrics through Prometheus-compatible instrumentation.

Build Grafana dashboards showing:

- Active UEs.
- Downlink throughput.
- Uplink throughput.
- Scheduler latency.
- Per-UE fairness.
- Retransmission rate.
- Queue depth.
- Packet drops.
- CPU utilization.
- Memory consumption.

Provide structured logs containing:

- Timestamp.
- Component.
- UE identifier where applicable.
- Slot identifier.
- Event type.
- Severity.
- Correlation identifier where applicable.

Avoid excessive logging in performance-critical paths.

## 17. CI/CD

Implement GitHub Actions workflows for:

- Debug builds.
- Release builds.
- GCC builds.
- Clang builds.
- Unit tests.
- Integration tests.
- Sanitizer tests.
- Formatting checks.
- Static analysis.
- Coverage reporting.
- Benchmark smoke tests.

Use separate sanitizer configurations when required.

Pin or otherwise control dependency versions.

Make CI failures actionable.

Avoid treating flaky tests as acceptable.

## 18. AI-assisted debugging extension

After the core platform works, implement an optional engineering assistant.

The assistant should ingest:

- CI failures.
- Compiler errors.
- Sanitizer reports.
- Structured logs.
- Benchmark regressions.

It should generate:

- Failure summaries.
- Likely root causes.
- Relevant source references.
- Suggested debugging steps.
- Potential regression explanations.

All suggestions must be traceable to observed evidence.

The assistant must not silently modify source code or claim to have fixed failures without rerunning tests.

Keep this extension isolated from the C++ data plane.

## 19. Repository structure

Use a maintainable structure similar to:

aether-5g/
- CMakeLists.txt
- CMakePresets.json
- README.md
- LICENSE
- include/aether/
  - common/
  - mac/
  - rlc/
  - pdcp/
  - sdap/
  - phy/
  - runtime/
- src/
  - common/
  - mac/
  - rlc/
  - pdcp/
  - sdap/
  - phy/
  - runtime/
- simulator/
- scenarios/
- tests/
  - unit/
  - integration/
  - protocol/
  - concurrency/
  - fuzz/
- benchmarks/
- tools/
- observability/
- docs/
- .github/workflows/
- Dockerfile

Organize source code by responsibilities.

Avoid giant files, circular dependencies, and duplicated protocol logic.

## 20. Engineering quality requirements

Use:

- Modern C++20.
- RAII.
- Const correctness.
- Explicit ownership.
- Strong types for identifiers and units.
- Clear error handling.
- Dependency injection where appropriate.
- Small cohesive interfaces.
- Deterministic tests.
- Minimal unnecessary dependencies.
- Consistent naming.
- Clear documentation.

Avoid:

- Raw owning pointers.
- Global mutable state.
- Undefined behavior.
- Unbounded queues.
- Busy-wait loops without justification.
- Detached worker threads.
- Hidden timing assumptions.
- Unnecessary abstractions.
- Placeholder implementations presented as finished functionality.

Follow a consistent formatting and static-analysis configuration.

## 21. Sprint plan

### Sprint 0: Repository audit and architecture

Deliver:

- Existing repository assessment.
- Working baseline.
- Architecture document.
- Component boundaries.
- Risk register.
- Standards scope.
- Initial CI.

### Sprint 1: MAC scheduling foundation

Deliver:

- UE models.
- Simulated slot clock.
- Resource allocation model.
- Round-robin scheduling.
- Proportional-fair scheduling.
- Deterministic test scenarios.
- Scheduler invariants.
- Initial benchmarks.

### Sprint 2: Uplink and downlink integration

Deliver:

- DL traffic pipeline.
- UL buffer-status model.
- UL grant scheduling.
- Resource accounting.
- CQI-based abstraction.
- Simplified HARQ.
- End-to-end MAC tests.

### Sprint 3: RLC implementation

Deliver:

- TM.
- UM.
- AM.
- Segmentation and reassembly.
- Retransmission state.
- Protocol test suite.

### Sprint 4: PDCP and SDAP

Deliver:

- PDCP sequence handling.
- Reordering.
- Duplicate detection.
- SDAP QoS mapping.
- Bearer configuration.
- Integrated packet processing.

### Sprint 5: Concurrent runtime

Deliver:

- Multithreaded workers.
- Bounded queues.
- Backpressure.
- Thread-safe shutdown.
- Sanitizer testing.
- Single-thread vs multicore benchmarks.

### Sprint 6: Verification and fault injection

Deliver:

- Protocol integration tests.
- Failure injection.
- Stress tests.
- Fuzzing.
- Coverage report.
- Regression suite.

### Sprint 7: Profiling and observability

Deliver:

- Performance benchmarks.
- Profiling reports.
- Performance optimizations.
- Prometheus metrics.
- Grafana dashboards.
- Reproducible measurements.

### Sprint 8: Release engineering

Deliver:

- Clean installation instructions.
- Docker environment.
- CI pipeline.
- Architecture documentation.
- Protocol traceability.
- Benchmark report.
- Demonstration scenarios.
- Tagged release.

### Sprint 9: Optional advanced extensions

Implement only after the core release is verified:

- DPDK experiments.
- NUMA-aware optimization.
- Expanded protocol behavior.
- AI-assisted CI triage.
- Additional scheduler algorithms.

## 22. Required documentation

Create:

1. `README.md`: Product overview, quick start, features, limitations.
2. `docs/architecture.md`: Component architecture and interfaces.
3. `docs/protocol-scope.md`: Supported protocol behavior.
4. `docs/standards-traceability.md`: 3GPP clause mapping.
5. `docs/concurrency.md`: Thread model and synchronization.
6. `docs/testing.md`: Test strategy and execution.
7. `docs/benchmark-methodology.md`: Measurement procedure.
8. `docs/performance-results.md`: Verified benchmark results.
9. `docs/limitations.md`: Unsupported functionality.
10. `docs/engineering-decisions.md`: Design tradeoffs.
11. `docs/release-checklist.md`: Release criteria.
12. `docs/STATUS.md`: Current implementation status.

Documentation must match the actual implementation.

## 23. Definition of done

A feature is complete only when:

- Its requirements are documented.
- Its code is implemented.
- It builds successfully.
- Its unit tests pass.
- Its relevant integration tests pass.
- Its error paths are handled.
- Its concurrency assumptions are documented.
- Its supported standards behavior is traceable.
- Its limitations are recorded.
- It does not break existing functionality.

A sprint is complete only when all mandatory acceptance criteria pass.

If anything remains incomplete, label it explicitly.

## 24. Final deliverables

Produce:

- Complete source repository.
- Working Linux build.
- Reproducible simulation executable.
- Automated test suite.
- CI/CD workflows.
- Benchmark suite.
- Performance report.
- Architecture documentation.
- Standards traceability matrix.
- Scenario configurations.
- Optional Grafana dashboards.
- Docker development environment.
- Release notes.
- Demonstration instructions.

The final demonstration should show:

1. Starting the simulator.
2. Registering simulated UEs.
3. Configuring a scheduling policy.
4. Generating uplink and downlink traffic.
5. Observing resource allocation.
6. Exercising selected RLC/PDCP/SDAP processing.
7. Injecting channel degradation.
8. Observing retransmission and recovery.
9. Comparing scheduler performance.
10. Running automated tests and benchmarks.

## 25. Non-negotiable verification rules

Never fabricate:

- Test results.
- Benchmark numbers.
- Coverage.
- Standards compliance.
- Performance improvements.
- Hardware support.
- Production readiness.

If a command cannot execute, state that it was not executed.

If a dependency or environment is unavailable, identify the blocker and provide a reproducible way to verify it elsewhere.

Separate:

- Implemented.
- Tested.
- Benchmark-verified.
- Partially implemented.
- Planned.
- Blocked.

Do not use words such as "fully compliant," "production-ready," or "enterprise-grade" as evidence substitutes.

## 26. How to execute this assignment

Work autonomously through the engineering milestones when the environment permits.

Do not stop after writing a plan.

Do not respond with only pseudocode or a proposed architecture.

Implement actual compilable C++ code, tests, build configuration, and documentation.

At the start:

1. Audit the starter repository.
2. Run baseline tests.
3. Create the architecture and implementation backlog.
4. Begin Sprint 1.
5. Implement the first working vertical slice.
6. Run its tests.
7. Record results.

At the end of each sprint, provide:

**Sprint summary**
- Features implemented.
- Files changed.
- Architecture decisions.
- Tests executed.
- Tests passed and failed.
- Performance results, if measured.
- Known limitations.
- Next sprint priorities.

Continue to the next sprint when possible, without sacrificing verification quality.

Do not spend excessive time on cosmetic improvements before the core software works.

## 27. Success criteria

The ultimate goal is to demonstrate competence in the engineering areas relevant to Qualcomm's Wireless R&D Test Base Station team:

- Modern C++ development.
- Linux systems programming.
- Multicore and multithreaded software.
- Wireless protocol implementation.
- 3GPP specification interpretation.
- MAC/RLC/PDCP/SDAP concepts.
- Uplink and downlink scheduling.
- Performance-sensitive data-plane design.
- Software debugging and profiling.
- CI/CD and verification.
- Engineering documentation.
- Reliable software delivery.

The final repository must withstand technical review by an experienced systems software engineer.

A reviewer should be able to inspect the code, understand the protocol scope, execute the tests, reproduce benchmark results, and evaluate the design tradeoffs.

**Begin now by auditing the supplied repository and implementing Sprint 1. Prioritize working software and verified evidence over presentation.**