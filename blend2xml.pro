QT       += core xml
QT       -= gui

TARGET = blend2xml
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
SOURCES += main.cpp blendtoxml.cpp
HEADERS +=  blendtoxml.h

CONFIG += c++11
