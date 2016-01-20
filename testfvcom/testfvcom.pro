TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CFLAGS += -std=c99

SOURCES += main.c
INCLUDEPATH += ../fvcomlib


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-fvcomlib-Desktop-Debug/release/ -lfvcomlib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-fvcomlib-Desktop-Debug/debug/ -lfvcomlib
else:unix: LIBS += -L$$PWD/../build-fvcomlib-Desktop-Debug/ -lfvcomlib

INCLUDEPATH += $$PWD/../build-fvcomlib-Desktop-Debug
DEPENDPATH += $$PWD/../build-fvcomlib-Desktop-Debug
