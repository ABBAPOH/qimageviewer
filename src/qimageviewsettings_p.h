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
    bool useOpenGL;

private:
    QList<QSingleImageView *> views;

    friend class QImageViewSettings;
};

#endif // QIMAGEVIEWSETTINGS_P_H
