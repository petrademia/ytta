# Stage 1 Hot-Path Primitives (Go Port) Implementation Plan

> **For agentic workers:** Port only. Do not change frozen specs/fixtures/C++ unless you find a real inconsistency — then STOP and report.

**Goal:** Idiomatic Go Stage 1 parity: SPSC + object pool + LatencyProbe v2 + sync/queued modes + burst smoke, matching C++ behavior contracts in `specs/stage1_hotpath.md`.

**Architecture:** Same topology as C++: producer → bounded SPSC\<Tick\> → single consumer (Strategy → Gateway → Engine). v0 golden must still byte-match in both `sync` and `queued` modes.

**Tech Stack:** Go 1.22, stdlib only. Extend `go/internal/v0/` and `go/cmd/ytta_v0`.

## Global Constraints

- Follow `specs/stage1_hotpath.md` and `specs/v0_messages.md`
- Do **not** edit `shared/fixtures/v0/golden.ndjson`, `shared/fixtures/v1/ticks_burst.ndjson`, or C++/Rust/Java
- `make go-test` must cover: engine tests, v0 golden (sync + queued), SPSC/pool unit tests, burst smoke (`drops==0`, actions==N)
- Absolute µs not CI gates
- No sockets / Stage 2

## Suggested layout

| Path | Role |
|------|------|
| `go/internal/v0/spsc.go` | Bounded SPSC |
| `go/internal/v0/pool.go` | Small free-list pool |
| `go/internal/v0/probe.go` | LatencyProbe v2 (stages + drops) |
| `go/internal/v0/pipeline.go` | sync + queued modes |
| `go/cmd/ytta_v0/main.go` | `--mode=sync\|queued`, burst fixture support |
| `go/internal/v0/*_test.go` | SPSC, pool, golden both modes, burst |
| `go/README.md` + root `Makefile` | `go-test`, optional `go-run-v0` flags |

## Tasks

### Task 1: SPSC + pool + tests

- [ ] Commit: `feat(go): add stage1 SPSC queue and object pool`

### Task 2: Probe v2 + queued pipeline (v0 golden both modes)

- [ ] stderr latency JSON field set matches `specs/stage1_hotpath.md`
- [ ] Commit: `feat(go): queued pipeline and stage latency probes`

### Task 3: Burst smoke + Makefile/README

- [ ] Use existing `shared/fixtures/v1/ticks_burst.ndjson` (do not regenerate unless broken)
- [ ] Commit: `test(go): add stage1 hotpath golden and burst smoke`

## Done when

- [ ] `make go-test` green
- [ ] CLI sync and queued both `cmp` to v0 golden
- [ ] Burst: drops=0, actions=2048 (or fixture N)
- [ ] cpp/rust/java/fixtures/specs untouched
