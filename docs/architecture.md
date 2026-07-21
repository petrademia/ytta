# Architecture Overview

Status: v0 path **locked** — see `docs/superpowers/specs/2026-07-22-v0-learning-path-design.md`.

YTTA decomposes tick-to-trade into three stages:

1. Ingest: receive ticks/events with minimal overhead.
2. Decide: apply strategy/risk logic under latency discipline.
3. Execute: place/cancel orders with low jitter.

## v0 data flow (in-process)

```
[TickSource] → [Strategy] → [OrderGateway] → [MatchingEngine] → [Fill/Ack]
                    ↑                              │
                    └──────── market data ─────────┘
```

`LatencyProbe` timestamps stage boundaries and reports coarse e2e latency.

## Components (v0)

| Component | Role |
|-----------|------|
| TickSource | Replay fixture ticks |
| Strategy | Emit `NOOP \| NEW_ORDER \| CANCEL` |
| OrderGateway | Forward orders to the engine |
| MatchingEngine | One instrument, one book; fills/rejects |
| Market data | Engine publishes book/trade updates back to strategy |
| LatencyProbe | Stage + e2e latency reporting |

## Cross-cutting (thicken after v0 slice works)

- Time synchronization and monotonic clocks
- Lock/contention management
- Allocation minimization in hot paths
- Deterministic observability (latency histograms, event tracing)

## Constraints (v0)

- One instrument, one strategy, one book
- Integer price/qty; fixture replay only; paper execute
- In-process only (sockets come after v0)
- C++ is the milestone reference; other languages match golden behavior
