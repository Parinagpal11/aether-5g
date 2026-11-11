# Supported protocol scope

## Scheduling and capacity
The original RR/PF/QoS policies and capacity function remain unchanged. Quality 0 means outage; quality 1..15 selects synthetic modulation order 2/4/6 and code rate (128+48*quality)/1024. Capacity is floor(PRBs * 144 * Qm * rate / 8), one layer. This excludes normative TBS quantization, CRC, actual MCS/CQI tables, headers, coding and RF effects. PRBs are limited to 275 as a simulation bound.

Sprint 2 uses independent UL and DL resource budgets and state. Retries have priority, rotate between UEs, preserve block byte count, and require a whole block to fit under current quality. One grant per UE/direction/slot. Policy histories reflect new-data allocations only. There is no shared TDD allocation, DCI or real UL grant timing.

## BSR abstraction
Exact cumulative admitted and retired counters plus an outstanding snapshot, timestamp and sequence. Reports are trusted simulator data and validated for internal consistency, ordering, age and bounds. They include pending bytes in outstanding. They do not implement 3GPP BSR MAC CE encoding, LCGs, quantization, triggers or timers. Stale reports prevent new UL grants but do not prevent retransmission of existing reservations.

## HARQ abstraction
Four states: idle, waiting ACK, retry ready, failed. Configurable process count (1..16), total attempts (1..32), and positive timeout. Per-UE/per-direction namespaces, monotonically increasing generations and attempt tokens reject duplicate and stale feedback. Defaults are simulation choices, not standards-derived timing. No NDI bit, RV, soft combining, coding, bundling, DTX interpretation or real receiver process. ACK and delivery are one atomic simulation event. Silence therefore does not model successful receiver delivery followed by ACK loss.

## Unsupported
RLC, PDCP, SDAP, full RRC, MAC PDU parsing/multiplexing, security, real PHY/RF, UE removal/re-registration in the new controller, multicore runtime, and standards compliance/interoperability. Legacy Scheduler still has removal; new controller registration lasts for its lifetime.
