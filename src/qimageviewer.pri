INCLUDEPATH *= $$PWD
DEPENDPATH  *= $$PWD

SOURCES += main.cpp\
    qsingleimageview.cpp \
    mainwindow.cpp \
    qimageviewsettings.cpp \
    preferenceswidget.cpp \
    qcolorbutton.cpp \
    src/qimageresizedialog.cpp

HEADERS  += \
    qsingleimageview.h \
    qsingleimageview_p.h \
    mainwindow.h \
    qimageviewsettings.h \
    qimageviewsettings_p.h \
    preferenceswidget.h \
    qcolorbutton.h \
    qcolorbutton_p.h \
    src/qimageresizedialog.h

FORMS += \
    mainwindow.ui \
    preferenceswidget.ui \
    src/qimageresizedialog.ui

