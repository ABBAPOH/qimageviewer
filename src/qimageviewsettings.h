#ifndef QIMAGEVIEWSETTINGS_H
#define QIMAGEVIEWSETTINGS_H

#include <QColor>

class QImageViewSettingsPrivate;
class QImageViewSettings
{
    Q_DECLARE_PRIVATE(QImageViewSettings)

public:
    enum ImageBackgroundType {
        None,
        Chess,
        SolidColor
    };

    QImageViewSettings();
    ~QImageViewSettings();

    static QImageViewSettings *globalSettings();

    QColor imageBackgroundColor() const;
    void setImageBackgroundColor(const QColor &color);

    ImageBackgroundType imageBackgroundType() const;
    void setiImageBackgroundType(ImageBackgroundType type);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);

private:
    QImageViewSettingsPrivate *d_ptr;

    friend class QSingleImageView;
};

#endif // QIMAGEVIEWSETTINGS_H
