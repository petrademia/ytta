# YTTA Stage 2 — Localhost TCP Protocol

Status: frozen for C++ Stage 2 lead. Aligns with `docs/superpowers/specs/2026-07-22-stage2-sockets.md`.

## Roles

| Process | Owns | Does not |
|---------|------|----------|
| **Engine** (`ytta_engine`) | MatchingEngine, book, TCP listen | ticks, strategy |
| **Client** (`ytta_client`) | TickSource, Strategy, LatencyProbe, TCP connect | book matching |

One instrument (`INSTR1`), paper only, localhost only.

## Topology

```
[TickSource] → [Strategy] → [TcpOrderGateway] ----TCP----> [EngineServer / MatchingEngine]
                     ↑                                         │
                     └──────── ack / fill / md <───────────────┘
```

In-process `sync` / `queued` modes remain available without TCP.

## Framing

Each message on the wire:

1. `uint32` **big-endian** payload length (bytes)
2. UTF-8 JSON object (exactly `length` bytes; no trailing newline required)

Max payload size: 1 MiB (reject larger).

## Client → engine

### Order

```json
{"type":"order","ts_ns":1001,"cl_ord_id":1,"op":"NEW","side":"BUY","price":100,"qty":5}
{"type":"order","ts_ns":1002,"cl_ord_id":1,"op":"CANCEL"}
```

| Field | Notes |
|-------|-------|
| `op` | `NEW` \| `CANCEL` |
| `side`/`price`/`qty` | required for `NEW`; omit for `CANCEL` |
| Semantics | Same as v0 engine (`specs/v0_messages.md`) |

### Goodbye (optional)

```json
{"type":"goodbye"}
```

Client may close the socket instead. Engine treats disconnect or `goodbye` as end of session and may accept a new client or exit (C++ reference: **stay up** until process kill; test harness kills the engine).

## Engine → client

After each order, engine sends zero or more event frames, then a **done** frame.

### Events (same fields as golden)

```json
{"type":"ack","ts_ns":1001,"cl_ord_id":1,"status":"ACCEPTED"}
{"type":"ack","ts_ns":1002,"cl_ord_id":99,"status":"REJECTED","reason":"unknown_id"}
{"type":"fill","ts_ns":1001,"cl_ord_id":1,"price":100,"qty":5,"liquidity":"MAKER"}
{"type":"md","ts_ns":1001,"instrument":"INSTR1","best_bid":99,"best_ask":101,"best_bid_qty":10,"best_ask_qty":8}
```

### Done

```json
{"type":"done"}
```

Client reads frames until `done`, then continues the next strategy decision.

## Session lifecycle

1. Engine binds `127.0.0.1:PORT` and listens
2. Client connects
3. Client replays fixture: for each tick, strategy runs locally; if not `NOOP`, send `order` and collect events until `done`
4. Client writes golden NDJSON (local `action` + remote `ack`/`fill`/`md`, same sort as v0)
5. Client disconnects (or sends `goodbye`)
6. Engine may accept another client; CI kills engine after one run

## Golden parity

Client must produce the same sorted NDJSON as in-process v0 for `shared/fixtures/v0/ticks.ndjson` → `shared/fixtures/v0/golden.ndjson`.

Strategy stays on the client; engine never sees ticks.

## Out of scope

UDP, TLS, multi-client matching, live feeds, language ports.
