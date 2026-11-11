# Limitations and risks

- Standards-informed simulation, not NR interoperability or full Layer 2 implementation.
- BSR is a trusted exact cumulative report, not an on-air MAC control element. Reset, quantization, LCGs and corrupted wire parsing are unsupported.
- No UE removal/re-registration in MacController; this avoids identifier-lifetime aliasing of old feedback. Legacy removal remains intact.
- No payload storage or receiver duplicate elimination. Valid ACK is the single delivery event; lost ACK after successful independent delivery is not modeled.
- Pending retries can remain pending indefinitely under permanent outage, inactivity or permanently insufficient budgets. Failed attempts are bounded; starvation time is not. The demo restores outages and has a finite drain deadline.
- Failed bytes are terminal drops; there is no RLC recovery yet.
- Greedy QoS can starve lower priorities. Retry traffic can starve new traffic. PF history excludes retransmissions and delivery outcomes.
- Copying scheduler/link state for transactional scheduling favors correctness over speed. No Sprint 2 benchmark, real-time or multicore claim.
- Configurable byte bounds limit accounting, not packet memory. Scenario event count is bounded by UE count, per-slot grants and <=100-slot feedback delay.
- No real TBS, MCS/CQI mapping, soft combining, RV/NDI, RF, shared TDD grid, RRC, security or MAC wire format.
- ASan/UBSan testing disables leak detection because LeakSanitizer cannot operate under this host's tracing setup. Leak verification remains pending on an unrestricted host.
- Clang, Docker and remote CI configurations are supplied but were not executed here. Coverage, 60-minute stress and full-project 150-test target remain future work.
