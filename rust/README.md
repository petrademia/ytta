# Rust Implementation

## Thread demo
```bash
cargo run --manifest-path rust/Cargo.toml
# or from rust/
cargo run
```

## v0 tick-to-trade slice
From repo root:

```bash
cargo run --manifest-path rust/Cargo.toml --bin ytta_v0 -- \
  --fixture shared/fixtures/v0/ticks.ndjson \
  --out /tmp/ytta_rust_out.ndjson
```

Latency summary (`p50_ns` / `p99_ns`) prints to stderr.

```bash
make rust-run-v0
make rust-test
```

## Test
```bash
cargo test --manifest-path rust/Cargo.toml
```

### v0 acceptance (Rust)
- [x] Builds and replays shared fixture
- [x] Emits canonical stream matching `shared/fixtures/v0/golden.ndjson`
- [x] Strategy emits `NEW_ORDER` and `CANCEL`
- [x] MatchingEngine maintains one book
- [x] Reports e2e latency p50/p99 on stderr
- [x] Golden smoke test via `cargo test`
