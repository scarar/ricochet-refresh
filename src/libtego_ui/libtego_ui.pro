QMAKE_INCLUDES = $${PWD}/../qmake_includes

include($${QMAKE_INCLUDES}/artifacts.pri)
include($${QMAKE_INCLUDES}/compiler_flags.pri)

TEMPLATE = lib
TARGET = tego_ui
CONFIG += staticlib

QT += core gui network quick widgets

CONFIG(release,debug|release):DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT

# INCLUDEPATH += $${PWD}
# INCLUDEPATH += $${PWD}/../extern/fmt/include

macx {
    QT += macextras
}

CONFIG += precompile_header
PRECOMPILED_HEADER = precomp.hpp

SOURCES += \
    libtego_callbacks.cpp \
    ui/ContactsModel.cpp \
    ui/LanguagesModel.cpp \
    ui/LinkedText.cpp \
    ui/MainWindow.cpp \

HEADERS += \
    libtego_callbacks.hpp \
    ui/ContactsModel.h \
    ui/LanguagesModel.h \
    ui/LinkedText.h \
    ui/MainWindow.h \

include($${QMAKE_INCLUDES}/protobuf.pri)
include($${QMAKE_INCLUDES}/openssl.pri)
include($${PWD}/../libtego/libtego.pri)
