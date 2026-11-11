# Concurrency
Single-threaded reference mode only. Scheduler and clock are not thread-safe. One owner must serialize all calls; references returned by state() must not be held across concurrent mutations. No detached threads or shared globals. Bounded queues, jthreads, shutdown semantics and concurrent/reference comparison are Sprint 5 work; no TSan race-free claim is made.
