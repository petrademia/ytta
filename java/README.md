# Java Implementation

## Thread demo
```bash
mvn -q -f java/pom.xml package
java -cp java/target/ytta-java-0.1.0.jar com.ytta.App
# or
make java-run
```

## v0 / Stage 1 tick-to-trade slice
From repo root:

```bash
mvn -q -f java/pom.xml package

# sync
java -cp java/target/ytta-java-0.1.0.jar com.ytta.v0.YttaV0 \
  --mode=sync \
  --fixture shared/fixtures/v0/ticks.ndjson \
  --out /tmp/ytta_java_out.ndjson

# queued (SPSC ingress; same golden)
java -cp java/target/ytta-java-0.1.0.jar com.ytta.v0.YttaV0 \
  --mode=queued \
  --fixture shared/fixtures/v0/ticks.ndjson \
  --out /tmp/ytta_java_out.ndjson
```

### Burst (Stage 1)
```bash
java -cp java/target/ytta-java-0.1.0.jar com.ytta.v0.YttaV0 \
  --mode=queued \
  --fixture shared/fixtures/v1/ticks_burst.ndjson \
  --out /tmp/ytta_java_burst.ndjson
```

Latency JSON on stderr includes e2e + ingest/decide/execute p50/p99 and `drops`. Absolute µs values are not a CI gate.

```bash
make java-run-v0
make java-test
```

## Test
```bash
mvn -q -f java/pom.xml test
```

Golden/fixture paths are resolved from the repo root (works when Surefire runs with cwd `java/`).

### Acceptance (Java)
- [x] v0 golden (sync + queued)
- [x] SPSC + object pool unit tests
- [x] Stage latency probe fields on stderr
- [x] Burst smoke: drops==0, action count == N
- [x] `com.ytta.App` thread demo still runs
