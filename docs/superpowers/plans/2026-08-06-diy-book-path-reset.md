# DIY Book Path Reset Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Convert YTTA into an empty C++ DIY lab with Packt-aligned milestones M0–M6, curriculum docs, and skeleton tests that fail until the learner implements them.

**Architecture:** Wipe production implementations from the working tree (recoverable via git history). Keep shared fixtures/schemas as behavioral contracts. Replace the polyglot finished product with a CMake C++ scaffold, stub APIs, and per-milestone test binaries. Delete historical Stage 0/1/2 docs (no `docs/archive/`); active spine is `docs/BOOK_PATH.md` + `docs/PROGRESS.md`.

**Tech Stack:** C++20, CMake ≥ 3.16, Make, NDJSON fixtures under `shared/`, assert-style test mains (no Catch2/GTest dependency).

## Global Constraints

- Critical path is C++ only; Go/Rust/Java are “port later” stubs, not finish criteria.
- No special archive branch/tag or `reference/` tree; rely on git history.
- Milestone spine is M0–M6 (book parts), not Stage 0/1/2.
- Skeleton tests for M1–M6 must **fail** after the reset; M0 smoke must **pass**.
- Do not re-implement matching engine, strategies, or networking as part of this plan.
- Never use the em dash character in docs or commit messages; use plain hyphen.

## File Structure

| Path | Responsibility |
|------|----------------|
| `docs/BOOK_PATH.md` | Active curriculum: chapters → milestones → files → verify commands |
| `docs/PROGRESS.md` | M0–M6 checkboxes |
| `specs/messages.md` | Active message/behavior contract for M2+ (adapted from old v0) |
| `shared/schemas/v0/` | JSON schemas (kept) |
| `shared/fixtures/v0/` | ticks + golden (kept for M2/M4) |
| `shared/fixtures/v1/` | burst ticks (kept for optional M6 stress) |
| `cpp/CMakeLists.txt` | Build smoke + stub libs + milestone test binaries |
| `cpp/src/smoke/smoke.cpp` | M0 empty main |
| `cpp/src/m1/*.hpp` | Building-block APIs (stubs) |
| `cpp/src/m2/*.hpp` + `*.cpp` | Book/engine/gateway stubs |
| `cpp/src/m3/net/*` | Framing/TCP stubs |
| `cpp/src/m4/*` | Pipeline/client stubs |
| `cpp/src/m5/*` | Strategy stub |
| `cpp/src/m6/*` | LatencyProbe stub |
| `cpp/tests/mN_*.cpp` | Skeleton tests labeled by milestone |
| `cpp/tests/run_m4_golden.sh` | Golden compare harness (expects learner binary later) |
| `go/README.md`, `rust/README.md`, `java/README.md` | Port-later only |
| `Makefile` | `cpp`, `cpp-test`, `test-m0`…`test-m6` |
| `README.md` | DIY one-screen quick start |

---

### Task 1: Delete historical docs and lock design status

**Files:**
- Delete: `docs/architecture.md`
- Delete: `docs/handoff/` (entire tree)
- Delete: `docs/superpowers/plans/2026-07-22-*.md`
- Delete: `docs/superpowers/specs/2026-07-22-*.md`
- Delete: `specs/stage1_hotpath.md`, `specs/stage2_sockets.md`, `specs/tick_to_trade_spec.md`
- Keep temporarily: `specs/v0_messages.md` until Task 2 copies it to `specs/messages.md`, then delete `specs/v0_messages.md`
- Modify: `docs/superpowers/specs/2026-08-06-diy-book-path-design.md` (status → locked)
- Keep: DIY design + this reset plan under `docs/superpowers/`
- Do **not** create `docs/archive/`

**Interfaces:**
- Consumes: nothing
- Produces: cleaned docs tree; design status `locked`

- [ ] **Step 1: Delete Stage-era docs**

```bash
git rm -f docs/architecture.md
git rm -rf docs/handoff 2>/dev/null || rm -rf docs/handoff
git rm -f docs/superpowers/plans/2026-07-22-*.md
git rm -f docs/superpowers/specs/2026-07-22-*.md
git rm -f specs/stage1_hotpath.md specs/stage2_sockets.md specs/tick_to_trade_spec.md
# Leave specs/v0_messages.md for Task 2 copy; delete it there.
```

If some paths are untracked, use `rm -rf` then continue.

- [ ] **Step 2: Mark design locked**

In `docs/superpowers/specs/2026-08-06-diy-book-path-design.md`, set:

```markdown
Status: **locked**
```

- [ ] **Step 3: Commit**

```bash
git add -u docs specs
git add docs/superpowers/specs/2026-08-06-diy-book-path-design.md
git status
git commit -m "$(cat <<'EOF'
docs: delete stage-era material for DIY book path

Drop historical Stage 0/1/2 docs from the tree; recover from git history if needed.
EOF
)"
```

---

### Task 2: Active message contract + curriculum docs

