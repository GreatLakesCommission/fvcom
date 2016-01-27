TEMPLATE = lib
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CFLAGS += -std=gnu99

SOURCES += \
    util.c \
    nchelper.c \
    fvcom.c

HEADERS += \
    util.h \
    nchelper.h \
    fvcom.h \
    fvmddscontext.h


unix|win32: LIBS += -lgdal

unix|win32: LIBS += -lnetcdf

FLEX_FILE=dds.l
BISON_FILE=dds.y

OTHER_FILES += \
    $$BISON_FILE \
    $$FLEX_FILE

bisonsource.input = BISON_FILE
bisonsource.output = ${QMAKE_FILE_BASE}.c
bisonsource.commands = bison -d --defines=${QMAKE_FILE_BASE}.h -o ${QMAKE_FILE_BASE}.c ${QMAKE_FILE_IN}
bisonsource.variable_out = SOURCES
bisonsource.name = Bison Sources ${QMAKE_FILE_IN}
bisonsource.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += bisonsource

bisonheader.input = BISON_FILE
bisonheader.output = ${QMAKE_FILE_BASE}.h
bisonheader.commands = @true
bisonheader.variable_out = HEADERS
bisonheader.name = Bison Headers ${QMAKE_FILE_IN}
bisonheader.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += bisonheader

flexsource.input = FLEX_FILE
flexsource.output = ${QMAKE_FILE_BASE}.flex.c
flexsource.commands = flex --prefix=fvm -o ${QMAKE_FILE_BASE}.flex.c ${QMAKE_FILE_IN}
flexsource.variable_out = SOURCES
flexsource.name = Flex Sources ${QMAKE_FILE_IN}
flexsource.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += flexsource


