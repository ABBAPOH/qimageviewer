#ifndef QSINGLEIMAGEVIEW_P_H
#define QSINGLEIMAGEVIEW_P_H

#include "qsingleimageview.h"

#include <QtCore/QVariantAnimation>

class ZoomAnimation : public QVariantAnimation
{
    Q_OBJECT

public:
    explicit ZoomAnimation(QSingleImageViewPrivate *dd, QObject *parent = 0);

protected:
    void updateCurrentValue(const QVariant &value);

private:
    QSingleImageViewPrivate *d;
};

class QSingleImageViewPrivate
{
    Q_DECLARE_PUBLIC(QSingleImageView)

public:
    explicit QSingleImageViewPrivate(QSingleImageView *qq) :
        zoomFactor(1.0),
        visualZoomFactor(1.0),
        zoomAnimation(this),
        q_ptr(qq)
    {}

    void setZoomFactor(qreal factor);
    void setVisualZoomFactor(qreal factor);
    void updateScrollBars();

public:
    QImage image;
    QPixmap originalPixmap;
    QPixmap pixmap;

    qreal zoomFactor;
    qreal visualZoomFactor;
    ZoomAnimation zoomAnimation;

    QPoint prevPos;

private:
    QSingleImageView *q_ptr;
};

#endif // QSINGLEIMAGEVIEW_P_H
