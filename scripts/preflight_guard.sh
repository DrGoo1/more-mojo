#!/usr/bin/env bash
set -euo pipefail
SRC="${1:-app/Sources}"

fail() { echo "❌ $*"; exit 1; }

# 1) InterpMode/ProcessorParams: declared exactly once
IM_COUNT=$(grep -RIn --include="*.swift" 'enum[[:space:]]+InterpMode' "$SRC" | wc -l | tr -d ' ')
PP_COUNT=$(grep -RIn --include="*.swift" 'struct[[:space:]]+ProcessorParams\b' "$SRC" | wc -l | tr -d ' ')
[ "$IM_COUNT" = "1" ] || fail "InterpMode must be defined exactly once (found $IM_COUNT)."
[ "$PP_COUNT" = "1" ] || fail "ProcessorParams must be defined exactly once (found $PP_COUNT)."

# 2) No nested ProcessorParams.InterpMode usages
if grep -RIn --include="*.swift" 'ProcessorParams\.InterpMode' "$SRC" >/dev/null; then
  fail "Found nested ProcessorParams.InterpMode; use top-level InterpMode instead."
fi

# 3) macOS 11: no .borderedProminent
if grep -RIn --include="*.swift" '\.buttonStyle\(\.borderedProminent\)' "$SRC" >/dev/null; then
  fail "Found .borderedProminent (macOS 12+); use PMXProminent() instead."
fi

# 4) Duplicate faceplate/interpolator files
FPL=$(ls "$SRC"/*Faceplate*.swift 2>/dev/null | wc -l | tr -d ' ')
INT=$(ls "$SRC"/Interpolator*.swift "$SRC"/Interpolators*.swift 2>/dev/null | wc -l | tr -d ' ')
[ "$FPL" -le 1 ] || fail "Multiple Faceplate overlay sources detected; keep exactly one."
[ "$INT" -le 1 ] || fail "Multiple Interpolator sources detected; keep exactly one."

echo "✅ Preflight guard passed."
