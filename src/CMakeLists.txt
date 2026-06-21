set(effect_SRCS
    Effect.cpp
    plugin.cpp
)
kconfig_add_kcfg_files(effect_SRCS Config.kcfgc)

kcoreaddons_add_plugin(kwin_effect_crt
    SOURCES ${effect_SRCS}
    INSTALL_NAMESPACE "kwin/effects/plugins/"
)
target_compile_features(kwin_effect_crt PRIVATE cxx_std_20)
target_link_libraries(kwin_effect_crt
    Qt6::DBus
    Qt6::CorePrivate
    KF6::CoreAddons
    KF6::ConfigGui
    KWin::kwin
    epoxy::epoxy
)

add_subdirectory(kcm)
