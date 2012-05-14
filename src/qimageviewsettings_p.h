#ifndef QIMAGEVIEWSETTINGS_P_H
#define QIMAGEVIEWSETTINGS_P_H

#include "qimageviewsettings.h"

class QSingleImageView;

class QImageViewSettingsPrivate
{
public:
    void addView(QSingleImageView *view);
    void removeView(QSingleImageView *view);

    void updateViews();

public:
    QImageViewSettings::ImageBackgroundType imageBackgroundType;
    QColor imageBackgroundColor;
    QColor backgroundColor;

private:
    QList<QSingleImageView *> views;
};

#endif // QIMAGEVIEWSETTINGS_P_H