**Files:**
- Create: `specs/messages.md` (copy content from `specs/v0_messages.md`, change title/status header)
- Delete: `specs/v0_messages.md` after the copy
- Create: `docs/BOOK_PATH.md`
- Create: `docs/PROGRESS.md`
- Modify: `README.md`

**Interfaces:**
- Consumes: `specs/v0_messages.md` behavior text
- Produces: active learner-facing docs

- [ ] **Step 1: Write `specs/messages.md`**

Copy `specs/v0_messages.md` to `specs/messages.md` and replace the top matter with:

```markdown
# YTTA Messages and Behavior (DIY)

Status: **active** for milestones M2+ (paper/replay).

Canonical NDJSON contracts for ticks, actions, acks, fills, and market data.
Schemas live under `shared/schemas/v0/`. Fixtures under `shared/fixtures/v0/`.
```

Leave the field dictionary and matching rules intact. Then delete `specs/v0_messages.md`.

- [ ] **Step 2: Write `docs/BOOK_PATH.md`**

```markdown
# YTTA Book Path

Primary text: *Building Low Latency Applications with C++* (Sourav Ghosh, Packt).

Workflow: read chapters → implement under `cpp/src/` → run the milestone tests → tick `docs/PROGRESS.md`.

| ID | Book | Implement | Verify |
|----|------|-----------|--------|
| M0 | Ch 1–3 | Notes + scaffold | `make test-m0` |
| M1 | Ch 4 | SPSC, pool, clock, thread utils | `make test-m1` |
| M2 | Ch 5–6 | Book + matching engine + in-process gateway | `make test-m2` |
| M3 | Ch 7 | Localhost TCP framing + gateway/engine I/O | `make test-m3` |
| M4 | Ch 8–9 | Client pipeline + golden fixture replay | `make test-m4` |
| M5 | Ch 10 | One MM **or** one liquidity-taking strategy | `make test-m5` |
| M6 | Ch 11–12 | LatencyProbe histograms + report p50/p99 | `make test-m6` |

## M0 — Concepts (Ch 1–3)

- Goals: understand latency-sensitive apps; set up the repo.
- Files: `cpp/src/smoke/smoke.cpp`
- Done when: `make test-m0` passes.

## M1 — Building blocks (Ch 4)

- Goals: bounded SPSC queue, object pool, monotonic clock helper, basic thread utility.
- Files: `cpp/src/m1/spsc_queue.hpp`, `object_pool.hpp`, `mono_clock.hpp`, `thread_utils.hpp`
- Done when: `make test-m1` passes.

## M2 — Matching engine (Ch 5–6)

- Goals: one-instrument paper book; NEW/CANCEL; maker/taker fills.
- Contract: `specs/messages.md`
- Files: `cpp/src/m2/types.hpp`, `book.hpp`, `matching_engine.hpp`, `order_gateway.hpp` (+ `.cpp` as needed)
- Done when: `make test-m2` passes.

## M3 — Participant I/O (Ch 7)

- Goals: length-prefixed framing; localhost TCP engine/client helpers.
- Files: `cpp/src/m3/net/framing.hpp`, `tcp.hpp` (+ `.cpp`)
- Done when: `make test-m3` passes.

## M4 — Client path (Ch 8–9)

- Goals: replay `shared/fixtures/v0/ticks.ndjson`; emit canonical golden stream.
- Files: `cpp/src/m4/pipeline.hpp`, `tick_source.hpp`, `strategy.hpp` (+ mains later)
- Done when: `make test-m4` passes (unit/pipeline + golden script).

## M5 — Strategy (Ch 10)

- Goals: implement **either** simple market-making **or** liquidity-taking (pick one).
- Files: `cpp/src/m5/strategy.hpp`
- Done when: `make test-m5` passes.

## M6 — Measure (Ch 11–12)

- Goals: record e2e samples; print p50/p99; optional burst fixture.
- Files: `cpp/src/m6/latency_probe.hpp`
- Done when: `make test-m6` passes (reporting required; no hard µs SLO).

## Ports (optional, later)

`go/`, `rust/`, `java/` are placeholders. Do not block C++ milestones on them.
```

- [ ] **Step 3: Write `docs/PROGRESS.md`**

```markdown
# YTTA Progress (DIY)

Active path: book milestones M0–M6. Design: `docs/superpowers/specs/2026-08-06-diy-book-path-design.md`.

| Milestone | Book | C++ | Notes |
|-----------|------|-----|-------|
| M0 | Ch 1–3 | [ ] | scaffold / smoke |
| M1 | Ch 4 | [ ] | building blocks |
| M2 | Ch 5–6 | [ ] | book + engine |
| M3 | Ch 7 | [ ] | TCP I/O |
| M4 | Ch 8–9 | [ ] | client golden |
| M5 | Ch 10 | [ ] | MM **or** taker |
| M6 | Ch 11–12 | [ ] | probe / p50/p99 |

Tick a box only when that milestone's `make test-mN` is green.
```

- [ ] **Step 4: Rewrite `README.md`**

