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

## Sprint 2 additions

The same TS 38.321 V18.2.0 Release 18 edition is retained. These are conceptual correspondences, not claims of complete clause implementation.

| Clause | Interpretation | Module | Tests | Status and deviations |
|---|---|---|---|---|
| 5.3.2, 5.3.2.2 | DL HARQ entity/process concepts | src/mac/harq.cpp and controller.cpp | Harq.AckDeliversOnce; Harq.NackRetryKeepsBytesAndRejectsOldAttempt; Controller.DirectionsAndProcessNamespacesAreIndependent | Simplified ACK/NACK state machine; no NDI, RV, soft combining or receiver decoding |
| 5.4.2 | UL HARQ concept | controller.cpp separate UL manager | Controller.DirectionsAndProcessNamespacesAreIndependent; FullAccountingAgainstIndependentLedger | Separate UL namespace; simulated timing and bounded attempts, not full UL procedure |
| 5.4.5 | Buffer status informs UL allocation | controller.cpp report_uplink | Bsr.InvalidReportsAreAtomic; DelayedReportDoesNotReadmitPendingOrDeliveredBytes; FreshnessStopsNewGrantsButNotRetries | Exact cumulative simulation report; no standard CE encoding, quantization, LCGs or reporting triggers |

Token generations, attempt matching, cumulative admission/retirement fields, fixed timeout, independent UL/DL grids and hash-based PHY are engineering choices. No standards-derived values are claimed for them. TS 38.214 capacity remains the documented Sprint 1 approximation.
