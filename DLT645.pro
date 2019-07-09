#-------------------------------------------------
#
# Project created by QtCreator 2019-07-07T10:32:17
# Bohdan Zikranets bohdan@kts-intek.com.ua
#-------------------------------------------------
QT       += core
QT       -= gui

CONFIG += c++11

TARGET = DLT645
TEMPLATE = lib
CONFIG += plugin

DEFINES += DLT645_LIBRARY

#for meteroperations.h
DEFINES += STANDARD_METER_PLUGIN
DEFINES += METERPLUGIN_FILETREE

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



SOURCES += dlt645.cpp \
        dt645encoderdecoder.cpp

HEADERS += dlt645.h \
        dlt645defines.h \
        dt645encoderdecoder.h

EXAMPLE_FILES = zbyralko.json

linux-beagleboard-g++:{
    target.path = /opt/matilda/plugin
    INSTALLS += target
}
include(../../../Matilda-units/meter-plugin-shared/meter-plugin-shared/meter-plugin-shared.pri)
include(../../../Matilda-units/matilda-base/type-converter/type-converter.pri)

