# YTTA v0 Learning Path (Locked)

Date: 2026-07-22  
Status: **locked**

## Goal

Learn C++, Go, Rust, and Java through a shared tick-to-trade slice, loosely informed by *Building Low Latency Applications with C++* (Packt). Success is comparable learning across four languages plus a coherent architecture story, not a line-for-line book clone.

## Locked decisions

| Topic | Decision |
|-------|----------|
| Primary goal | Thin end-to-end pipeline first, then harden systems primitives where measurement forces it |
| Sequencing | Spec-first lockstep slice: shared behavior/message spec → implement in all four → thicken |
| Market data | Recorded fixture replay (paper execute locally) |
| Book role | C++ leads each milestone as reference; Go/Rust/Java reach behavioral parity before the next milestone |
| First milestone | Thin vertical slice: minimal exchange + one trading client |
| Transport (v0) | In-process only (threads/queues). Sockets later |
| Overall approach | Spec-first lockstep, not chapter-by-chapter 4-way ports |

## Explicitly rejected (for now)

- Primitives-first across all four languages before any pipeline
- C++-only until the book is finished
- Live public feeds for v0
- Local sockets / UDP for v0
- Full matching-engine milestone before a working client path
- Chapter lockstep ports of every Packt chapter into all four languages

## v0 scope

### In

- Components: `TickSource` → `Strategy` → `OrderGateway` → `MatchingEngine` → `Fill/Ack`, with market data back to strategy; plus `LatencyProbe`
- One instrument, one strategy, one book
- Actions: `NOOP | NEW_ORDER | CANCEL`
- Integer price/qty; sequence numbers; monotonic timestamps
- Fixture replay tick source
- In-process wiring only
- Coarse e2e latency reporting (p50/p99); no full tracing requirement
- Shared golden outputs: same fixture → same ordered fills/acks across languages

### Out

- Live feeds, real money
- Sockets / multi-process
- `REPLACE` (or amend)
- Multi-instrument, risk limits, persistence, UI
- Full market-making / liquidity-taking suites
- Cross-language IPC
- Absolute latency SLOs (reporting required; beating N µs is not a v0 gate)

## Milestone workflow

1. Freeze/update shared spec + fixture under `specs/`
2. Implement reference slice in `cpp/`
3. Capture C++ golden output
4. Port Go / Rust / Java to golden parity
5. Only then start the next stage

## Acceptance: language X done for v0

- [ ] Builds and runs one command that replays the shared fixture
- [ ] Emits the same ordered actions/acks/fills as the C++ golden (canonical form OK)
- [ ] Implements TickSource, Strategy, OrderGateway, MatchingEngine, LatencyProbe (real interfaces; stubs allowed only where behavior is still correct)
- [ ] Strategy can emit `NEW_ORDER` and `CANCEL` (not NOOP-only)
- [ ] MatchingEngine maintains one book and returns fills/rejects per spec
- [ ] Reports e2e latency for the fixture run
- [ ] Smoke test: fixture in → golden out
- [ ] Language README documents run + test commands

## Stages after v0

1. **Hot-path primitives** — queues, pooling/arenas where measured, clock discipline, better histograms; same slice, burst fixtures
2. **Process split + sockets** — engine and client over local TCP; still paper / one instrument
3. **Algo thickening** — MD fan-out, second strategy mode (simple MM *or* taker), richer order ops, stage latency breakdown; still replay/sim

## Assumptions

- Absolute microsecond targets are educational later, not a v0 merge gate
- Idiomatic Go/Java/Rust implementations beat slavish C++ ports
- The Packt book informs *what to thicken next*, not the day-to-day port schedule
