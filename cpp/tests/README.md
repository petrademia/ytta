# C++ Tests

```bash
# From repo root:
make cpp-test

# Or directly:
./cpp/build/ytta_v0_tests
./cpp/tests/run_v0_golden.sh
```

`run_v0_golden.sh` rebuilds `ytta_v0` if needed, replays `shared/fixtures/v0/ticks.ndjson`, and compares to `shared/fixtures/v0/golden.ndjson`.
