#-------------------------------------------------
#
# Project created by QtCreator 2012-05-05T19:33:42
#
#-------------------------------------------------

QT       += core gui

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QImageViewer
TEMPLATE = app


SOURCES += main.cpp\
    qsingleimageview.cpp \
    mainwindow.cpp \
    qimageviewsettings.cpp

HEADERS  += \
    qsingleimageview.h \
    qsingleimageview_p.h \
    mainwindow.h \
    qimageviewsettings.h \
    qimageviewsettings_p.h

FORMS += \
    mainwindow.ui
