# YTTA - Yield-Tuned Tick-to-Trade Architecture

YTTA is a polyglot systems project exploring tick-to-trade architecture:
collect market/runtime signals quickly, decide quickly, and execute quickly.

## Implementations
- C++ (`cpp/`) — reference (v0 slice + Stage 1 hot-path)
- Rust (`rust/`)
- Go (`go/`)
- Java (`java/`)

## Current path (locked)

Spec-first thin vertical slice: C++ leads each milestone, then Go / Rust / Java to golden parity.

- Locked decisions: `docs/superpowers/specs/2026-07-22-v0-learning-path-design.md`
- Messages + strategy: `specs/v0_messages.md`
- Stage 1 hot-path: `docs/superpowers/specs/2026-07-22-stage1-hotpath.md` + `specs/stage1_hotpath.md`
- Stage 2 sockets: `docs/superpowers/specs/2026-07-22-stage2-sockets.md` + `specs/stage2_sockets.md`

## Repository Layout
- `docs/` — architecture and design notes
- `specs/` — protocol and behavior specs
- `shared/schemas/v0/` — message schemas
- `shared/fixtures/v0/` — tick fixture + golden event stream
- `shared/fixtures/v1/` — Stage 1 burst fixture
- `cpp/`, `rust/`, `go/`, `java/` — language-specific implementations

## Quick Start

```bash
# C++ v0 / Stage 1
make cpp
./cpp/build/ytta_v0 --mode=queued \
  --fixture shared/fixtures/v0/ticks.ndjson \
  --out /tmp/ytta_v0_out.ndjson
make cpp-test

# Burst stress (no golden byte-compare)
./cpp/build/ytta_v0 --mode=queued \
  --fixture shared/fixtures/v1/ticks_burst.ndjson \
  --out /tmp/ytta_burst_out.ndjson
```

Other languages: see each directory's README (ports follow C++ milestones).
