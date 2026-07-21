# Go Implementation

## Thread demo
```bash
go run ./cmd/ytta
```

## v0 / Stage 1 tick-to-trade slice
From the `go/` directory:

```bash
# sync
go run ./cmd/ytta_v0 --mode=sync \
  --fixture ../shared/fixtures/v0/ticks.ndjson \
  --out /tmp/ytta_go_out.ndjson

# queued (SPSC ingress; same golden)
go run ./cmd/ytta_v0 --mode=queued \
  --fixture ../shared/fixtures/v0/ticks.ndjson \
  --out /tmp/ytta_go_out.ndjson
```

### Burst (Stage 1)
```bash
go run ./cmd/ytta_v0 --mode=queued \
  --fixture ../shared/fixtures/v1/ticks_burst.ndjson \
  --out /tmp/ytta_go_burst.ndjson
```

Latency JSON on stderr includes e2e + ingest/decide/execute p50/p99 and `drops`. Absolute µs values are not a CI gate.

From repo root:
```bash
make go-run-v0
make go-test
```

## Test
```bash
go test ./...
```

### Acceptance (Go)
- [x] v0 golden (sync + queued)
- [x] SPSC + object pool unit tests
- [x] Stage latency probe fields on stderr
- [x] Burst smoke: drops==0, action count == N
