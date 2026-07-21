#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD="${ROOT}/cpp/build"
BIN="${BUILD}/ytta_v0"
FIXTURE="${ROOT}/shared/fixtures/v0/ticks.ndjson"
GOLDEN="${ROOT}/shared/fixtures/v0/golden.ndjson"
OUT="$(mktemp)"

if [[ ! -x "${BIN}" ]]; then
  cmake -S "${ROOT}/cpp" -B "${BUILD}"
  cmake --build "${BUILD}" --target ytta_v0
fi

"${BIN}" --fixture "${FIXTURE}" --out "${OUT}" >/dev/null 2>/dev/null

if ! cmp -s "${OUT}" "${GOLDEN}"; then
  echo "golden drift: output differs from ${GOLDEN}" >&2
  diff -u "${GOLDEN}" "${OUT}" >&2 || true
  rm -f "${OUT}"
  exit 1
fi

rm -f "${OUT}"
echo "v0 golden ok"
