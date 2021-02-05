QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14
CONFIG += optimize_full

QMAKE_CXXFLAGS+= -fopenmp
QMAKE_LFLAGS +=  -fopenmp

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/colored_bayer.cpp \
    src/dng_raw.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/raw_utils.cpp \
    src/renderer.cpp \
    src/settings.cpp \
    src/simple_debayer.cpp \
    src/stb_image.cpp \
    src/tiny_dng_loader.cpp

HEADERS += \
    src/base_debayer.h \
    src/base_format.h \
    src/colored_bayer.h \
    src/dng_raw.h \
    src/mainwindow.h \
    src/raw_utils.h \
    src/renderer.h \
    src/settings.h \
    src/simple_debayer.h \
    src/stb_image.h \
    src/tiny_dng_loader.h

FORMS += \
    src/mainwindow.ui

TRANSLATIONS += \
    raw-postproc_ru_RU.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
