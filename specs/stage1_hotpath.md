# YTTA Stage 1 — Hot-Path Primitives (behavior)

Status: frozen for C++ Stage 1 lead. Aligns with `docs/superpowers/specs/2026-07-22-stage1-hotpath.md`.

## Goal

Same v0 trading behavior (`specs/v0_messages.md`); add bounded SPSC ingress, small object reuse, and richer latency probes. Burst fixture stresses the queue without requiring a large golden.

## Queue topology

```
[TickSource / producer] --SPSC<Tick>--> [single consumer: Strategy → OrderGateway → MatchingEngine]
                                              |
                                              v
                                    golden events (canonical sort as v0)
```

- Bounded SPSC: `try_push` / `try_pop`; full push returns false (drop counted); empty pop returns false.
- Capacity should be a power of two when using a ring buffer.
- **Single consumer** (or single-threaded pump of the SPSC API) so event order stays deterministic → v0 golden byte parity.
- Optional: pool allocates reusable nodes/slots for hot objects; not required on the golden path if unused.

## Pipeline modes

| Mode | Behavior |
|------|----------|
| `sync` | Direct call path (v0 original): for each tick, strategy+engine inline |
| `queued` | Producer `try_push`es ticks into SPSC; consumer drains and processes in FIFO order |

Both modes on `shared/fixtures/v0/ticks.ndjson` must emit identical golden NDJSON after sort.

## Latency probe (stderr only)

Monotonic clock: `std::chrono::steady_clock` for probe deltas (not fixture `ts_ns`).

```json
{"type":"latency","count":N,"p50_ns":...,"p99_ns":...,"ingest_p50_ns":...,"ingest_p99_ns":...,"decide_p50_ns":...,"decide_p99_ns":...,"execute_p50_ns":...,"execute_p99_ns":...,"drops":0}
```

| Field | Meaning |
|-------|---------|
| `count` | Ticks processed |
| `p50_ns` / `p99_ns` | End-to-end (ingest start → execute end) |
| `ingest_*` | Load/enqueue tick into the pipeline |
| `decide_*` | Strategy decision |
| `execute_*` | Gateway + matching engine |
| `drops` | Failed `try_push` count |

Not part of golden compare. Absolute µs values are **not** CI fail gates.

## Burst fixture

- Path: `shared/fixtures/v1/ticks_burst.ndjson`
- N ≥ 1000 deterministic ticks (`seq`/`ts_ns` strictly increasing; `INSTR1`)
- Strategy remains v0 seq-script (seq 1–4 active, then `NOOP`) — burst is for queue/probe stress
- Acceptance: run completes; `drops==0` with capacity ≥ N (or capacity documented in test); prints latency JSON; action count == N
- No full golden byte-compare for burst

## Out of scope

Sockets, MPMC, live feeds, REPLACE, changing v0 golden/strategy, language ports.
