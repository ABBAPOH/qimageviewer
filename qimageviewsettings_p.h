#ifndef QIMAGEVIEWSETTINGS_P_H
#define QIMAGEVIEWSETTINGS_P_H

#include "qimageviewsettings.h"

class QImageViewSettingsPrivate
{
public:
    QImageViewSettings::ImageBackgroundType imageBackgroundType;
    QColor imageBackgroundColor;
    QColor backgroundColor;
};

#endif // QIMAGEVIEWSETTINGS_P_H
