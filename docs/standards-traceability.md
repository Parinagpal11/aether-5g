# Standards traceability

Consulted official ETSI publications on 2026-11-11. These links pin reference editions, not a claim of latest releases.

| Reference | Clause | Interpretation and status | Source and verification | Deviations |
|---|---|---|---|---|
| TS 38.214 V18.2.0, Release 18 | 5.1.3.1 and 5.1.3.2 | Modulation/code rate and transport block sizing are distinct concerns; approximate capacity only | src/mac/scheduler.cpp capacity; Capacity tests | Synthetic quality/rate, no normative TBS algorithm or quantization |
| TS 38.321 V18.2.0, Release 18 | 5.4.3.1 | UE uplink logical-channel prioritization is distinct from gNB inter-UE scheduling; not implemented | Qos.PriorityOverridesChannel tests only custom UE priority | No Bj buckets, LCP restrictions or logical channels; QoS policy is not LCP |

Sources:
- https://www.etsi.org/deliver/etsi_ts/138200_138299/138214/18.02.00_60/ts_138214v180200p.pdf
- https://www.etsi.org/deliver/etsi_ts/138300_138399/138321/18.02.00_60/ts_138321v180200p.pdf

TS 38.322, 38.323, 37.324 and 38.331: planned, editions and clause mapping not yet selected or reviewed. No implemented behavior claimed. Before each subsequent protocol sprint, pin an edition and trace each implemented state transition to tests.
