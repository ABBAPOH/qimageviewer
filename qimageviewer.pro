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
    qimageviewsettings.cpp \
    preferenceswidget.cpp \
    qcolorbutton.cpp

HEADERS  += \
    qsingleimageview.h \
    qsingleimageview_p.h \
    mainwindow.h \
    qimageviewsettings.h \
    qimageviewsettings_p.h \
    preferenceswidget.h \
    qcolorbutton.h \
    qcolorbutton_p.h

FORMS += \
    mainwindow.ui \
    preferenceswidget.ui
