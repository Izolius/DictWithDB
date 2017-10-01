QT += core sql
QT -= gui

CONFIG += c++11

TARGET = DictionaryToDB
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    CDict.cpp

HEADERS += \
    CDict.h

RESOURCES += \
    creationsql.qrc