```markdown
# YTTA — Yield-Tuned Tick-to-Trade Architecture

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

- `cpp/` — your C++ implementation + skeleton tests
- `shared/` — schemas + fixtures (M2+)
- `specs/messages.md` — NDJSON / matching contract
- `go/`, `rust/`, `java/` — optional ports later

Prior finished implementations and Stage-era docs live in git history, not this tree.
```

- [ ] **Step 5: Commit**

```bash
git add specs/messages.md docs/BOOK_PATH.md docs/PROGRESS.md README.md
git add -u specs/v0_messages.md
git commit -m "$(cat <<'EOF'
docs: add DIY book path curriculum and message contract

Give learners an M0–M6 checklist and keep NDJSON behavior under specs/messages.md.
EOF
)"
```

---

### Task 3: Strip language trees and slim Makefile

**Files:**
- Delete: all production sources under `cpp/src/`, old `cpp/tests/*`, `go/` internals, `rust/src`, `java/src` (entire trees except replacement READMEs)
- Create: `go/README.md`, `rust/README.md`, `java/README.md` (port-later)
- Modify: root `Makefile`, root `CMakeLists.txt` if present
- Keep: `shared/` intact

**Interfaces:**
- Consumes: nothing
- Produces: empty language placeholders; Makefile ready for Task 4+

- [ ] **Step 1: Remove C++ / Go / Rust / Java implementations**

```bash
# From repo root. Do not delete shared/.
rm -rf cpp/src cpp/tests cpp/build cpp/benchmarks
rm -rf go/cmd go/internal go/tests go/benchmarks
rm -rf rust/src rust/tests rust/benches rust/target
rm -rf java/src java/benchmarks java/target
# Keep go.mod / Cargo.toml / pom.xml only if rewritten in later steps; simpler: remove and leave README-only dirs
rm -f go/go.mod rust/Cargo.toml rust/Cargo.lock java/pom.xml
```

- [ ] **Step 2: Write port-later READMEs**

`go/README.md`, `rust/README.md`, `java/README.md` each:

```markdown
# <Lang> port (optional)

Not on the DIY critical path. Implement C++ milestones M0–M6 first
(`docs/BOOK_PATH.md`). Reintroduce this language later against
`shared/fixtures` + `specs/messages.md` if desired.
```

- [ ] **Step 3: Replace root `Makefile`**

```makefile
.PHONY: all help cpp cpp-test test-m0 test-m1 test-m2 test-m3 test-m4 test-m5 test-m6 test

all: cpp

help:
	@echo "Targets: cpp cpp-test test-m0 test-m1 test-m2 test-m3 test-m4 test-m5 test-m6 test"

cpp:
	cmake -S cpp -B cpp/build
	cmake --build cpp/build

test-m0: cpp
	./cpp/build/ytta_m0_smoke

test-m1: cpp
	./cpp/build/ytta_m1_tests

test-m2: cpp
	./cpp/build/ytta_m2_tests

test-m3: cpp
	./cpp/build/ytta_m3_tests

test-m4: cpp
	./cpp/build/ytta_m4_tests
	./cpp/tests/run_m4_golden.sh

test-m5: cpp
	./cpp/build/ytta_m5_tests

test-m6: cpp
	./cpp/build/ytta_m6_tests

cpp-test: test-m0
	@echo "Note: test-m1..m6 are expected to FAIL until you implement them."
	-./cpp/build/ytta_m1_tests; \
	-./cpp/build/ytta_m2_tests; \
	-./cpp/build/ytta_m3_tests; \
	-./cpp/build/ytta_m4_tests; \
	-./cpp/build/ytta_m5_tests; \
	-./cpp/build/ytta_m6_tests; \
	true

test: test-m0
```

- [ ] **Step 4: Commit**

```bash
git add -A
git commit -m "$(cat <<'EOF'
chore: strip finished implementations for DIY reset

Leave shared fixtures and language README stubs; recover old code from git history if needed.
EOF
)"
```

---

### Task 4: M0 smoke scaffold (must pass)

**Files:**
- Create: `cpp/CMakeLists.txt`
- Create: `cpp/src/smoke/smoke.cpp`
- Create: `cpp/README.md`
- Create: `cpp/tests/README.md`

**Interfaces:**
- Consumes: nothing
- Produces: executable `ytta_m0_smoke` exiting 0

- [ ] **Step 1: Write smoke main**

`cpp/src/smoke/smoke.cpp`:

```cpp
#include <iostream>

int main() {
  std::cout << "ytta_m0_smoke ok\n";
  return 0;
}
```

- [ ] **Step 2: Write initial `cpp/CMakeLists.txt`**

```cmake
cmake_minimum_required(VERSION 3.16)
project(ytta_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(ytta_m0_smoke src/smoke/smoke.cpp)
```

(Later tasks append targets to this file.)

- [ ] **Step 3: Write `cpp/README.md`**

```markdown
# YTTA C++ DIY

Follow `docs/BOOK_PATH.md`. Build with `make cpp` from repo root.

Milestone tests: `make test-m0` … `make test-m6`.
```

- [ ] **Step 4: Run M0**

```bash
make test-m0
```

