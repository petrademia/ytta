# YTTA Stage 2 — Process Split + Local TCP (Locked intent)

Date: 2026-07-22  
Status: **ready for C++ lead implementation** (ports after C++ lands)

## Goal

Split the in-process slice into **matching engine** and **trading client** processes communicating over **local TCP**. Same paper trading behavior and v0 golden when client replays the fixture against a local engine.

## In scope

- Two processes: `engine` (MatchingEngine + book + session) and `client` (TickSource + Strategy + OrderGateway + LatencyProbe)
- Localhost TCP only (default `127.0.0.1`, configurable port)
- Framing + message types frozen under `specs/` (prefer length-prefixed or NDJSON-over-TCP; pick one and document)
- Client drives v0 fixture → collects ack/fill/md (+ local actions) → canonical golden sort → must match `shared/fixtures/v0/golden.ndjson`
- Keep existing in-process `sync`/`queued` modes working (regression)
- Smoke: start engine, run client, tear down; CI-friendly (bound to localhost, short timeouts)
- C++ reference first; Go/Rust/Java ports **out of this handoff**

## Out of scope

- UDP / multicast / kernel bypass
- TLS, auth, discovery beyond CLI host/port
- Multi-instrument, REPLACE, live feeds, real money
- Hot-path redesign (reuse Stage 1 probes where easy)
- Absolute µs CI gates
- Language ports

## Acceptance (C++ Stage 2 done)

- [ ] Spec documents wire protocol + roles (`specs/stage2_sockets.md`)
- [ ] Engine binary listens; client connects, replays fixture, disconnects cleanly
- [ ] Two-process path byte-matches v0 golden
- [ ] In-process sync/queued golden + Stage 1 burst still pass via `make cpp-test`
- [ ] README documents how to run engine + client (manual and test harness)
- [ ] No Go/Rust/Java changes
