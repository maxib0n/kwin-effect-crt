#!/usr/bin/env bash
# Toggle the CRT effect on/off at runtime via D-Bus (no restart).
set -euo pipefail

ID="kwin_effect_crt"
Q=""
for q in qdbus6 qdbus qdbus-qt6; do
    command -v "$q" >/dev/null && Q="$q" && break
done
[ -n "$Q" ] || { echo "qdbus not found"; exit 1; }

if [ "$("$Q" org.kde.KWin /Effects isEffectLoaded "$ID" 2>/dev/null || echo false)" = "true" ]; then
    "$Q" org.kde.KWin /Effects unloadEffect "$ID" >/dev/null
    echo "CRT: OFF"
else
    "$Q" org.kde.KWin /Effects loadEffect "$ID" >/dev/null
    echo "CRT: ON"
fi
