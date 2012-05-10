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

class AxisAnimation : public QVariantAnimation
{
    Q_OBJECT

public:
    explicit AxisAnimation(Qt::Axis axis, QSingleImageViewPrivate *dd, QObject *parent = 0);

    Qt::Axis axis() const { return m_axis; }
    qreal angle() const { return currentValue().toReal(); }

protected:
    void updateCurrentValue(const QVariant &value);

private:
    QSingleImageViewPrivate *d;
    Qt::Axis m_axis;
};

class QSingleImageViewPrivate
{
    Q_DECLARE_PUBLIC(QSingleImageView)

public:
    explicit QSingleImageViewPrivate(QSingleImageView *qq);

    void setZoomFactor(qreal factor);
    void setVisualZoomFactor(qreal factor);

    void rotate(bool left);

    void updateScrollBars();

    void animationFinished();

    void updateViewport();

    void addAxisAnimation(Qt::Axis axis, qreal endValue, int msecs);
    bool hasRunningAnimations();
    void stopAnimations();
    void syncPixmap();

public:
    QImage image;
    QPixmap originalPixmap;
    QPixmap pixmap;

    qreal zoomFactor;
    qreal visualZoomFactor;
    ZoomAnimation zoomAnimation;

    QList<AxisAnimation *> runningAnimations;

    QPoint prevPos;

private:
    QSingleImageView *q_ptr;
};

#endif // QSINGLEIMAGEVIEW_P_H
