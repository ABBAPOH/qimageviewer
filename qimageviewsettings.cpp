#include "qimageviewsettings.h"
#include "qimageviewsettings_p.h"

#include <QApplication>

#include "qsingleimageview.h"

QImageViewSettings::QImageViewSettings() :
    d_ptr(new QImageViewSettingsPrivate)
{
    Q_D(QImageViewSettings);

    d->imageBackgroundType = None;
    d->imageBackgroundType = Chess;
//    d->imageBackgroundType = SolidColor;
    d->imageBackgroundColor = qApp->palette().color(QPalette::Base);
    d->backgroundColor = qApp->palette().color(QPalette::Window).darker(150);
}

QImageViewSettings::~QImageViewSettings()
{
    delete d_ptr;
}

Q_GLOBAL_STATIC(QImageViewSettings, static_instance)

QImageViewSettings * QImageViewSettings::globalSettings()
{
    return static_instance();
}

QColor QImageViewSettings::imageBackgroundColor() const
{
    Q_D(const QImageViewSettings);

    return d->imageBackgroundColor;
}

void QImageViewSettings::setImageBackgroundColor(const QColor &color)
{
    // TODO: implement
}

QImageViewSettings::ImageBackgroundType QImageViewSettings::imageBackgroundType() const
{
    Q_D(const QImageViewSettings);

    return d->imageBackgroundType;
}

void QImageViewSettings::setiImageBackgroundType(QImageViewSettings::ImageBackgroundType type)
{
    // TODO: implement
}

QColor QImageViewSettings::backgroundColor() const
{
    Q_D(const QImageViewSettings);

    return d->backgroundColor;
}

void QImageViewSettings::setBackgroundColor(const QColor &color)
{
    // TODO: implement
}
