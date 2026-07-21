# Tick-to-Trade Spec (v0)

Status: aligned with locked path in `docs/superpowers/specs/2026-07-22-v0-learning-path-design.md`.

## Objective

Process a replayed tick stream and produce trading actions against one matching engine, with deterministic fills/acks comparable across C++, Go, Rust, and Java.

## Scope

- One instrument, one strategy, one book
- In-process only
- Fixture replay (no live feeds)
- Actions: `NOOP | NEW_ORDER | CANCEL`

## Inputs (tick)

- Tick timestamp (monotonic or fixture-provided)
- Instrument identifier
- Side / price / size (integer price and qty)
- Sequence number

## Outputs (action)

- Action: `NOOP | NEW_ORDER | CANCEL`
- Action timestamp
- Optional reason code

## Engine outputs

- Ack / reject for each order request
- Fill events when the book matches
- Market-data updates consumed by the strategy

## Non-functional (v0)

- Deterministic golden: same fixture → same ordered fills/acks across languages
- Report e2e latency (p50/p99); absolute µs budgets are **not** a v0 gate
- No dropped-sequence handling requirement beyond what the fixture exercises

## Out of scope (v0)

- `REPLACE` / amend, multi-instrument, risk, sockets, live data, real money
