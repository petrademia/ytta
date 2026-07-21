# Stage 1 Hot-Path Primitives (Java Port) Implementation Plan

> **For agentic workers:** Port only. Do not change frozen specs/fixtures/C++/Go/Rust unless you find a real inconsistency — then STOP and report.

**Goal:** Idiomatic Java 17 Stage 1 parity: SPSC + object pool + LatencyProbe v2 + sync/queued modes + burst smoke, per `specs/stage1_hotpath.md`. Completes Stage 1 language parity.

**Architecture:** Producer → bounded SPSC\<Tick\> → single consumer (Strategy → Gateway → Engine). Both `sync` and `queued` must byte-match v0 golden.

**Tech Stack:** Java 17, Maven, JUnit Jupiter (test scope). Prefer no new runtime deps. Extend `com.ytta.v0.*` and `YttaV0` CLI.

## Global Constraints

- Follow `specs/stage1_hotpath.md` and `specs/v0_messages.md`
- Do **not** edit goldens, burst fixture, or cpp/go/rust
- `make java-test` covers: engine tests, v0 golden sync+queued, SPSC/pool unit tests, burst smoke
- Absolute µs not CI gates; no sockets; keep `com.ytta.App` working

## Suggested layout

| Path | Role |
|------|------|
| `java/src/main/java/com/ytta/v0/SpscQueue.java` | Bounded SPSC |
| `java/src/main/java/com/ytta/v0/ObjectPool.java` | Small free-list pool |
| `java/src/main/java/com/ytta/v0/LatencyProbe.java` | Probe v2 |
| `java/src/main/java/com/ytta/v0/Pipeline.java` | sync + queued |
| `java/src/main/java/com/ytta/v0/YttaV0.java` | `--mode=sync\|queued` |
| `java/src/test/java/com/ytta/v0/*Test.java` | SPSC, pool, golden both modes, burst |
| `java/README.md` + Makefile | Document / wire |

## Tasks

### Task 1: SPSC + pool + tests
- [ ] Commit: `feat(java): add stage1 SPSC queue and object pool`

### Task 2: Probe v2 + queued pipeline
- [ ] Commit: `feat(java): queued pipeline and stage latency probes`

### Task 3: Burst smoke + docs
- [ ] Use existing `shared/fixtures/v1/ticks_burst.ndjson`
- [ ] Commit: `test(java): add stage1 hotpath golden and burst smoke`

## Done when

- [ ] `make java-test` green
- [ ] CLI sync and queued both `cmp` to v0 golden
- [ ] Burst: drops=0, actions==N
- [ ] `make java-run` (App demo) still works
- [ ] cpp/go/rust/fixtures/specs untouched
