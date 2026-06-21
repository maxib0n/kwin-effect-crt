add_definitions(-DTRANSLATION_DOMAIN=\"kwin_effect_crt\")

set(kcm_SRCS
    KCM.cpp
    plugin.cpp
)
kconfig_add_kcfg_files(kcm_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../Config.kcfgc)

kcoreaddons_add_plugin(kwin_effect_crt_config
    SOURCES ${kcm_SRCS}
    INSTALL_NAMESPACE "kwin/effects/configs"
)
target_link_libraries(kwin_effect_crt_config
    Qt6::DBus
    Qt6::Widgets
    Qt6::CorePrivate
    KF6::KCMUtils
    KF6::ConfigWidgets
    KF6::ConfigGui
    KF6::I18n
)
