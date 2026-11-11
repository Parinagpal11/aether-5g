# Concurrency
Single-threaded reference mode only. MacController, HarqManager, Scheduler and the clock require serialized ownership. Const inspection references must not be used across concurrent mutation. FeedbackModel is immutable and stateless; no concurrent-runtime claims are made.

Each scheduling call stages a copy of one direction and commits on success. Registration stages both directions. This is exception safety, not thread synchronization. Accounting and feedback processing do not use callbacks or background threads. The scenario feedback queue is bounded by finite supported UE count and feedback delay, not a production work queue. Bounded producer/consumer queues and jthread shutdown remain Sprint 5 work. TSan was not run for Sprint 2.