Expected: prints `ytta_m0_smoke ok` and exit 0.

- [ ] **Step 5: Commit**

```bash
git add cpp/CMakeLists.txt cpp/src/smoke/smoke.cpp cpp/README.md
git commit -m "$(cat <<'EOF'
feat(cpp): add M0 smoke scaffold

Establish a passing baseline build for the DIY lab.
EOF
)"
```

---

### Task 5: M1 stubs + failing skeleton tests

**Files:**
- Create: `cpp/src/m1/spsc_queue.hpp`, `object_pool.hpp`, `mono_clock.hpp`, `thread_utils.hpp`
- Create: `cpp/tests/m1_blocks_test.cpp`
- Modify: `cpp/CMakeLists.txt`

**Interfaces:**
- Consumes: nothing
- Produces:
  - `ytta::m1::SpscQueue<T, N>::try_push(T)` / `try_pop(T&)` → bool
  - `ytta::m1::ObjectPool<T>::acquire(T)` → `T*`; `release(T*)`; `free_count()`; `living()`
  - `ytta::m1::MonoClock::now_ns()` → `std::uint64_t` (must be non-decreasing across calls)
  - `ytta::m1::pin_current_thread_to_core(int)` → bool (stub may return false)

- [ ] **Step 1: Write stub headers (intentionally incomplete behavior)**

`cpp/src/m1/spsc_queue.hpp`:

```cpp
#pragma once
#include <cstddef>
#include <optional>
#include <array>

namespace ytta::m1 {

template <typename T, std::size_t N>
class SpscQueue {
 public:
  bool try_push(const T&) { return false; }  // stub: always full
  bool try_pop(T&) { return false; }         // stub: always empty
};

}  // namespace ytta::m1
```

`cpp/src/m1/object_pool.hpp`:

```cpp
#pragma once
#include <cstddef>

namespace ytta::m1 {

template <typename T>
class ObjectPool {
 public:
  T* acquire(const T&) { return nullptr; }  // stub
  void release(T*) {}
  std::size_t free_count() const { return 0; }
  std::size_t living() const { return 0; }
};

}  // namespace ytta::m1
```

`cpp/src/m1/mono_clock.hpp`:

```cpp
#pragma once
#include <cstdint>

namespace ytta::m1 {

struct MonoClock {
  // Stub: returns 0 always (breaks monotonicity / usefulness checks).
  static std::uint64_t now_ns() { return 0; }
};

}  // namespace ytta::m1
```

`cpp/src/m1/thread_utils.hpp`:

```cpp
#pragma once

namespace ytta::m1 {

inline bool pin_current_thread_to_core(int /*core*/) { return false; }

}  // namespace ytta::m1
```

- [ ] **Step 2: Write failing test `cpp/tests/m1_blocks_test.cpp`**

```cpp
#include "spsc_queue.hpp"
#include "object_pool.hpp"
#include "mono_clock.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

namespace {
int failures = 0;
void expect(bool cond, const char* msg) {
  if (!cond) {
    std::cerr << "FAIL: " << msg << "\n";
    ++failures;
  }
}
}  // namespace

int main() {
  using ytta::m1::ObjectPool;
  using ytta::m1::SpscQueue;
  using ytta::m1::MonoClock;

  {
    SpscQueue<int, 4> q;
    expect(q.try_push(1), "push 1");
    expect(q.try_push(2), "push 2");
    expect(q.try_push(3), "push 3");
    expect(q.try_push(4), "push 4");
    expect(!q.try_push(5), "full rejects");
    int v = 0;
    expect(q.try_pop(v) && v == 1, "fifo 1");
    expect(q.try_pop(v) && v == 2, "fifo 2");
    expect(q.try_pop(v) && v == 3, "fifo 3");
    expect(q.try_pop(v) && v == 4, "fifo 4");
    expect(!q.try_pop(v), "empty rejects");
  }

  {
    SpscQueue<std::string, 2> q;
    expect(q.try_push("a"), "push a");
    expect(q.try_push("b"), "push b");
    expect(!q.try_push("c"), "cap 2 full");
    std::string s;
    expect(q.try_pop(s) && s == "a", "pop a");
    expect(q.try_push("c"), "reuse slot");
    expect(q.try_pop(s) && s == "b", "pop b");
    expect(q.try_pop(s) && s == "c", "pop c");
  }

  {
    ObjectPool<int> pool;
    int* a = pool.acquire(7);
    int* b = pool.acquire(8);
    expect(a && b && a != b && *a == 7 && *b == 8, "acquire distinct");
    pool.release(a);
    int* c = pool.acquire(9);
    expect(c == a && *c == 9, "reuse freed slot");
    pool.release(b);
    pool.release(c);
    expect(pool.free_count() == 2, "two free");
    for (int i = 0; i < 1000; ++i) {
      int* p = pool.acquire(i);
      expect(p != nullptr, "loop acquire");
      pool.release(p);
    }
    expect(pool.living() <= 2, "no unbounded growth on reuse loop");
  }

  {
    auto t0 = MonoClock::now_ns();
    auto t1 = MonoClock::now_ns();
    expect(t1 >= t0, "monotonic non-decreasing");
    expect(t0 != 0 || t1 != 0, "clock not stuck at zero forever");
    // Allow equal values; require at least one non-zero after a tiny sleep.
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    auto t2 = MonoClock::now_ns();
    expect(t2 >= t1 && t2 > 0, "clock advances past zero");
  }

  if (failures != 0) {
    std::cerr << failures << " failure(s)\n";
    return EXIT_FAILURE;
  }
  std::cout << "ytta_m1_tests ok\n";
  return EXIT_SUCCESS;
}
```

