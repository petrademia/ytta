#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BIN="$ROOT/cpp/build/ytta_m4"
OUT="$(mktemp)"
"$BIN" --fixture "$ROOT/shared/fixtures/v0/ticks.ndjson" --out "$OUT"
diff -u "$ROOT/shared/fixtures/v0/golden.ndjson" "$OUT"
rm -f "$OUT"
echo "m4 golden ok"
