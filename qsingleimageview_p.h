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

class QImageViewerRealAnimation : public QVariantAnimation
{
    Q_OBJECT

    typedef void (QSingleImageViewPrivate::*Func)(qreal);

public:
    explicit QImageViewerRealAnimation(Func f, QSingleImageViewPrivate *dd, QObject *parent = 0);

protected:
    void updateCurrentValue(const QVariant &value);

private:
    QSingleImageViewPrivate *d;
    Func func;
};

class QSingleImageViewPrivate
{
    Q_DECLARE_PUBLIC(QSingleImageView)

public:
    explicit QSingleImageViewPrivate(QSingleImageView *qq);

    void setZoomFactor(qreal factor);
    void setVisualZoomFactor(qreal factor);

    void rotate(bool left);
    void setRotationAngle(qreal angle);

    void updateScrollBars();

    void animationFinished();
    void stopAnimations();

    void syncPixmap();

public:
    QImage image;
    QPixmap originalPixmap;
    QPixmap pixmap;

    qreal zoomFactor;
    qreal visualZoomFactor;
    ZoomAnimation zoomAnimation;

    qreal rotationAngle;
    QImageViewerRealAnimation rotateAnimation;

    QPoint prevPos;

private:
    QSingleImageView *q_ptr;
};

#endif // QSINGLEIMAGEVIEW_P_H
