################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

#include( $${PWD}/../examples.pri )

TARGET       = PlotController

HEADERS = \
    settings.h \
    circularbuffer.h \
    plot.h \
    mainwindow.h \
    PlotController.h \
    DataSource.h

SOURCES = \
    circularbuffer.cpp \
    plot.cpp \
    mainwindow.cpp \
    main.cpp \
    PlotController.cpp

CONFIG += qt
CONFIG += qwt
QT += widgets
QT += printsupport
QT += opengl

INCLUDEPATH += /usr/local/opt/qwt/lib/qwt.framework/Headers /usr/local/Cellar/qt5/5.7.0/lib/QtWidgets.framework/Headers/QApplication /usr/local/Cellar/qt5/5.7.0/lib/QtWidgets.framework/Headers/
LIBS += -L /usr/local/opt/qwt/lib/qwt.framework/ /usr/local/opt/qwt/lib/qwt.framework/qwt
