# YTTA Stage 1 — Hot-Path Primitives (Locked intent)

Date: 2026-07-22  
Status: **ready for C++ lead implementation** (ports after C++ lands)

## Goal

Harden the existing v0 tick-to-trade slice with book-shaped hot-path primitives: bounded SPSC queues, simple object reuse, monotonic timing, richer latency stats. Same trading behavior; better instrumentation and a burst workload.

## In scope

- Bounded **SPSC** queue (tick ingress at minimum; optional second queue for orders)
- **Object reuse** for hot structs (free-list / small pool) where it removes per-tick heap churn
- **Clock discipline**: document and use monotonic clocks for probes; optional cycle counter alongside ns
- **LatencyProbe v2**: e2e + per-stage (ingest / decide / execute); p50/p99; fixed capacity or histogram (no unbounded surprise growth on burst)
- **Queued pipeline mode** that still preserves v0 golden when replaying `shared/fixtures/v0/ticks.ndjson`
- **Burst fixture** under `shared/fixtures/v1/` (or `stage1/`) for stress: many ticks, report drops=0, counts, latency summary
- C++ reference first; Go/Rust/Java ports are **out of this handoff**

## Out of scope

- Sockets / multi-process (Stage 2)
- REPLACE, multi-instrument, live feeds, real money
- Absolute µs SLOs as CI fail gates
- Changing v0 golden behavior or strategy rules
- Lock-free MPMC, kernel bypass, SIMD matching

## Acceptance (C++ Stage 1 done)

- [ ] `make cpp-test` still passes v0 golden byte-compare
- [ ] Queued (or dual) pipeline path can produce the same v0 golden
- [ ] SPSC unit tests: capacity, full/empty, FIFO order
- [ ] Burst run completes with zero drops (or documented drop policy + test for “no drop” config)
- [ ] stderr latency includes e2e + stage breakdown (p50/p99)
- [ ] `specs/stage1_hotpath.md` (or this doc + messages addendum) describes queue topology and probe fields
- [ ] `cpp/README.md` documents sync vs queued vs burst invocations
