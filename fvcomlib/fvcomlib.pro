TEMPLATE = lib
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    util.c

HEADERS += \
    util.h


unix|win32: LIBS += -lgdal

unix|win32: LIBS += -lnetcdf


