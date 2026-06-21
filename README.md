# CRT - KWin Desktop Effect

A fullscreen CRT post-processing effect for KDE Plasma 6 / KWin 6 (Wayland). It
runs the whole desktop through a shader that emulates a cathode-ray tube: phosphor
mask, scanlines, glow and curvature. The pipeline is gamma-correct and the masks
are luminance-preserving, so text stays readable and the image is not darkened.

## Features

- Three mask types: aperture grille (Trinitron), shadow mask, slot mask
- Four phosphor schemes: RGB, green (P1), amber (P3), white (B/W)
- Gaussian-beam scanlines
- Glow, curvature with bezel, vignette
- Brightness, contrast, gamma, saturation
- Everything is adjustable live from the effect's settings page

## Requirements

KDE Plasma 6 / KWin 6 on Wayland with OpenGL compositing.

Build dependencies (Fedora):

```bash
sudo dnf install extra-cmake-modules kwin-devel qt6-qtbase-devel \
  qt6-qtbase-private-devel qt6-qtdeclarative-devel kf6-kcoreaddons-devel \
  kf6-kconfig-devel kf6-kconfigwidgets-devel kf6-kcmutils-devel kf6-ki18n-devel \
  kf6-kwindowsystem-devel libdrm-devel libepoxy-devel mesa-libgbm-devel \
  vulkan-loader-devel vulkan-headers wayland-protocols-devel
```

## Install on Fedora (COPR)

```bash
sudo dnf copr enable maxib0n/kwin-effect-crt
sudo dnf install kwin-effect-crt
```

Then log out and back in, and enable it in System Settings > Desktop Effects > CRT.

## Build and install from source

```bash
./build-and-install.sh
```

This builds the plugin and installs it to `/usr` (sudo is used only for the install
step). KWin keeps a loaded plugin in memory, so after installing, log out and back
in to load the new binary.

## Usage

Enable it in System Settings > Desktop Effects > CRT, then open the settings button
next to it to choose the mask type, phosphor and the rest. Settings are stored under
`kwinrc [CRT]`.

To toggle it from a terminal:

```bash
./toggle.sh
```

## Develop and test

A nested KWin session lets you try the effect in a window without touching your real
session, using the local build:

```bash
cmake -B build -S . && cmake --build build --parallel
./scripts/test-nested.sh
MASK=2 COLOR=2 GLOW=0.25 ./scripts/test-nested.sh
```

Quick parameters are passed via environment variables (`MASK`, `COLOR`, `MS`, `SS`,
`GLOW`, `CURV`, `VIG`, `SCALE`).

## How it works

A `KWin::Effect` that runs first in the chain. In `paintScreen()` it renders the
composited scene into a per-output offscreen texture, then draws one fullscreen quad
sampling that texture through the CRT fragment shader. The shader is embedded in the
binary. See `src/Effect.cpp`.

## Uninstall

```bash
./uninstall.sh
```

## License

GPL-3.0-or-later.
