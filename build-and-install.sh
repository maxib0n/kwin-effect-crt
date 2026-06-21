#!/usr/bin/env bash
# Build and install the CRT effect for KWin 6 (Plasma 6).
# Build needs no privileges; install into /usr uses sudo.
#
# NOTE: KWin keeps the loaded plugin in memory — after installing a new build,
# log out and back in (or restart the compositor) to load it.
set -euo pipefail

SRC="$(cd "$(dirname "$0")" && pwd)"
cd "$SRC"

cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DKDE_INSTALL_LIBDIR=lib64 \
    -DKDE_INSTALL_USE_QT_SYS_PATHS=ON

cmake --build build --parallel
sudo cmake --install build

echo
echo "Installed. Log out and back in, then enable it in:"
echo "  System Settings > Desktop Effects > CRT"
