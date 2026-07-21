# Rust Implementation

## Thread demo
```bash
cargo run --manifest-path rust/Cargo.toml
# or from rust/
cargo run
```

## v0 / Stage 1 tick-to-trade slice
From repo root:

```bash
# sync
cargo run --manifest-path rust/Cargo.toml --bin ytta_v0 -- \
  --mode=sync \
  --fixture shared/fixtures/v0/ticks.ndjson \
  --out /tmp/ytta_rust_out.ndjson

# queued (SPSC ingress; same golden)
cargo run --manifest-path rust/Cargo.toml --bin ytta_v0 -- \
  --mode=queued \
  --fixture shared/fixtures/v0/ticks.ndjson \
  --out /tmp/ytta_rust_out.ndjson
```

### Burst (Stage 1)
```bash
cargo run --manifest-path rust/Cargo.toml --bin ytta_v0 -- \
  --mode=queued \
  --fixture shared/fixtures/v1/ticks_burst.ndjson \
  --out /tmp/ytta_rust_burst.ndjson
```

Latency JSON on stderr includes e2e + ingest/decide/execute p50/p99 and `drops`. Absolute µs values are not a CI gate.

```bash
make rust-run-v0
make rust-test
```

## Test
```bash
cargo test --manifest-path rust/Cargo.toml
```

### Acceptance (Rust)
- [x] v0 golden (sync + queued)
- [x] SPSC + object pool unit tests
- [x] Stage latency probe fields on stderr
- [x] Burst smoke: drops==0, action count == N
