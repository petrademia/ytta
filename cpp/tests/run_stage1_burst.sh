#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD="${ROOT}/cpp/build"
BIN="${BUILD}/ytta_v0"
FIXTURE="${ROOT}/shared/fixtures/v1/ticks_burst.ndjson"
OUT="$(mktemp)"
LAT="$(mktemp)"

if [[ ! -x "${BIN}" ]]; then
  cmake -S "${ROOT}/cpp" -B "${BUILD}"
  cmake --build "${BUILD}" --target ytta_v0
fi

"${BIN}" --mode=queued --fixture "${FIXTURE}" --out "${OUT}" >/dev/null 2>"${LAT}"

if ! grep -q '"drops":0' "${LAT}"; then
  echo "burst smoke: expected drops:0" >&2
  cat "${LAT}" >&2
  rm -f "${OUT}" "${LAT}"
  exit 1
fi

ACTIONS=$(grep -c '"type":"action"' "${OUT}" || true)
TICKS=$(grep -c '"type":"tick"' "${FIXTURE}" || true)
if [[ "${ACTIONS}" -ne "${TICKS}" ]]; then
  echo "burst smoke: action count ${ACTIONS} != tick count ${TICKS}" >&2
  rm -f "${OUT}" "${LAT}"
  exit 1
fi

echo "stage1 burst ok (ticks=${TICKS}, actions=${ACTIONS})"
cat "${LAT}"
rm -f "${OUT}" "${LAT}"
