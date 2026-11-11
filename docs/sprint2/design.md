# Sprint 2 contracts and decisions

## State ownership and phases
One MacController lives for a simulation. Register a nonzero unique UE in both directions. There is no controller removal API. DL and UL qualities/priorities/active flags are independent. enqueue_downlink may run before the first slot; all scheduling, reporting and feedback require begin_slot.

Slot order: begin_slot(N) -> expire waiting HARQ -> consume feedback -> apply reports/arrivals/configuration -> schedule each direction at most once. Slot numbers must strictly increase. Skipping slots expires outstanding deadlines at the next begin_slot. Near-uint64-max slot values that cannot represent a deadline are rejected atomically.

## Accounting
For each UE/direction: admitted = queued + in_flight + delivered + failed. The checker subtracts each partition to avoid overflow hiding violations. Scheduler served = admitted - queued = in_flight + delivered + failed. Pending is the sum of waiting and retry-ready process bytes, excluding idle and failed records. Lifetime admission cannot exceed uint64; outstanding queued plus pending has a configurable bound.

A new grant moves bytes from queue to pending. A retry changes only attempt/deadline, not byte partitions. ACK moves bytes from pending to delivered; terminal NACK/timeout moves them to failed. Duplicate or stale feedback cannot change counters because state, process, generation, attempt and acceptance window must all match. Event identity is the complete (direction, UE, process, generation, attempt) tuple. Feedback fields are trusted simulator events, not authenticated network messages.

## BSR contract
BufferStatusReport = sequence, generated_slot, admitted, retired, outstanding. At generation, outstanding = admitted - retired, including in-flight bytes. Retired is a nondecreasing watermark of delivered plus failed bytes known to the reporting UE. It may lag current controller retirement but may not exceed it. Cumulative admission may not decrease. Sequence must be positive and strictly increase; generation timestamps may be equal but not decrease, be in the future, or exceed bsr_max_age at receipt. The report must fit max_outstanding_bytes. Report age <= limit is fresh.

Queue increment is report.admitted - previously_seen_admitted. A delayed snapshot therefore cannot reintroduce reserved or completed bytes. Reports claiming malformed counters/age/order/limits throw before mutation. No inference is made about unreported UL arrivals. Stale accepted reports leave queues accounted but disable new grants until refreshed; existing retries can proceed.

This richer cumulative report is deliberately nonstandard. It resolves ambiguity of stale exact-byte snapshots without claiming support for real quantized BSR formats.

## HARQ
One manager per UE per direction. Each has 1..16 processes, 1..32 total attempts and a positive timeout. Default 4/3/4. Idle or failed process slots may be reused with a new generation; generation overflow makes that process unavailable rather than wrapping. Failed state retains the old block for inspection but contributes zero pending bytes.

Feedback valid only when sent_slot < now < deadline and the token matches a waiting process. ACK returns one completion; NACK makes retry-ready or failed. expire returns newly failed bytes only and is idempotent. Retry must occur strictly after the previous send slot. Retry-ready state persists until resources arrive; it has no independent abandonment timer.

Retries are served first in rotating UE order, at most one per UE/direction/slot. New traffic cannot bypass a waiting retry for the same UE. Payload byte size stays fixed; required PRBs are recomputed with the existing capacity abstraction. A whole retry must fit. New-data ordering delegates to an unchanged Scheduler per direction, preserving RR/PF/QoS behavior for eligible traffic.

## PHY
FeedbackModel uses fixed unsigned-integer mixing, not implementation-dependent distributions. Silence threshold is applied first; NACK threshold applies to present feedback. Delay is positive; a delay at or beyond timeout intentionally yields stale feedback. There is no independent successful decode before ACK. Hash probabilities do not adapt to channel quality; quality controls capacity/outage, and feedback error thresholds are separate model inputs.

## Scenario
1..100 UEs, 1..1000 arrival slots, 100 PRBs/direction/slot, quality 7 except a temporary outage for every tenth UE. DL arrives at 48+8*(UE mod 5) bytes/slot; UL at 32+8*(UE mod 7). Priority is 1+(UE mod 4). Reports are fresh every slot. Feedback delay is limited to 100 slots in the reusable runner (CLI uses 1). The drain limit is 20,000 extra slots. Completed output requires both directions empty and all scheduled feedback consumed. Summary has exact totals, retry/transmission counts and a defined FNV-1a trace digest; tests also compare complete CSV traces, not only the digest.

The event queue has at most 2*UE_count*delay pending transmissions under one grant per UE/direction/slot. No infinite random retry loop exists: attempts are bounded and traffic generation stops.
