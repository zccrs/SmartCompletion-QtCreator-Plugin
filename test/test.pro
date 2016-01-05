QT += core
QT -= gui

TARGET = test-plugin
CONFIG += console c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += test.cpp

HEADERS += ../smartcompletionplugin_global.h
