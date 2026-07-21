# Stage 2 Process Split + Local TCP (C++ Lead) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Split C++ tick-to-trade into engine + client over localhost TCP while preserving v0 golden and Stage 1 in-process tests.

**Architecture:** Client owns TickSource + Strategy + LatencyProbe; sends orders over TCP; receives ack/fill/md. Engine owns MatchingEngine + book; applies NEW/CANCEL; publishes events. Framing and payload shapes frozen in `specs/stage2_sockets.md`. After a fixture run, client emits the same canonical golden NDJSON as today.

**Tech Stack:** C++20, CMake, POSIX sockets (or Asio only if already present — prefer raw POSIX/`sys/socket` for learning). No new heavy frameworks.

## Global Constraints

- Follow `docs/superpowers/specs/2026-07-22-stage2-sockets.md`
- Do not change `shared/fixtures/v0/golden.ndjson` or `specs/v0_messages.md` strategy rules
- Keep `make cpp-test` green for in-process Stage 0/1 paths
- Localhost only; no Go/Rust/Java ports
- Absolute µs not CI gates
- YAGNI: one connection, one instrument, blocking or simple poll loop OK

---

## Proposed file map

| Path | Responsibility |
|------|----------------|
| `specs/stage2_sockets.md` | Roles, framing, message enums/fields, lifecycle |
| `cpp/src/v0/net/tcp.hpp` (+ `.cpp` if needed) | Listen/connect/read/write helpers |
| `cpp/src/v0/net/framing.hpp` | Encode/decode wire messages |
| `cpp/src/v0/engine_server.*` | Engine process loop |
| `cpp/src/v0/client_session.*` | Client OrderGateway over TCP |
| `cpp/src/v0_engine_main.cpp` | `ytta_engine` binary |
| `cpp/src/v0_client_main.cpp` or extend `v0_main` | `ytta_client` / `--role=client` |
| `cpp/tests/run_v0_tcp_golden.sh` | Spawn engine, run client, cmp golden, kill engine |
| `cpp/CMakeLists.txt` / `cpp/README.md` / Makefile | Targets + docs |

## Recommended wire protocol (freeze in spec; tweak only with rationale)

**Framing:** `uint32` big-endian length + UTF-8 JSON payload (one object).  
**Client → engine:** `order` messages (`NEW` / `CANCEL`) matching v0 field semantics.  
**Engine → client:** `ack`, `fill`, `md` (same fields as golden types).  
**Lifecycle:** engine listen → client connect → replay → client sends optional `goodbye` or closes → engine exits on disconnect **or** stays up for multiple runs (document which; prefer stay-up + test kills process).

Keep strategy **local to client** (not on engine). Engine does not see ticks.

## Topology

```
[TickSource] → [Strategy] → [TcpOrderGateway] ----TCP----> [EngineServer / MatchingEngine]
                     ↑                                         │
                     └──────── ack / fill / md <───────────────┘
```

In-process modes remain available without TCP.

---

### Task 1: Spec + framing + socket helpers + unit tests

- [ ] Write `specs/stage2_sockets.md`
- [ ] Framing round-trip tests (encode/decode)
- [ ] Commit: `docs(spec): freeze stage2 localhost TCP protocol` + `feat(cpp): add stage2 framing and tcp helpers` (one or two commits OK)

### Task 2: Engine server + client gateway

- [ ] `ytta_engine --port N` serves one client session (or sequential sessions)
- [ ] Client path uses TCP gateway instead of in-process engine
- [ ] Commit: `feat(cpp): split engine and client over localhost TCP`

### Task 3: Two-process golden harness + docs

- [ ] `run_v0_tcp_golden.sh`: start engine, run client on v0 fixture, cmp golden, cleanup
- [ ] Wire into `make cpp-test` (or `cpp-test-tcp` + include from `cpp-test` if stable)
- [ ] Confirm in-process sync/queued + burst still pass
- [ ] README: how to run manually
- [ ] Commit: `test(cpp): assert v0 golden over TCP engine/client`

---

## Done when

- [ ] Acceptance checklist in Stage 2 intent doc is green for C++
- [ ] Go/Rust/Java untouched
- [ ] v0 golden + Stage 1 in-process tests still pass

## Out of scope

UDP, TLS, language ports, changing strategy/golden, multi-client matching, live feeds.
