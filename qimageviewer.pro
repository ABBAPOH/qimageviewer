#-------------------------------------------------
#
# Project created by QtCreator 2012-05-05T19:33:42
#
#-------------------------------------------------

QT       += core gui opengl

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qimageviewer
TEMPLATE = app
DESTDIR = bin

include(src/qimageviewer.pri)

win32 {
  RC_FILE = src/qimageviewer.rc
  ICON = src/icons/qimageviewer.ico
}

macx-* {
  ICON = src/icons/qimageviewer.icns
  QMAKE_BUNDLE_DATA += FILETYPES
  info.input = Info.plist.in
  info.output = $$OUT_PWD/QImageViewer.app/Contents/Info.plist
  QMAKE_SUBSTITUTES = info
}

OTHER_FILES += Info.plist.in
