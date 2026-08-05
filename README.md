# YTTA - Yield-Tuned Tick-to-Trade Architecture

DIY lab: implement a paper tick-to-trade system in C++, guided by
*Building Low Latency Applications with C++* (Packt).

**Start here:** [`docs/BOOK_PATH.md`](docs/BOOK_PATH.md) · progress: [`docs/PROGRESS.md`](docs/PROGRESS.md)

## Quick start

```bash
make cpp
make test-m0          # should pass after reset
make test-m1          # should FAIL until you implement M1
```

## Layout

- `cpp/` - your C++ implementation + skeleton tests
- `shared/` - schemas + fixtures (M2+)
- `specs/messages.md` - NDJSON / matching contract
- `go/`, `rust/`, `java/` - optional ports later

Prior finished implementations and Stage-era docs live in git history, not this tree.
