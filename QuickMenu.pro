#-------------------------------------------------
#
# Project created by QtCreator 2014-10-09T16:03:52
#
#-------------------------------------------------

QT       += core gui network svg
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QuickMenu
TEMPLATE = app


SOURCES += main.cpp \
    quickmenu.cpp

HEADERS  += \
    quickmenu.h

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    default-icon.svg \
    sample.json \
    LICENSE
