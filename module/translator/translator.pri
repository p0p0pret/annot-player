# translator.pri
# Since 11/2/2011

include(../../config.pri)

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/translator_config.h \
    $$PWD/translator.h

SOURCES += \
    $$PWD/translator.cc

QT      += core network xml

# EOF
