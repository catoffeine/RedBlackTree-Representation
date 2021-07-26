TEMPLATE = app
QT += qml quick widgets
CONFIG += debug
INCLUDEPATH += include
DESTDIR = build
HEADERS = $$files(include/*.h, true) $$files(RBTree/*.h, true)
#include/RBTree.h include/Backend.h
SOURCES = $$files(src/*.cpp, true) $$files(src/*.c, true) $$files(RBTree/*.c, true)
#main.cpp RBTree.c Backend.cpp
#RESOURCES = qml.qrc