Add `#include <chrono>` at the top of the test file.

- [ ] **Step 3: Append CMake targets**

```cmake
add_executable(ytta_m1_tests tests/m1_blocks_test.cpp)
target_include_directories(ytta_m1_tests PRIVATE src/m1)
target_link_libraries(ytta_m1_tests PRIVATE Threads::Threads)
```

- [ ] **Step 4: Verify fail-for-right-reason**

```bash
make test-m1
```

Expected: build succeeds; process exits non-zero; stderr contains `FAIL: push 1` (and more).

- [ ] **Step 5: Commit**

```bash
git add cpp/src/m1 cpp/tests/m1_blocks_test.cpp cpp/CMakeLists.txt
git commit -m "$(cat <<'EOF'
test(cpp): add failing M1 building-block skeleton

Stubs compile; assertions define the Ch 4 finish line for the learner.
EOF
)"
```

---

### Task 6: M2 stubs + failing engine tests

**Files:**
- Create: `cpp/src/m2/types.hpp`, `book.hpp`, `matching_engine.hpp`, `matching_engine.cpp`, `order_gateway.hpp`
- Create: `cpp/tests/m2_engine_test.cpp`
- Modify: `cpp/CMakeLists.txt`

**Interfaces:**
- Consumes: nothing from M1 (in-process engine can stand alone)
- Produces (namespace `ytta::m2`):
  - `enum class Side { Buy, Sell };`
  - `enum class AckStatus { Accepted, Rejected };`
  - `enum class Liquidity { Maker, Taker };`
  - `struct Order { std::uint64_t cl_ord_id; Side side; std::int64_t price; std::int64_t qty; };`
  - `struct AckEvent { std::uint64_t ts_ns; std::uint64_t cl_ord_id; AckStatus status; std::string reason; };`
  - `struct FillEvent { std::uint64_t ts_ns; std::uint64_t cl_ord_id; std::int64_t price; std::int64_t qty; Liquidity liquidity; };`
  - `struct MdEvent { std::uint64_t ts_ns; std::int64_t best_bid; std::int64_t best_ask; std::int64_t best_bid_qty; std::int64_t best_ask_qty; };`
  - `using EngineEvent = std::variant<AckEvent, FillEvent, MdEvent>;`
  - `class MatchingEngine;` (opaque ops used via gateway)
  - `class OrderGateway { explicit OrderGateway(MatchingEngine&); std::vector<EngineEvent> new_order(const Order&, std::uint64_t ts_ns); std::vector<EngineEvent> cancel(std::uint64_t cl_ord_id, std::uint64_t ts_ns); };`

Behavior contract: `specs/messages.md` (resting NEW → ack+md; cross → ack+maker fill+taker fill+md; bad cancel → rejected ack with reason `unknown_id`).

- [ ] **Step 1: Write types + stub engine/gateway**

`cpp/src/m2/types.hpp` — define the enums/structs/`EngineEvent` above (full definitions, no stubbing needed).

`cpp/src/m2/matching_engine.hpp`:

```cpp
#pragma once
#include "types.hpp"
#include <vector>

namespace ytta::m2 {

class MatchingEngine {
 public:
  std::vector<EngineEvent> new_order(const Order& order, std::uint64_t ts_ns);
  std::vector<EngineEvent> cancel(std::uint64_t cl_ord_id, std::uint64_t ts_ns);
};

}  // namespace ytta::m2
```

`cpp/src/m2/matching_engine.cpp`:

```cpp
#include "matching_engine.hpp"

namespace ytta::m2 {

std::vector<EngineEvent> MatchingEngine::new_order(const Order&, std::uint64_t) {
  return {};  // stub: no events
}

std::vector<EngineEvent> MatchingEngine::cancel(std::uint64_t, std::uint64_t) {
  return {};  // stub
}

}  // namespace ytta::m2
```

`cpp/src/m2/order_gateway.hpp`:

```cpp
#pragma once
#include "matching_engine.hpp"

namespace ytta::m2 {

class OrderGateway {
 public:
  explicit OrderGateway(MatchingEngine& engine) : engine_(engine) {}

  std::vector<EngineEvent> new_order(const Order& order, std::uint64_t ts_ns) {
    return engine_.new_order(order, ts_ns);
  }

  std::vector<EngineEvent> cancel(std::uint64_t cl_ord_id, std::uint64_t ts_ns) {
    return engine_.cancel(cl_ord_id, ts_ns);
  }

 private:
  MatchingEngine& engine_;
};

}  // namespace ytta::m2
```

