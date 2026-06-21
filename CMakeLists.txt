cmake_minimum_required(VERSION 3.20)

project(crt_trinitron VERSION 0.1.0 LANGUAGES CXX)

find_package(ECM 5.100 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(QT_MAJOR_VERSION 6)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick DBus CorePrivate)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(KF6 REQUIRED COMPONENTS CoreAddons I18n Config ConfigWidgets KCMUtils)
find_package(KWin REQUIRED COMPONENTS kwineffects)
find_package(epoxy REQUIRED)

add_subdirectory(src)
