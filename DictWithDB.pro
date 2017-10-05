QT += core sql
QT -= gui

CONFIG += c++11

TARGET = DictWithDB
CONFIG += console
CONFIG -= app_bundle
CONFIG +=c++14

TEMPLATE = app

SOURCES += main.cpp \
    CDict.cpp \
    CWord.cpp

HEADERS += \
    CDict.h \
    CWord.h

RESOURCES += \
    creationsql.qrc