`cpp/src/m2/book.hpp` — empty placeholder comment header for the learner:

```cpp
#pragma once
// Implement price-time FIFO book here (see Packt Ch 6 + specs/messages.md).
namespace ytta::m2 {
struct Book {
  // learner fills in
};
}  // namespace ytta::m2
```

- [ ] **Step 2: Write `cpp/tests/m2_engine_test.cpp`**

Port the scenarios from the pre-reset `v0_book_test.cpp`, switching namespaces `ytta::v0` → `ytta::m2` and includes to m2 headers. Keep the four cases: resting NEW, cross, bad CANCEL, good CANCEL.

- [ ] **Step 3: CMake**

```cmake
add_library(ytta_m2_lib STATIC src/m2/matching_engine.cpp)
target_include_directories(ytta_m2_lib PUBLIC src/m2)

add_executable(ytta_m2_tests tests/m2_engine_test.cpp)
target_link_libraries(ytta_m2_tests PRIVATE ytta_m2_lib)
```

- [ ] **Step 4: Verify failure**

```bash
make test-m2
```

Expected: FAIL on `new resting: ack+md` (empty stub events).

- [ ] **Step 5: Commit**

```bash
git add cpp/src/m2 cpp/tests/m2_engine_test.cpp cpp/CMakeLists.txt
git commit -m "$(cat <<'EOF'
test(cpp): add failing M2 matching-engine skeleton

Define OrderGateway scenarios so Ch 5–6 work has a clear verify gate.
EOF
)"
```

---

### Task 7: M3 stubs + failing net tests

**Files:**
- Create: `cpp/src/m3/net/framing.hpp`, `framing.cpp`, `tcp.hpp`, `tcp.cpp`
- Create: `cpp/tests/m3_net_test.cpp`
- Modify: `cpp/CMakeLists.txt`

**Interfaces:**
- Consumes: nothing
- Produces (`ytta::m3`):
  - `std::vector<std::uint8_t> frame(const std::string& payload);`
  - `bool try_deframe(std::string& buffer, std::string& payload);`  
    Framing: 4-byte big-endian length prefix + UTF-8 payload bytes.
  - `class TcpSocket` with `bool listen_localhost(std::uint16_t port);`, `bool connect_localhost(std::uint16_t port);`, `bool send_all(const std::uint8_t* data, std::size_t n);`, `ssize_t recv_some(std::uint8_t* data, std::size_t n);`, `void close();`

- [ ] **Step 1: Stub framing/tcp**

`framing.cpp` stubs: `frame` returns empty vector; `try_deframe` returns false.

`tcp.cpp` stubs: all methods return false / -1 / no-op.

- [ ] **Step 2: Write `m3_net_test.cpp`**

Minimum cases:
1. `frame("hi")` size == 6 and length bytes encode 2; round-trip via `try_deframe`.
2. Spin listen on port `0` (ephemeral) or fixed `19090`, connect, `send_all` framed payload, peer `recv_some` + deframe equals payload.

Use Threads for client/server in-process.

- [ ] **Step 3: CMake + verify fail**

```cmake
add_library(ytta_m3_lib STATIC src/m3/net/framing.cpp src/m3/net/tcp.cpp)
target_include_directories(ytta_m3_lib PUBLIC src/m3)

add_executable(ytta_m3_tests tests/m3_net_test.cpp)
target_link_libraries(ytta_m3_tests PRIVATE ytta_m3_lib Threads::Threads)
```

```bash
make test-m3
```

Expected: non-zero exit; FAIL on framing round-trip.

- [ ] **Step 4: Commit**

```bash
git add cpp/src/m3 cpp/tests/m3_net_test.cpp cpp/CMakeLists.txt
git commit -m "$(cat <<'EOF'
test(cpp): add failing M3 TCP framing skeleton

Lock Ch 7 verify gates for length-prefixed localhost messaging.
EOF
)"
```

---

### Task 8: M4 pipeline + golden harness (failing)

**Files:**
- Create: `cpp/src/m4/tick_source.hpp`, `strategy.hpp`, `pipeline.hpp`, `pipeline.cpp`
- Create: `cpp/src/m4_main.cpp` (CLI stub)
- Create: `cpp/tests/m4_pipeline_test.cpp`
- Create: `cpp/tests/run_m4_golden.sh`
- Modify: `cpp/CMakeLists.txt`
- Keep: `shared/fixtures/v0/ticks.ndjson`, `shared/fixtures/v0/golden.ndjson`

**Interfaces:**
- Consumes: `ytta::m2::OrderGateway` / engine types (pipeline may include m2 lib)
- Produces:
  - `ytta::m4::Pipeline::run(const std::string& fixture_path, const std::string& out_path) -> int`  
    Stub returns `1` and writes nothing.
  - Binary `ytta_m4` CLI: `--fixture PATH --out PATH`

- [ ] **Step 1: Stub pipeline + main**

`pipeline.cpp`: `run` returns 1.

