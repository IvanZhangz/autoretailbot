# 文件说明：Qt qmake工程配置文件，声明模块依赖、源码、头文件、资源文件和UI文件。
QT += core gui widgets

CONFIG += c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = Mvp_Cup

INCLUDEPATH += $$PWD
DEFINES += PROJECT_SOURCE_DIR=\"$$PWD\"

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    controller/RobotController.cpp \
    config/ConfigLoader.cpp \
    hardware/ActuatorFactory.cpp \
    hardware/FakeRetailActuator.cpp \
    interfaces/TabletService.cpp \
    interfaces/VoiceService.cpp \
    mission/RobotMission.cpp \
    model/RobotTypes.cpp

HEADERS += \
    mainwindow.h \
    controller/RobotController.h \
    config/ConfigLoader.h \
    hardware/ActuatorFactory.h \
    hardware/FakeRetailActuator.h \
    hardware/HardwareInterfaces.h \
    interfaces/TabletService.h \
    interfaces/VoiceService.h \
    mission/RobotMission.h \
    model/RobotTypes.h

DISTFILES += \
    config/store.json \
    config/factories.json \
    config/interfaces.json \
    config/task_policy.json \
    config/shelf_layout.json \
    config/product_catalog.json \
    config/inventory.json \
    config/home.json

RESOURCES += \
    resources.qrc

FORMS += \
    mainwindow.ui
