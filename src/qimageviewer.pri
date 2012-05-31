INCLUDEPATH *= $$PWD
DEPENDPATH  *= $$PWD

SOURCES += main.cpp\
    qimageview.cpp \
    mainwindow.cpp \
    qimageviewsettings.cpp \
    preferenceswidget.cpp \
    qcolorbutton.cpp \
    src/qimageresizedialog.cpp \
    src/welcomewindow.cpp

HEADERS  += \
    qimageview.h \
    qimageview_p.h \
    mainwindow.h \
    qimageviewsettings.h \
    qimageviewsettings_p.h \
    preferenceswidget.h \
    qcolorbutton.h \
    qcolorbutton_p.h \
    src/qimageresizedialog.h \
    src/welcomewindow.h

FORMS += \
    mainwindow.ui \
    preferenceswidget.ui \
    src/qimageresizedialog.ui \
    src/welcomewindow.ui

RESOURCES += \
    src/resources.qrc

