# YTTA v0 Messages and Behavior

Status: **frozen** for v0 Phase 1 (C++ reference). Ports must follow this doc, not reverse-engineer C++.

## Streams

All fixtures and goldens are **NDJSON**: one JSON object per line, UTF-8, no pretty-print.

Instrument for v0: `INSTR1` only.

## Field dictionary

### Tick (fixture input)

| Field | Type | Notes |
|-------|------|-------|
| `type` | string | always `"tick"` |
| `ts_ns` | integer | fixture logical time (nanoseconds); not wall clock |
| `instrument` | string | `"INSTR1"` |
| `side` | string | `"BUY"` \| `"SELL"` |
| `price` | integer | positive |
| `qty` | integer | positive |
| `seq` | integer | strictly increasing in the fixture |

### Action (strategy decision; golden)

| Field | Type | Notes |
|-------|------|-------|
| `type` | string | `"action"` |
| `ts_ns` | integer | copy of triggering tick `ts_ns` |
| `action` | string | `"NOOP"` \| `"NEW_ORDER"` \| `"CANCEL"` |
| `cl_ord_id` | integer | required for `NEW_ORDER` and `CANCEL`; omit for `NOOP` |
| `side` | string | `"BUY"` \| `"SELL"`; required for `NEW_ORDER`; omit otherwise |
| `price` | integer | required for `NEW_ORDER`; omit otherwise |
| `qty` | integer | required for `NEW_ORDER`; omit otherwise |
| `reason` | string | short stable token |

### Ack (engine; golden)

| Field | Type | Notes |
|-------|------|-------|
| `type` | string | `"ack"` |
| `ts_ns` | integer | same as the order's `ts_ns` |
| `cl_ord_id` | integer | |
| `status` | string | `"ACCEPTED"` \| `"REJECTED"` |
| `reason` | string | present on reject; omit on accept |

### Fill (engine; golden)

| Field | Type | Notes |
|-------|------|-------|
| `type` | string | `"fill"` |
| `ts_ns` | integer | |
| `cl_ord_id` | integer | order that was filled |
| `price` | integer | match price (resting / maker price) |
| `qty` | integer | filled quantity |
| `liquidity` | string | `"MAKER"` \| `"TAKER"` |

When a cross matches resting liquidity, emit **one fill per order** involved (maker and taker), same `price`/`qty`/`ts_ns`.

### Market data (engine; golden)

| Field | Type | Notes |
|-------|------|-------|
| `type` | string | `"md"` |
| `ts_ns` | integer | |
| `instrument` | string | `"INSTR1"` |
| `best_bid` | integer | `0` if no bid |
| `best_ask` | integer | `0` if no ask |
| `best_bid_qty` | integer | `0` if no bid |
| `best_ask_qty` | integer | `0` if no ask |

Always emit all four book fields (use `0` for empty side).

### Order (internal only)

`order` messages may be used inside an implementation. They are **excluded** from the golden stream.

### Latency (stderr trailer only)

```json
{"type":"latency","count":N,"p50_ns":...,"p99_ns":...}
```

Not part of golden compare. Measured with `steady_clock` wall deltas per tick in the C++ reference; absolute values are not a pass/fail gate.

## Golden stream rules

1. Golden contains only: `action`, `ack`, `fill`, `md`.
2. Canonical sort key: `(ts_ns ASC, type_rank ASC, cl_ord_id ASC)` where `type_rank` is:
   - `md` = 0
   - `action` = 1
   - `ack` = 2
   - `fill` = 3
3. For `md` and `NOOP` actions without `cl_ord_id`, treat `cl_ord_id` as `0` for sorting.
4. Same fixture → identical golden bytes across language ports (after this sort).

## Matching engine (v0)

- One instrument, one book.
- Ops: `NEW`, `CANCEL` only.
- Price-time FIFO at each price level.
- `NEW` that crosses the opposite book matches immediately (no self-trade prevention).
- Match price = resting order price.
- Partial fills allowed; residual resting qty stays on book.
- `CANCEL` of unknown `cl_ord_id` → `REJECTED` / `unknown_id`.
- After every accepted `NEW` or `CANCEL` (and after matching), emit one `md` snapshot.
- Rejected ops emit `ack` only (no `md`).

## Deterministic strategy (v0)

State:

- `next_cl_ord_id` starts at `1`
- `resting_cl_ord_id` starts unset

On each tick, apply **exactly one** rule (first match wins). All actions use the tick's `ts_ns`.

1. If `seq == 1`: `NEW_ORDER` BUY `price=100` `qty=5` `cl_ord_id=next++` reason `resting_bid`
2. Else if `seq == 2`: `NEW_ORDER` SELL `price=100` `qty=5` `cl_ord_id=next++` reason `cross_self`
3. Else if `seq == 3`: `NEW_ORDER` BUY `price=99` `qty=1` `cl_ord_id=next++` reason `resting_bid`
4. Else if `seq == 4`: `CANCEL` the `cl_ord_id` created at `seq == 3` reason `done`
5. Else: `NOOP` reason `flat`

Track resting ids only as needed for rule 4 (`cl_ord_id` from rule 3). Do not key off market data for v0 decisions (MD is still emitted and must appear in the golden).

## Hand-computed expected behavior (`ticks.ndjson`)

| seq | Strategy | Engine notes |
|-----|----------|--------------|
| 1 | NEW BUY 100/5 id=1 | ACCEPTED; book bid 100x5; md |
| 2 | NEW SELL 100/5 id=2 | ACCEPTED; matches id=1 fully; fills id=1 MAKER + id=2 TAKER; book empty; md |
| 3 | NEW BUY 99/1 id=3 | ACCEPTED; book bid 99x1; md |
| 4 | CANCEL id=3 | ACCEPTED; book empty; md |
| 5–8 | NOOP | no engine ops |

## Fixture path

- Input: `shared/fixtures/v0/ticks.ndjson`
- Golden: `shared/fixtures/v0/golden.ndjson` (locked after C++ reference run)
- Schemas: `shared/schemas/v0/*.schema.json`
