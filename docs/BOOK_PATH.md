# YTTA Book Path

Primary text: *Building Low Latency Applications with C++* (Sourav Ghosh, Packt).

Workflow: read chapters → implement under `cpp/src/` → run the milestone tests → tick `docs/PROGRESS.md`.

| ID | Book | Implement | Verify |
|----|------|-----------|--------|
| M0 | Ch 1–3 | Notes + scaffold | `make test-m0` |
| M1 | Ch 4 | SPSC, pool, clock, thread utils | `make test-m1` |
| M2 | Ch 5–6 | Book + matching engine + in-process gateway | `make test-m2` |
| M3 | Ch 7 | Localhost TCP framing + gateway/engine I/O | `make test-m3` |
| M4 | Ch 8–9 | Client pipeline + golden fixture replay | `make test-m4` |
| M5 | Ch 10 | One MM **or** one liquidity-taking strategy | `make test-m5` |
| M6 | Ch 11–12 | LatencyProbe histograms + report p50/p99 | `make test-m6` |

## M0 — Concepts (Ch 1–3)

- Goals: understand latency-sensitive apps; set up the repo.
- Files: `cpp/src/smoke/smoke.cpp`
- Done when: `make test-m0` passes.

## M1 — Building blocks (Ch 4)

- Goals: bounded SPSC queue, object pool, monotonic clock helper, basic thread utility.
- Files: `cpp/src/m1/spsc_queue.hpp`, `object_pool.hpp`, `mono_clock.hpp`, `thread_utils.hpp`
- Done when: `make test-m1` passes.

## M2 — Matching engine (Ch 5–6)

- Goals: one-instrument paper book; NEW/CANCEL; maker/taker fills.
- Contract: `specs/messages.md`
- Files: `cpp/src/m2/types.hpp`, `book.hpp`, `matching_engine.hpp`, `order_gateway.hpp` (+ `.cpp` as needed)
- Done when: `make test-m2` passes.

## M3 — Participant I/O (Ch 7)

- Goals: length-prefixed framing; localhost TCP engine/client helpers.
- Files: `cpp/src/m3/net/framing.hpp`, `tcp.hpp` (+ `.cpp`)
- Done when: `make test-m3` passes.

## M4 — Client path (Ch 8–9)

- Goals: replay `shared/fixtures/v0/ticks.ndjson`; emit canonical golden stream.
- Files: `cpp/src/m4/pipeline.hpp`, `tick_source.hpp`, `strategy.hpp` (+ mains later)
- Done when: `make test-m4` passes (unit/pipeline + golden script).

## M5 — Strategy (Ch 10)

- Goals: implement **either** simple market-making **or** liquidity-taking (pick one).
- Files: `cpp/src/m5/strategy.hpp`
- Done when: `make test-m5` passes.

## M6 — Measure (Ch 11–12)

- Goals: record e2e samples; print p50/p99; optional burst fixture.
- Files: `cpp/src/m6/latency_probe.hpp`
- Done when: `make test-m6` passes (reporting required; no hard µs SLO).

## Ports (optional, later)

`go/`, `rust/`, `java/` are placeholders. Do not block C++ milestones on them.
