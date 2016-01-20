TEMPLATE = lib
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CFLAGS += -std=gnu99

SOURCES += \
    util.c \
    nchelper.c \
    fvcom.cpp

HEADERS += \
    util.h \
    nchelper.h \
    fvcom.h


unix|win32: LIBS += -lgdal

unix|win32: LIBS += -lnetcdf


