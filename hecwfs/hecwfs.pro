TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CFLAGS += -std=c99

SOURCES += hecwfs.c


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-fvcomlib-Desktop-Debug/release/ -lfvcomlib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-fvcomlib-Desktop-Debug/debug/ -lfvcomlib
else:unix:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-fvcomlib-Desktop-Release/ -lfvcomlib
else:unix:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-fvcomlib-Desktop-Debug/ -lfvcomlib


INCLUDEPATH += $$PWD/../fvcomlib
DEPENDPATH += $$PWD/../fvcomlib

INCLUDEPATH += /usr/local/include
