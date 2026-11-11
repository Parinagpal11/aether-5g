# Protocol scope

All implemented scheduling policies are simulation design choices, not mandated 3GPP algorithms. Quality 0 means outage. Quality 1..15 selects synthetic modulation order 2/4/6 and code rate (128+48*quality)/1024. Capacity is floor(PRBs * 12 subcarriers * 12 data symbols * Qm * rate / 8), one layer. It excludes TBS quantization, CRC, actual MCS/CQI tables, MAC headers, coding and RF effects. Per-grant rounding means capacities need not be additive across grants. PRBs are limited to 275 as a simulation bound, not a validated bandwidth configuration.

A slot clock supports duration 1 ms / 2^mu, mu 0..4, independent of wall time. No real-time execution or deadline claims. Logical channels, bearer scheduling, BSR, MAC PDU formats and HARQ are future work. Priorities currently belong to UEs, not logical channels.
