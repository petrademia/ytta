# C++ Implementation

## Build
```bash
cmake -S . -B build
cmake --build build
```

## Demos
```bash
./build/ytta_thread_demo
./build/ytta_mem_pool_demo
```

## v0 tick-to-trade slice
Replay the shared fixture and write canonical events:

```bash
./build/ytta_v0 \
  --fixture ../../shared/fixtures/v0/ticks.ndjson \
  --out /tmp/ytta_v0_out.ndjson
```

Latency summary (`p50_ns` / `p99_ns`) prints to stderr. Absolute µs budgets are not a pass/fail gate.

### Tests
```bash
./build/ytta_v0_tests
./tests/run_v0_golden.sh
```

### v0 acceptance (C++)
- [x] Builds and replays shared fixture
- [x] Emits canonical `action` / `ack` / `fill` / `md` stream
- [x] Strategy emits `NEW_ORDER` and `CANCEL` (not NOOP-only)
- [x] MatchingEngine maintains one book
- [x] Reports e2e latency p50/p99 on stderr
- [x] Smoke test compares output to `shared/fixtures/v0/golden.ndjson`
