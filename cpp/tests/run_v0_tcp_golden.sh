#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD="${ROOT}/cpp/build"
ENGINE="${BUILD}/ytta_engine"
CLIENT="${BUILD}/ytta_client"
FIXTURE="${ROOT}/shared/fixtures/v0/ticks.ndjson"
GOLDEN="${ROOT}/shared/fixtures/v0/golden.ndjson"

if [[ ! -x "${ENGINE}" || ! -x "${CLIENT}" ]]; then
  cmake -S "${ROOT}/cpp" -B "${BUILD}"
  cmake --build "${BUILD}" --target ytta_engine ytta_client
fi

# Bind ephemeral port via engine --port 0, parse ready line.
PORT_FILE="$(mktemp)"
ENGINE_LOG="$(mktemp)"
OUT="$(mktemp)"
ENGINE_PID=""

cleanup() {
  if [[ -n "${ENGINE_PID}" ]] && kill -0 "${ENGINE_PID}" 2>/dev/null; then
    kill "${ENGINE_PID}" 2>/dev/null || true
    wait "${ENGINE_PID}" 2>/dev/null || true
  fi
  rm -f "${PORT_FILE}" "${ENGINE_LOG}" "${OUT}"
}
trap cleanup EXIT

"${ENGINE}" --port 0 >"${ENGINE_LOG}" 2>&1 &
ENGINE_PID=$!

# Wait for engine_ready JSON on stderr (merged into ENGINE_LOG).
PORT=""
for _ in $(seq 1 50); do
  if grep -q '"type":"engine_ready"' "${ENGINE_LOG}" 2>/dev/null; then
    PORT="$(sed -n 's/.*"port":\([0-9][0-9]*\).*/\1/p' "${ENGINE_LOG}" | head -n1)"
    break
  fi
  if ! kill -0 "${ENGINE_PID}" 2>/dev/null; then
    echo "engine exited before ready:" >&2
    cat "${ENGINE_LOG}" >&2
    exit 1
  fi
  sleep 0.05
done

if [[ -z "${PORT}" ]]; then
  echo "timed out waiting for engine_ready" >&2
  cat "${ENGINE_LOG}" >&2
  exit 1
fi

"${CLIENT}" --host 127.0.0.1 --port "${PORT}" --fixture "${FIXTURE}" --out "${OUT}" \
  >/dev/null 2>/dev/null

if ! cmp -s "${OUT}" "${GOLDEN}"; then
  echo "golden drift (tcp): output differs from ${GOLDEN}" >&2
  diff -u "${GOLDEN}" "${OUT}" >&2 || true
  exit 1
fi

echo "v0 golden ok (tcp)"
