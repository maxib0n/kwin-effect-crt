#!/usr/bin/env bash
# Remove the CRT effect from all install locations.
set -euo pipefail

ID="kwin_effect_crt"

for q in qdbus6 qdbus qdbus-qt6; do
    command -v "$q" >/dev/null && "$q" org.kde.KWin /Effects unloadEffect "$ID" 2>/dev/null && break || true
done

sudo rm -f \
    /usr/lib64/qt6/plugins/kwin/effects/plugins/kwin_effect_crt.so \
    /usr/lib64/qt6/plugins/kwin/effects/configs/kwin_effect_crt_config.so \
    /usr/lib64/plugins/kwin/effects/plugins/kwin_effect_crt.so \
    /usr/lib64/plugins/kwin/effects/configs/kwin_effect_crt_config.so

command -v kwriteconfig6 >/dev/null && \
    kwriteconfig6 --file kwinrc --group Plugins --key "${ID}Enabled" false || true

echo "Removed. Log out and back in to unload it from the running compositor."
