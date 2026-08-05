# YTTA DIY Book Path (Design)

Date: 2026-08-06  
Status: **approved** (pending user review of this written spec)

## Goal

Turn YTTA into an empty, implement-yourself lab guided by *Building Low Latency Applications with C++* (Sourav Ghosh, Packt). The repo keeps curriculum docs and skeleton tests as guardrails. You implement the C++ program milestone by milestone. Prior completed implementations are not preserved in-tree; recover from git history if needed.

## Locked decisions

| Topic | Decision |
|-------|----------|
| Working tree | Empty of production implementations; docs + scaffolds + skeleton tests remain |
| Guardrails | Plan/milestones + skeleton tests (fail until implemented); shared fixtures/schemas from M2+ |
| Language critical path | C++ only for DIY milestones |
| Other languages | `go/`, `rust/`, `java/` reduced to “port later” stubs; not part of finish criteria |
| Backup | No special archive branch/tag/`reference/` copy; rely on existing git history |
| Milestone spine | Book-led, collapsed by book **parts** into M0–M6 (Approach 2) |
| Old Stage 0/1/2 | Archived vocabulary only; not the active checklist |

## Explicitly rejected

- Keeping working implementations on `main` with a DIY overlay only
- Polyglot lockstep (all four languages per milestone) as the current path
- One milestone per book chapter (too many design-only chapters with thin tests)
- Hybrid “primitives first then old YTTA stages as primary names”
- In-repo frozen `reference/` implementation tree

## Book reference

Primary text: *Building Low Latency Applications with C++* (Packt).

| Part | Chapters | Theme |
|------|----------|--------|
| 1 | 1–4 | Concepts + C++ building blocks |
| 2 | 5–7 | Trading exchange (design, matching engine, participant I/O) |
| 3 | 8–10 | Algo trading client (MD/orders, building blocks, MM / liquidity taking) |
| 4 | 11–12 | Instrumentation and optimization |

YTTA remains paper/replay and educational. Absolute microsecond SLOs are not merge gates.

## Milestone map (active checklist)

| ID | Book | You implement (C++) | Skeleton tests / guardrails |
|----|------|---------------------|-----------------------------|
| **M0** | Ch 1–3 | Notes + repo scaffold only | Build system works; empty smoke target passes |
| **M1** | Ch 4 | Thread utils, SPSC (or LF queue), object pool, monotonic clock helpers | Unit: queue push/pop, pool reuse, clock monotonicity |
| **M2** | Ch 5–6 | Order book + matching engine (paper, one instrument) | Unit/scenario: resting NEW, cross → fills, CANCEL accept/reject; optional early golden NDJSON |
| **M3** | Ch 7 | Localhost TCP: order gateway + market-data publish path | Integration: client↔engine framing; order → ack/fill round-trip |
| **M4** | Ch 8–9 | Client MD + order path + algo building blocks (in-process or over M3) | Pipeline: fixture ticks → ordered events (golden compare) |
| **M5** | Ch 10 | One market-making **or** one liquidity-taking strategy (choose one first) | Deterministic strategy scenario fixtures |
| **M6** | Ch 11–12 | LatencyProbe / histograms + a small optimize pass | Measurement harness runs; report p50/p99; no hard µs SLO |

**Done rule:** a milestone is complete when its C++ skeleton tests pass.

### Mapping note (archived Stage vocabulary)

Old YTTA Stage 0/1/2/3 names may still appear under `docs/archive/`. They are not the active tracker. Rough overlap for orientation only:

- Stage 0 ≈ parts of M2 + M4
- Stage 1 ≈ parts of M1 + M6
- Stage 2 ≈ M3
- Stage 3 ≈ M5

## Target repo shape (after reset)

### Keep / rewrite

- `README.md` — DIY-first: book → milestones → how to run tests
- `docs/PROGRESS.md` — M0–M6 checkboxes (C++ critical path)
- `docs/BOOK_PATH.md` — per-milestone chapter links, goals, owned files, verify commands
- Existing architecture/specs/plans/handoffs → `docs/archive/` (historical; not the spine)

### Remove from working tree

- Current production sources under `cpp/src`, `go/internal`, `rust/src`, `java/src` (and equivalents)
- Stage-complete binaries/tests that assume the finished pre-DIY system
- Anything that makes `main` look like a finished polyglot product rather than an empty lab

Recoverable via git history on `main` / prior commits.

### Scaffolds to leave

- `cpp/` — CMake + Makefile + empty/stub `src/` slots + skeleton tests labeled by milestone (`m1_*`, `m2_*`, …)
- `go/`, `rust/`, `java/` — short “port later” README only
- `shared/schemas/` and `shared/fixtures/` — kept/adapted as contracts starting M2; M0–M1 need no golden

## Day-to-day workflow

1. Open `docs/BOOK_PATH.md` for the current milestone.
2. Read the named Packt chapters.
3. Implement only under `cpp/` for that milestone’s scope.
4. Run the milestone’s tests (e.g. `make test-m1` or ctest labels).
5. Tick `docs/PROGRESS.md` when green.

Skeleton tests are real assertions written up front. Stubs fail for the right reason (missing behavior), not a broken harness. Peeking at old implementations is optional via git history, not required by the curriculum.

## Fixtures and schemas

- **Schemas** (`shared/schemas/`): shared message contracts (tick, action, ack, fill, md).
- **Golden fixtures** (`shared/fixtures/`): recorded input ticks plus expected ordered event output for behavioral parity.
- Required from **M2+**. M0–M1 are scaffold/unit only.

## Out of scope for the reset

- Re-implementing the matching engine, strategies, or networking for the user
- Opening Go/Rust/Java port milestones
- Live feeds, real money, multi-instrument exchange
- Creating a special archive branch/tag or `reference/` tree

## Success criteria for the conversion

- [ ] Production implementations removed from the working tree
- [ ] `docs/BOOK_PATH.md` and `docs/PROGRESS.md` describe M0–M6 with book chapter mapping
- [ ] Old stage docs live under `docs/archive/` (or equivalent) and are clearly historical
- [ ] C++ skeleton tests exist per milestone and fail until implemented (except M0 smoke)
- [ ] `go/` / `rust/` / `java/` are non-blocking stubs
- [ ] README explains the DIY path in one screen

## Assumptions

- You own the Packt book and will read chapters as the primary teaching material.
- Idiomatic C++ for learning beats cloning the book’s repository structure line-for-line.
- Paper/replay remains enough through M6; live trading is never required for this path.
