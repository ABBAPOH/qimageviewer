INCLUDEPATH *= $$PWD
DEPENDPATH  *= $$PWD

SOURCES += main.cpp\
    qimageview.cpp \
    mainwindow.cpp \
    qimageviewsettings.cpp \
    preferenceswidget.cpp \
    qcolorbutton.cpp \
    qimageresizedialog.cpp \
    welcomewindow.cpp \
    windowsmenu.cpp

HEADERS  += \
    qimageview.h \
    qimageview_p.h \
    mainwindow.h \
    qimageviewsettings.h \
    qimageviewsettings_p.h \
    preferenceswidget.h \
    qcolorbutton.h \
    qcolorbutton_p.h \
    qimageresizedialog.h \
    welcomewindow.h \
    windowsmenu.h

FORMS += \
    mainwindow.ui \
    preferenceswidget.ui \
    qimageresizedialog.ui \
    welcomewindow.ui

RESOURCES += \
    src/resources.qrc

