# YTTA - Yield-Tuned Tick-to-Trade Architecture

YTTA is a polyglot systems project exploring tick-to-trade architecture:
collect market/runtime signals quickly, decide quickly, and execute quickly.

## Implementations
- C++ (`cpp/`) — v0 reference slice
- Rust (`rust/`)
- Go (`go/`)
- Java (`java/`)

## Current path (locked)

Spec-first thin vertical slice: C++ leads each milestone, then Go / Rust / Java to golden parity.

- Locked decisions: `docs/superpowers/specs/2026-07-22-v0-learning-path-design.md`
- Messages + strategy: `specs/v0_messages.md`
- C++ slice plan: `docs/superpowers/plans/2026-07-22-v0-cpp-slice.md`

## Repository Layout
- `docs/` — architecture and design notes
- `specs/` — protocol and behavior specs
- `shared/schemas/v0/` — message schemas
- `shared/fixtures/v0/` — tick fixture + golden event stream
- `cpp/`, `rust/`, `go/`, `java/` — language-specific implementations

## Quick Start

```bash
# C++ v0 slice (fixture → golden)
make cpp
./cpp/build/ytta_v0 \
  --fixture shared/fixtures/v0/ticks.ndjson \
  --out /tmp/ytta_v0_out.ndjson
make cpp-test
```

Other languages: see each directory's README (parity ports come after C++ milestones).
