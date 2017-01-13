INCLUDEPATH *= $$PWD
DEPENDPATH  *= $$PWD

SOURCES += $$PWD/main.cpp\
    $$PWD/qimageview.cpp \
    $$PWD/mainwindow.cpp \
    $$PWD/qimageviewsettings.cpp \
    $$PWD/preferenceswidget.cpp \
    $$PWD/qcolorbutton.cpp \
    $$PWD/qimageresizedialog.cpp \
    $$PWD/windowsmenu.cpp \
    $$PWD/application.cpp

HEADERS  += \
    $$PWD/qimageview.h \
    $$PWD/qimageview_p.h \
    $$PWD/mainwindow.h \
    $$PWD/qimageviewsettings.h \
    $$PWD/qimageviewsettings_p.h \
    $$PWD/preferenceswidget.h \
    $$PWD/qcolorbutton.h \
    $$PWD/qcolorbutton_p.h \
    $$PWD/qimageresizedialog.h \
    $$PWD/windowsmenu.h \
    $$PWD/application.h

FORMS += \
    $$PWD/preferenceswidget.ui \
    $$PWD/qimageresizedialog.ui

RESOURCES += \
    $$PWD/resources.qrc

TRANSLATIONS = $$PWD//translations/qimageviewer_ru.ts

include(qtsingleapplication/qtsingleapplication.pri)