`m4_main.cpp`: parse `--fixture` / `--out`, call `Pipeline::run`, return its status.

- [ ] **Step 2: Unit test**

`m4_pipeline_test.cpp`: call `run` with `shared/fixtures/v0/ticks.ndjson` to a temp out path; expect return `0` and file exists (will fail on stub).

- [ ] **Step 3: Golden script**

`cpp/tests/run_m4_golden.sh`:

```bash
#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BIN="$ROOT/cpp/build/ytta_m4"
OUT="$(mktemp)"
"$BIN" --fixture "$ROOT/shared/fixtures/v0/ticks.ndjson" --out "$OUT"
diff -u "$ROOT/shared/fixtures/v0/golden.ndjson" "$OUT"
rm -f "$OUT"
echo "m4 golden ok"
```

`chmod +x cpp/tests/run_m4_golden.sh`

- [ ] **Step 4: CMake**

```cmake
add_library(ytta_m4_lib STATIC src/m4/pipeline.cpp)
target_include_directories(ytta_m4_lib PUBLIC src/m4 src/m2)
target_link_libraries(ytta_m4_lib PUBLIC ytta_m2_lib)

add_executable(ytta_m4 src/m4_main.cpp)
target_link_libraries(ytta_m4 PRIVATE ytta_m4_lib)

add_executable(ytta_m4_tests tests/m4_pipeline_test.cpp)
target_link_libraries(ytta_m4_tests PRIVATE ytta_m4_lib)
```

- [ ] **Step 5: Verify fail**

```bash
make test-m4
```

Expected: `ytta_m4_tests` fails and/or golden `diff` fails.

- [ ] **Step 6: Commit**

```bash
git add cpp/src/m4 cpp/src/m4_main.cpp cpp/tests/m4_pipeline_test.cpp cpp/tests/run_m4_golden.sh cpp/CMakeLists.txt
git commit -m "$(cat <<'EOF'
test(cpp): add failing M4 golden pipeline skeleton

Wire fixture replay + golden diff as the Ch 8–9 finish line.
EOF
)"
```

---

### Task 9: M5 strategy stub + failing tests

**Files:**
- Create: `cpp/src/m5/strategy.hpp`, `strategy.cpp`
- Create: `cpp/tests/m5_strategy_test.cpp`
- Create: `shared/fixtures/m5/ticks_taker.ndjson` (small deterministic input)
- Create: `shared/fixtures/m5/expected_actions_taker.ndjson` (expected actions only)
- Modify: `cpp/CMakeLists.txt`

**Interfaces:**
- Consumes: m2 types (`Side`, order fields)
- Produces: `ytta::m5::Strategy::on_tick(const Tick&, const MdSnapshot&) -> Action`  
  where `Action` is `{ NOOP | NEW_ORDER | CANCEL, ... }` matching `specs/messages.md`.

Pick **liquidity-taking** as the default skeleton scenario (document MM as alternate in test comment).

- [ ] **Step 1: Write `cpp/src/m5/strategy.hpp` + stub `.cpp`**

```cpp
#pragma once
#include <cstdint>
#include <string>

namespace ytta::m5 {

enum class Side { Buy, Sell };
enum class ActionType { Noop, NewOrder, Cancel };

struct Tick {
  std::uint64_t ts_ns;
  Side side;
  std::int64_t price;
  std::int64_t qty;
};

struct MdSnapshot {
  std::int64_t best_bid{0};
  std::int64_t best_ask{0};
  std::int64_t best_bid_qty{0};
  std::int64_t best_ask_qty{0};
};

struct Action {
  ActionType type{ActionType::Noop};
  std::uint64_t cl_ord_id{0};
  Side side{Side::Buy};
  std::int64_t price{0};
  std::int64_t qty{0};
  std::string reason{"noop"};
};

class Strategy {
 public:
  // Naive taker (default skeleton):
  // - If best_ask > 0 and tick is Buy crossing ask: NEW_ORDER SELL? No —
  //   If tick side Buy and best_ask > 0 and tick.price >= best_ask: emit NEW_ORDER Buy at best_ask qty=1 id=next_id_
  // - If flat after fill simulation flag has_open_: next tick emits CANCEL of that id
  // Stub always returns Noop.
  Action on_tick(const Tick& tick, const MdSnapshot& md);

 private:
  std::uint64_t next_id_{1};
  bool has_open_{false};
  std::uint64_t open_id_{0};
};

}  // namespace ytta::m5
```

```cpp
#include "strategy.hpp"
namespace ytta::m5 {
Action Strategy::on_tick(const Tick&, const MdSnapshot&) {
  return Action{};  // stub Noop
}
}  // namespace ytta::m5
```

- [ ] **Step 2: Write fixtures**

`shared/fixtures/m5/ticks_taker.ndjson`:

```json
{"type":"tick","ts_ns":1000,"instrument":"INSTR1","side":"BUY","price":101,"qty":1,"seq":1}
{"type":"tick","ts_ns":2000,"instrument":"INSTR1","side":"BUY","price":100,"qty":1,"seq":2}
```

