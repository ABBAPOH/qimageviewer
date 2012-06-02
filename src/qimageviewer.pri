INCLUDEPATH *= $$PWD
DEPENDPATH  *= $$PWD

SOURCES += main.cpp\
    qimageview.cpp \
    mainwindow.cpp \
    qimageviewsettings.cpp \
    preferenceswidget.cpp \
    qcolorbutton.cpp \
    qimageresizedialog.cpp \
    windowsmenu.cpp \
    application.cpp

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
    windowsmenu.h \
    application.h

FORMS += \
    preferenceswidget.ui \
    qimageresizedialog.ui

RESOURCES += \
    src/resources.qrc

