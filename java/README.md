# Java Implementation

## Thread demo
```bash
mvn -q -f java/pom.xml package
java -cp java/target/ytta-java-0.1.0.jar com.ytta.App
# or
make java-run
```

## v0 tick-to-trade slice
From repo root:

```bash
mvn -q -f java/pom.xml package
java -cp java/target/ytta-java-0.1.0.jar com.ytta.v0.YttaV0 \
  --fixture shared/fixtures/v0/ticks.ndjson \
  --out /tmp/ytta_java_out.ndjson
```

Latency summary (`p50_ns` / `p99_ns`) prints to stderr.

```bash
make java-run-v0
make java-test
```

## Test
```bash
mvn -q -f java/pom.xml test
```

Golden/fixture paths are resolved from the repo root (works when Surefire runs with cwd `java/`).

### v0 acceptance (Java)
- [x] Builds and replays shared fixture
- [x] Emits canonical stream matching `shared/fixtures/v0/golden.ndjson`
- [x] Strategy emits `NEW_ORDER` and `CANCEL`
- [x] MatchingEngine maintains one book
- [x] Reports e2e latency p50/p99 on stderr
- [x] Golden smoke test via JUnit
