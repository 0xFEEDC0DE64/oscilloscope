QT = core gui widgets multimedia

CONFIG += c++14
DEFINES += QT_DEPRECATED_WARNINGS QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
        audiodevice.cpp \
        basedevice.cpp \
        fakedevice.cpp \
        main.cpp \
        mainwindow.cpp \
        osciwidget.cpp

HEADERS += \
    audiodevice.h \
    basedevice.h \
    fakedevice.h \
    mainwindow.h \
    osciwidget.h

OTHER_FILES += \
    README.md

FORMS += \
    mainwindow.ui
