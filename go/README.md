# Go Implementation

## Thread demo
```bash
go run ./cmd/ytta
```

## v0 tick-to-trade slice
From the `go/` directory (fixture paths relative to repo root):

```bash
go run ./cmd/ytta_v0 \
  --fixture ../shared/fixtures/v0/ticks.ndjson \
  --out /tmp/ytta_go_out.ndjson
```

Latency summary (`p50_ns` / `p99_ns`) prints to stderr.

From repo root:
```bash
make go-run-v0
make go-test
```

## Test
```bash
go test ./...
```

### v0 acceptance (Go)
- [x] Builds and replays shared fixture
- [x] Emits canonical stream matching `shared/fixtures/v0/golden.ndjson`
- [x] Strategy emits `NEW_ORDER` and `CANCEL`
- [x] MatchingEngine maintains one book
- [x] Reports e2e latency p50/p99 on stderr
- [x] Golden smoke test in `go test ./...`
