# Release criteria

## Sprint 2 gate
The nine requested acceptance criteria and evidence are recorded in [sprint2/report.md](sprint2/report.md). This is a locally verified simulation release, not deployment/standards certification. Leak detection is excluded from the host-limited sanitizer run.

## Full platform release still pending
- [ ] RLC TM/UM/AM and protocol error-path tests
- [ ] PDCP/SDAP and packet-level pipeline
- [ ] Concurrent bounded workers, backpressure and shutdown verification
- [ ] TSan, parser fuzzing and measured coverage
- [ ] 150 meaningful tests and 85% core coverage target
- [ ] 60-minute stress and multicore profiling
- [ ] Verified remote CI, compiler matrix and Docker execution
- [ ] LeakSanitizer on an unrestricted Linux host
- [ ] Standards-scope review for every supported protocol layer
- [ ] Tagged release and deployment documentation
