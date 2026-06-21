#!/usr/bin/env bash
# Test the effect in a NESTED KWin Wayland session (a window), using the local
# build — no sudo, no changes to your real ~/.config, no risk to your session.
# A nested compositor always loads the latest build/ binary (the real session
# caches the loaded plugin until you log out).
#
#   ./scripts/test-nested.sh [app]
#
# Quick params via env:
#   MASK=0|1|2 (grille|shadow|slot)  COLOR=0|1|2|3 (rgb|green|amber|bw)
#   MS=mask strength  SS=scanline strength  GLOW=..  CURV=..  VIG=..  SCALE=..
#   e.g.  MASK=1 COLOR=2 GLOW=0.2 SCALE=1.25 ./scripts/test-nested.sh
set -euo pipefail

SRC="$(cd "$(dirname "$0")/.." && pwd)"
SO="$SRC/build/bin/kwin/effects/plugins/kwin_effect_crt.so"
[ -f "$SO" ] || { echo "No build found. Run:  cmake --build build --parallel"; exit 1; }

APP="${1:-konsole}"

# Isolated config: enables the effect only inside the nested session.
CFG="$(mktemp -d /tmp/crt-test.XXXXXX)"
trap 'rm -rf "$CFG" "$LAUNCH"' EXIT
cat > "$CFG/kwinrc" <<EOF
[Plugins]
kwin_effect_crtEnabled=true

[CRT]
Brightness=${BRIGHT:-1.0}
MaskType=${MASK:-0}
MaskStrength=${MS:-0.2}
MaskSize=${MSZ:-3}
ColorScheme=${COLOR:-0}
ScanlineStrength=${SS:-0.15}
ScanlineSize=${SSZ:-4}
ScanlineBeam=${BEAM:-1.0}
Glow=${GLOW:-0.0}
Curvature=${CURV:-0.0}
Vignette=${VIG:-0.0}
EOF

# kwin_wayland parses dashed args as its own, so wrap the app in a launcher.
LAUNCH="$(mktemp /tmp/crt-launch.XXXXXX.sh)"
if [ "$APP" = "konsole" ]; then
  cat > "$LAUNCH" <<'EOF'
#!/usr/bin/env bash
exec konsole -e bash -c '
  (fastfetch 2>/dev/null || neofetch 2>/dev/null || ls --color=always -la /usr/share/applications | head -40)
  echo; echo "Type colored commands to judge the CRT. Close the window to exit."
  exec bash'
EOF
else
  printf '#!/usr/bin/env bash\nexec %s\n' "$APP" > "$LAUNCH"
fi
chmod +x "$LAUNCH"

echo ">> Nested KWin starting (effect: kwin_effect_crt). Close the window to exit."
echo ">> Your real session is untouched."

exec env \
    QT_PLUGIN_PATH="$SRC/build/bin:${QT_PLUGIN_PATH:-}" \
    XDG_CONFIG_HOME="$CFG" \
    dbus-run-session -- \
    kwin_wayland --width 1600 --height 900 --scale "${SCALE:-1.25}" --no-lockscreen "$LAUNCH"
