# 文件说明：Qt qmake工程配置文件，声明模块依赖、源码、头文件、资源文件和UI文件。
QT += core gui widgets

CONFIG += c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = autoretailbot

INCLUDEPATH += $$PWD
DEFINES += PROJECT_SOURCE_DIR=\"$$PWD\"

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    controller/RobotController.cpp \
    config/ConfigLoader.cpp \
    hardware/ActuatorFactory.cpp \
    hardware/FakeRetailActuator.cpp \
    hardware/RealRetailActuator.cpp \
    interfaces/FakeTabletService.cpp \
    interfaces/FakeVisionService.cpp \
    interfaces/FakeVoiceService.cpp \
    interfaces/GrpcTabletService.cpp \
    interfaces/Ros2VisionService.cpp \
    interfaces/Ros2VoiceService.cpp \
    interfaces/TabletService.cpp \
    interfaces/TabletServiceFactory.cpp \
    interfaces/VisionService.cpp \
    interfaces/VisionServiceFactory.cpp \
    interfaces/VoiceService.cpp \
    interfaces/VoiceServiceFactory.cpp \
    mission/RobotMission.cpp \
    model/RobotTypes.cpp

HEADERS += \
    mainwindow.h \
    controller/RobotController.h \
    config/ConfigLoader.h \
    hardware/ActuatorFactory.h \
    hardware/FakeRetailActuator.h \
    hardware/HardwareInterfaces.h \
    hardware/RealRetailActuator.h \
    interfaces/FakeTabletService.h \
    interfaces/FakeVisionService.h \
    interfaces/FakeVoiceService.h \
    interfaces/GrpcTabletService.h \
    interfaces/Ros2VisionService.h \
    interfaces/Ros2VoiceService.h \
    interfaces/TabletService.h \
    interfaces/TabletServiceFactory.h \
    interfaces/VisionService.h \
    interfaces/VisionServiceFactory.h \
    interfaces/VoiceService.h \
    interfaces/VoiceServiceFactory.h \
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