`shared/fixtures/m5/expected_actions_taker.ndjson`:

```json
{"type":"action","ts_ns":1000,"action":"NEW_ORDER","cl_ord_id":1,"side":"BUY","price":100,"qty":1,"reason":"take_ask"}
{"type":"action","ts_ns":2000,"action":"CANCEL","cl_ord_id":1,"reason":"done"}
```

Test harness feeds synthetic `MdSnapshot{best_ask:100,best_ask_qty:5}` on first tick and empty book on second; compares Action sequence to expected (field-wise, not raw NDJSON parse required if easier).

- [ ] **Step 3: Write `m5_strategy_test.cpp`** that constructs those two ticks/MD pairs, calls `on_tick`, expects NEW then CANCEL; stub yields FAIL.

- [ ] **Step 4: CMake + `make test-m5` expect FAIL**

```cmake
add_library(ytta_m5_lib STATIC src/m5/strategy.cpp)
target_include_directories(ytta_m5_lib PUBLIC src/m5)
add_executable(ytta_m5_tests tests/m5_strategy_test.cpp)
target_link_libraries(ytta_m5_tests PRIVATE ytta_m5_lib)
```

- [ ] **Step 5: Commit**

```bash
git add cpp/src/m5 cpp/tests/m5_strategy_test.cpp shared/fixtures/m5 cpp/CMakeLists.txt
git commit -m "$(cat <<'EOF'
test(cpp): add failing M5 strategy skeleton

Provide a deterministic taker scenario; MM remains an documented alternate.
EOF
)"
```

---

### Task 10: M6 probe stub + failing tests

**Files:**
- Create: `cpp/src/m6/latency_probe.hpp`
- Create: `cpp/tests/m6_probe_test.cpp`
- Modify: `cpp/CMakeLists.txt`

**Interfaces:**
- Consumes: nothing
- Produces: `ytta::m6::LatencyProbe` with `void record(std::uint64_t e2e_ns);`, `std::uint64_t p50() const;`, `std::uint64_t p99() const;`, `std::string report() const;`  
  Stub returns 0 / empty; tests require non-zero percentiles after recording a known multiset.

- [ ] **Step 1: Stub header returning zeros**

- [ ] **Step 2: Test records `{10,20,30,...,100}` (ns) and expects `p50`/`p99` in plausible ranges; `report()` contains `p50` and `p99` substrings

- [ ] **Step 3: CMake + `make test-m6` expect FAIL**

- [ ] **Step 4: Commit**

```bash
git add cpp/src/m6 cpp/tests/m6_probe_test.cpp cpp/CMakeLists.txt
git commit -m "$(cat <<'EOF'
test(cpp): add failing M6 latency probe skeleton

Require p50/p99 reporting without enforcing absolute latency SLOs.
EOF
)"
```

---

### Task 11: Final verification + tests README

**Files:**
- Create: `cpp/tests/README.md`
- Modify: design success criteria mentally checked against tree

- [ ] **Step 1: Write `cpp/tests/README.md`**

```markdown
# C++ milestone tests

| Command | Expected after reset |
|---------|----------------------|
| `make test-m0` | PASS |
| `make test-m1` … `make test-m6` | FAIL until you implement |

Golden (M4): `cpp/tests/run_m4_golden.sh` compares to `shared/fixtures/v0/golden.ndjson`.
```

- [ ] **Step 2: End-state verification**

```bash
make test-m0
# must pass

for n in 1 2 3 4 5 6; do
  if make test-m$n; then
    echo "UNEXPECTED PASS: m$n"; exit 1
  else
    echo "m$n fails as expected"
  fi
done

test -f docs/BOOK_PATH.md
test -f docs/PROGRESS.md
test -f specs/messages.md
test -f shared/fixtures/v0/golden.ndjson
test -f go/README.md && test -f rust/README.md && test -f java/README.md
test ! -d cpp/src/v0
```

Expected: M0 pass; M1–M6 fail; curriculum + fixtures present; no old `v0` tree.

- [ ] **Step 3: Commit**

```bash
git add cpp/tests/README.md
git commit -m "$(cat <<'EOF'
docs(cpp): document DIY milestone test expectations

Spell out which targets pass after reset versus which the learner must green.
EOF
)"
```

---

## Self-review (plan vs spec)

| Spec requirement | Task |
|------------------|------|
| Remove production implementations | Task 3 |
| `BOOK_PATH` + `PROGRESS` with M0–M6 | Task 2 |
| Delete old stage docs (no archive tree) | Task 1 |
| C++ skeleton tests fail until implemented (M0 smoke passes) | Tasks 4–10, verified in 11 |
| go/rust/java non-blocking stubs | Task 3 |
| README DIY one-screen | Task 2 |
| Keep shared fixtures/schemas for M2+ | Task 3 (keep), Task 8 (use) |
| No reference/ backup tree | honored globally |
| Do not re-implement engine for the user | stubs only in Tasks 5–10 |

Placeholder scan: no TBD/TODO implementation handoffs; APIs named consistently `ytta::mN`.
