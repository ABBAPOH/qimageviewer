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
    src/application.cpp

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
    src/application.h

FORMS += \
    mainwindow.ui \
    preferenceswidget.ui \
    qimageresizedialog.ui

RESOURCES += \
    src/resources.qrc

