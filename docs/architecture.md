# Architecture

Sprint 2 adds MacController, HarqManager, FeedbackModel and a bidirectional scenario runner. The original byte-budget and PRB scheduler APIs, sources and tests are unchanged.

MacController owns two Plane instances. Each has its own Scheduler, UE Link map, retry cursor and per-slot scheduling guard. Each Link owns a HarqManager and independent admitted/delivered/failed counters. Queue bytes remain inside the original scheduler; its served counter is used only as unique-first-transmission accounting, never as delivery.

New grants transition queued bytes to HARQ reservations. ACK/failure completion moves reservations to delivered/failed exactly once. Retries do not enqueue or readmit bytes. UL reports update cumulative admission; pending grants are already absent from the scheduler queue.

begin_slot is strictly increasing and expires waiting processes first. The caller then supplies due feedback, arrivals/configuration/reports and schedules each direction at most once. ACK at a deadline loses to timeout. Invalid configuration and reports do not change state. Registration stages both planes; scheduling stages one plane before commit, preserving state if staging throws. Other methods perform validation before mutation.

FeedbackModel has no queue or mutable RNG. Integer mixing over seed, UE, direction, process, generation, attempt and slot provides reproducible outcomes independent of evaluation order. The scenario runner owns bounded-horizon feedback events and writes optional trace CSV. It stops admitting after traffic_slots and drains to a hard iteration bound.

No threading, shared mutable global state, payload ownership, receiver stack or real PHY is introduced. See sprint2/design.md for the contract and limitations.
