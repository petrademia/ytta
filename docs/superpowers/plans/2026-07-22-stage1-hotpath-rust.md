# Stage 1 Hot-Path Primitives (Rust Port) Implementation Plan

> **For agentic workers:** Port only. Do not change frozen specs/fixtures/C++/Go unless you find a real inconsistency — then STOP and report.

**Goal:** Idiomatic Rust Stage 1 parity: SPSC + object pool + LatencyProbe v2 + sync/queued modes + burst smoke, per `specs/stage1_hotpath.md`.

**Architecture:** Producer → bounded SPSC\<Tick\> → single consumer (Strategy → Gateway → Engine). Both `sync` and `queued` must byte-match v0 golden.

**Tech Stack:** Rust 2021, prefer stdlib only. Extend `rust/src/v0/` and `rust/src/bin/ytta_v0.rs`.

## Global Constraints

- Follow `specs/stage1_hotpath.md` and `specs/v0_messages.md`
- Do **not** edit goldens, burst fixture, or cpp/go/java
- `make rust-test` covers: engine tests, v0 golden sync+queued, SPSC/pool unit tests, burst smoke
- Absolute µs not CI gates; no sockets

## Suggested layout

| Path | Role |
|------|------|
| `rust/src/v0/spsc.rs` | Bounded SPSC |
| `rust/src/v0/pool.rs` | Small free-list pool |
| `rust/src/v0/probe.rs` | LatencyProbe v2 |
| `rust/src/v0/pipeline.rs` | sync + queued |
| `rust/src/bin/ytta_v0.rs` | `--mode=sync\|queued` |
| `rust/tests/v0_golden.rs` (+ unit tests) | Both modes + burst |
| `rust/README.md` + Makefile | Document / wire |

## Tasks

### Task 1: SPSC + pool + tests
- [ ] Commit: `feat(rust): add stage1 SPSC queue and object pool`

### Task 2: Probe v2 + queued pipeline
- [ ] Commit: `feat(rust): queued pipeline and stage latency probes`

### Task 3: Burst smoke + docs
- [ ] Use existing `shared/fixtures/v1/ticks_burst.ndjson`
- [ ] Commit: `test(rust): add stage1 hotpath golden and burst smoke`

## Done when

- [ ] `make rust-test` green
- [ ] CLI sync and queued both `cmp` to v0 golden
- [ ] Burst: drops=0, actions==N
- [ ] cpp/go/java/fixtures/specs untouched
