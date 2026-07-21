# Stage 1 Hot-Path Primitives (C++ Lead) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add SPSC queues, hot-object reuse, and richer latency probes to the C++ v0 slice without breaking golden parity; add a burst fixture workload.

**Architecture:** Keep MatchingEngine/Strategy behavior from `specs/v0_messages.md`. Introduce a bounded SPSC between tick producer and the decide/execute consumer (single consumer preserves order → same golden). LatencyProbe records e2e + stage timestamps. Burst mode replays a large fixture and asserts no queue drops under adequate capacity.

**Tech Stack:** C++20, CMake, existing `cpp/src/v0/`. May reuse ideas from `cpp/src/mem_pool.h` and `cpp/src/cycle_clock.h` (adapt into `v0`/`v1` namespaces; do not break demos).

## Global Constraints

- Do not modify `shared/fixtures/v0/golden.ndjson` or `specs/v0_messages.md` strategy rules
- `make cpp-test` v0 golden must keep passing
- No sockets, no Go/Rust/Java ports in this plan
- Absolute µs budgets are reported, not CI gates
- Prefer small, testable primitives over framework-y abstractions
- Follow `docs/superpowers/specs/2026-07-22-stage1-hotpath.md`

---

## Proposed file map

| Path | Responsibility |
|------|----------------|
| `specs/stage1_hotpath.md` | Queue topology, probe JSON fields, burst rules (can expand the locked intent doc) |
| `shared/fixtures/v1/ticks_burst.ndjson` | Large deterministic tick stream (generator script OK) |
| `cpp/src/v0/spsc_queue.hpp` | Bounded SPSC queue template |
| `cpp/src/v0/object_pool.hpp` | Tiny free-list pool for hot nodes/events |
| `cpp/src/v0/latency_probe.hpp` | Extend: stages + bounded storage / histogram |
| `cpp/src/v0/pipeline.cpp` / new `queued_pipeline.*` | Sync path unchanged; queued path for Stage 1 |
| `cpp/src/v0_main.cpp` | Flags: `--mode=sync\|queued`, `--burst` / burst fixture |
| `cpp/tests/v0_spsc_test.cpp` (or extend `ytta_v0_tests`) | Queue + pool tests |
| `cpp/tests/run_v0_golden.sh` | Still sync or queued→same golden |
| `cpp/README.md` | Document modes |

## Probe stderr shape (extend, keep `type:latency`)

```json
{"type":"latency","count":N,"p50_ns":...,"p99_ns":...,"ingest_p50_ns":...,"ingest_p99_ns":...,"decide_p50_ns":...,"decide_p99_ns":...,"execute_p50_ns":...,"execute_p99_ns":...}
```

Optional: `"drops":0`. Not part of golden compare.

## Queue topology (v1)

```
[TickSource / producer] --SPSC<Tick>--> [Strategy + OrderGateway + MatchingEngine consumer]
                                              |
                                              v
                                         golden events (sort as today)
```

Single consumer thread (or single-threaded pump that uses the queue API) so event order stays deterministic.

## Burst fixture rules

- Generate N ticks (recommend N≥1000) with deterministic seq/ts_ns
- Strategy may remain v0 seq-script for seq 1–4 then NOOP — burst is for queue/probe stress, not new alpha
- Acceptance: completes; `drops==0` with capacity ≥ N (or capacity documented + test uses sufficient capacity); prints latency summary
- No full golden byte-compare required for burst (too large); optional count assertion: actions emitted == N

---

### Task 1: Spec + SPSC + pool + unit tests

**Files:**
- Create/update: `specs/stage1_hotpath.md`
- Create: `cpp/src/v0/spsc_queue.hpp`, `cpp/src/v0/object_pool.hpp`
- Modify: `cpp/CMakeLists.txt`, tests target

- [ ] **Step 1:** Write `specs/stage1_hotpath.md` with topology, probe fields, burst rules (align with locked intent doc).
- [ ] **Step 2:** Implement bounded SPSC (`try_push`/`try_pop`, capacity power-of-two OK). Unit tests: FIFO, full→false, empty→false.
- [ ] **Step 3:** Implement small object pool; unit test allocate/free reuse without leak in a tight loop.
- [ ] **Step 4:** Commit: `feat(cpp): add stage1 SPSC queue and object pool`

---

### Task 2: LatencyProbe v2 + queued pipeline (v0 golden preserved)

**Files:**
- Modify: `cpp/src/v0/latency_probe.hpp`, `pipeline.*`, `v0_main.cpp`
- Modify: `cpp/tests/run_v0_golden.sh` if default mode changes (prefer: golden test runs `--mode=queued` OR both sync and queued)

- [ ] **Step 1:** Extend probe with ingest/decide/execute samples; summarize p50/p99 per stage.
- [ ] **Step 2:** Add queued pipeline path using SPSC; consumer runs strategy+engine; apply existing golden sort/format.
- [ ] **Step 3:** Ensure queued mode on v0 fixture `cmp`s to `shared/fixtures/v0/golden.ndjson`.
- [ ] **Step 4:** Keep sync path available (flag or default).
- [ ] **Step 5:** Commit: `feat(cpp): queued pipeline and stage latency probes`

---

### Task 3: Burst fixture + wiring + docs

**Files:**
- Create: `shared/fixtures/v1/ticks_burst.ndjson` and/or `scripts/gen_burst_fixture.py` (or `.sh`)
- Modify: Makefile / README / tests

- [ ] **Step 1:** Add burst fixture (checked in) sized for CI (~1k–10k lines; keep repo reasonable).
- [ ] **Step 2:** CLI runs burst; asserts zero drops; prints latency JSON.
- [ ] **Step 3:** `make cpp-test` includes SPSC/pool tests + v0 golden (queued) + lightweight burst smoke.
- [ ] **Step 4:** Update `cpp/README.md` and root README Stage 1 pointer.
- [ ] **Step 5:** Commit: `test(cpp): add stage1 burst fixture and hotpath smoke`

---

## Done when

- [ ] All acceptance boxes in `docs/superpowers/specs/2026-07-22-stage1-hotpath.md` checked for C++
- [ ] Go/Rust/Java **untouched**
- [ ] v0 golden unchanged and still passing

## Out of scope (reject mid-flight)

Ports to other languages, TCP/UDP, changing strategy/golden, MPMC, live feeds.
