# Tick-to-Trade Spec (v0)

Status: aligned with locked path in `docs/superpowers/specs/2026-07-22-v0-learning-path-design.md`.

Message shapes, golden rules, and the deterministic strategy are frozen in **`specs/v0_messages.md`**. Schemas live under `shared/schemas/v0/`.

## Objective

Process a replayed tick stream and produce trading actions against one matching engine, with deterministic fills/acks comparable across C++, Go, Rust, and Java.

## Scope

- One instrument (`INSTR1`), one strategy, one book
- In-process only
- Fixture replay (no live feeds)
- Actions: `NOOP | NEW_ORDER | CANCEL`
- Integer `price` / `qty`; fixture `ts_ns` (not RFC3339 wall time)

## Inputs (tick)

See `specs/v0_messages.md` and `shared/fixtures/v0/ticks.ndjson`.

## Outputs

- Strategy `action` events
- Engine `ack` / `fill` / `md` events
- Canonical golden: `shared/fixtures/v0/golden.ndjson`

## Non-functional (v0)

- Deterministic golden: same fixture → same ordered events across languages
- Report e2e latency (p50/p99); absolute µs budgets are **not** a v0 gate

## Out of scope (v0)

- `REPLACE` / amend, multi-instrument, risk, sockets, live data, real money
