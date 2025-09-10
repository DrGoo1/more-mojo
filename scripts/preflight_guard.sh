#!/usr/bin/env bash
set -euo pipefail
SRC="${1:-app/Sources}"

fail() { echo "❌ $*"; exit 1; }

# 1) InterpMode/ProcessorParams: declared exactly once
IM_COUNT=$(grep -RIn --include="*.swift" -E "enum\s+InterpMode" "$SRC" | wc -l | tr -d ' ')
PP_COUNT=$(grep -RIn --include="*.swift" -E "struct\s+ProcessorParams\b" "$SRC" | wc -l | tr -d ' ')
[ "$IM_COUNT" = "1" ] || fail "InterpMode must be defined exactly once (found $IM_COUNT)."
[ "$PP_COUNT" = "1" ] || fail "ProcessorParams must be defined exactly once (found $PP_COUNT)."

# 2) No nested ProcessorParams.InterpMode usages
if grep -RIn --include="*.swift" -E "ProcessorParams\.InterpMode" "$SRC" >/dev/null 2>&1; then
  fail "Found nested ProcessorParams.InterpMode; use top-level InterpMode instead."
fi

# 3) macOS 11: no .borderedProminent
if grep -RIn --include="*.swift" -E "\.buttonStyle\(\.borderedProminent\)" "$SRC" >/dev/null 2>&1; then
  fail "Found .borderedProminent (macOS 12+); use PMXProminent() instead."
fi

# 4) Duplicate faceplate/interpolator files
FPL=$(ls "$SRC"/*Faceplate*.swift 2>/dev/null | wc -l | tr -d ' ')
[ "$FPL" -le 1 ] || fail "Multiple Faceplate overlay sources detected; keep exactly one."

# Check Interpolator files - avoid double counting when one matches both patterns
INT_FILES=$(ls "$SRC"/Interpolator*.swift 2>/dev/null | sort -u)
INT_FILES_COUNT=$(echo "$INT_FILES" | grep -v '^$' | wc -l | tr -d ' ')
[ "$INT_FILES_COUNT" -le 1 ] || fail "Multiple Interpolator sources detected; keep exactly one."

echo "✅ Preflight guard passed."
