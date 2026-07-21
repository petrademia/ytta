# C++ Implementation

## Build
```bash
cmake -S . -B build
cmake --build build
# or from repo root:
make cpp
```

## Demos
```bash
./build/ytta_thread_demo
./build/ytta_mem_pool_demo
```

## v0 / Stage 1 tick-to-trade slice

Replay a fixture and write canonical events:

```bash
# sync (direct call path)
./build/ytta_v0 --mode=sync \
  --fixture ../../shared/fixtures/v0/ticks.ndjson \
  --out /tmp/ytta_v0_out.ndjson

# queued (SPSC ingress; single-consumer pump; same golden)
./build/ytta_v0 --mode=queued \
  --fixture ../../shared/fixtures/v0/ticks.ndjson \
  --out /tmp/ytta_v0_out.ndjson
```

Both modes must byte-match `shared/fixtures/v0/golden.ndjson`.

### Burst (Stage 1)
```bash
./build/ytta_v0 --mode=queued \
  --fixture ../../shared/fixtures/v1/ticks_burst.ndjson \
  --out /tmp/ytta_burst_out.ndjson
```

Latency JSON on stderr includes e2e + ingest/decide/execute p50/p99 and `drops`. Absolute µs values are not a CI gate; `drops` must be 0 for the burst smoke (capacity 16384 ≥ fixture size).

## Stage 2 — engine + client over localhost TCP

Strategy and ticks stay on the client; the engine only matches orders.

```bash
# terminal 1 — engine (stays up until killed; --port 0 = ephemeral)
./build/ytta_engine --port 9000

# terminal 2 — client
./build/ytta_client --host 127.0.0.1 --port 9000 \
  --fixture ../../shared/fixtures/v0/ticks.ndjson \
  --out /tmp/ytta_tcp_out.ndjson
```

Client output must byte-match `shared/fixtures/v0/golden.ndjson`. Protocol: `specs/stage2_sockets.md`.

### Tests
```bash
./build/ytta_v0_tests
./build/ytta_v0_spsc_tests
./build/ytta_v0_net_tests
./tests/run_v0_golden.sh          # sync + queued vs golden
./tests/run_stage1_burst.sh       # queued burst, drops==0
./tests/run_v0_tcp_golden.sh      # spawn engine + client vs golden
# from repo root:
make cpp-test
```

### Acceptance
- [x] v0 golden (sync + queued)
- [x] SPSC + object pool unit tests
- [x] Stage latency probe fields on stderr
- [x] Burst smoke: zero drops, action count == tick count
- [x] Stage 2 TCP golden (engine + client)
