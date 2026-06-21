Name:           kwin-effect-crt
Version:        0.1.0
Release:        1%{?dist}
Summary:        Fullscreen CRT desktop effect for KWin 6 (Plasma 6, Wayland)

License:        GPL-3.0-or-later
URL:            https://github.com/maxib0n/kwin-effect-crt
Source0:        %{url}/archive/refs/heads/main.tar.gz#/%{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  extra-cmake-modules
BuildRequires:  gcc-c++
BuildRequires:  kwin-devel
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtbase-private-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  kf6-kcoreaddons-devel
BuildRequires:  kf6-kconfig-devel
BuildRequires:  kf6-kconfigwidgets-devel
BuildRequires:  kf6-kcmutils-devel
BuildRequires:  kf6-ki18n-devel
BuildRequires:  kf6-kwindowsystem-devel
BuildRequires:  libepoxy-devel
BuildRequires:  libdrm-devel
BuildRequires:  mesa-libgbm-devel
BuildRequires:  vulkan-loader-devel
BuildRequires:  vulkan-headers
BuildRequires:  wayland-protocols-devel

Requires:       kwin-wayland

%description
A fullscreen CRT post-processing effect for KDE Plasma 6 / KWin 6 on Wayland.
It renders the whole desktop through a shader emulating a cathode-ray tube:
aperture-grille, shadow and slot masks, phosphor color schemes, Gaussian
scanlines, glow, curvature and vignette, all configurable from a settings page.

%prep
%autosetup -n %{name}-main

%build
%cmake -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
%cmake_build

%install
%cmake_install

%files
%license LICENSE
%doc README.md
%{_libdir}/qt6/plugins/kwin/effects/plugins/kwin_effect_crt.so
%{_libdir}/qt6/plugins/kwin/effects/configs/kwin_effect_crt_config.so

%changelog
* Sat Jun 21 2025 maxib0n <bedanamassimiliano@gmail.com> - 0.1.0-1
- Initial package
