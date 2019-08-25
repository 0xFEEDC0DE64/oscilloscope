QT = core gui widgets multimedia

CONFIG += c++17
DEFINES += QT_DEPRECATED_WARNINGS QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
        device.cpp \
        main.cpp \
        osciwidget.cpp

HEADERS += \
    device.h \
    osciwidget.h

STATECHARTS +=
