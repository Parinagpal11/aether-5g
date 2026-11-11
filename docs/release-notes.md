> Historical Sprint 1 evidence. For current Sprint 2 behavior and verification, see [Sprint 2 report](sprint2/report.md). These timings do not measure Sprint 2.

# Sprint 1 source checkpoint

Baseline audit and working MAC scheduling vertical slice. Added explicit PRB allocation and RR/PF/QoS policies without replacing the original byte-budget executable. Fixed legacy vector-shrink invalid access and Release-disabled assertions.

Verification: 19/19 CTest entries pass in Debug, Release, and ASan/UBSan with leak detection disabled due to host ptrace restrictions. Original demo output unchanged. Nine scheduler benchmark cases recorded. No remote CI, Docker, Clang, TSan, coverage or long-stress result claimed.

Demo: build using README, run scenarios/single.json and scenarios/saturation.json through tools/run_scenario.py, compare rr/pf/qos with aether_slots, run CTest and scheduler_bench. Channel/priority variation is currently library-test driven. HARQ recovery and upper protocol layers cannot yet be demonstrated.
