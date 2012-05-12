#ifndef QSINGLEIMAGEVIEW_P_H
#define QSINGLEIMAGEVIEW_P_H

#include "qsingleimageview.h"

#include <QtCore/QVariantAnimation>
#include <QtGui/QUndoCommand>

// Animations
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

// Undo stack commands
class ImageViewCommand : public QUndoCommand
{
public:
    explicit ImageViewCommand(QSingleImageViewPrivate *dd);

protected:
    QSingleImageViewPrivate *d;
};

class RotateCommand : public ImageViewCommand
{
public:
    explicit RotateCommand(bool left, QSingleImageViewPrivate *dd);

    void redo();
    void undo();

private:
    bool m_left;
};

class HFlipCommand : public ImageViewCommand
{
public:
    explicit HFlipCommand(QSingleImageViewPrivate *dd);

    void redo();
    void undo();
};

class VFlipCommand : public ImageViewCommand
{
public:
    explicit VFlipCommand(QSingleImageViewPrivate *dd);

    void redo();
    void undo();
};

class CutCommand : public ImageViewCommand
{
public:
    explicit CutCommand(const QRect &rect, QSingleImageViewPrivate *dd);

    void redo();
    void undo();

private:
    QRect m_rect;
    QImage m_image;
};

class QSingleImageViewPrivate
{
    Q_DECLARE_PUBLIC(QSingleImageView)

public:
    explicit QSingleImageViewPrivate(QSingleImageView *qq);

    void setZoomFactor(qreal factor);
    void setVisualZoomFactor(qreal factor);

    void rotate(bool left);
    void flipHorizontally();
    void flipVertically();

    void updateScrollBars();

    void animationFinished();

    void updateViewport();

    void addAxisAnimation(Qt::Axis axis, qreal endValue, int msecs);
    bool hasRunningAnimations();
    void stopAnimations();
    void syncPixmap();

    QPointF getCenter() const;
    QRect selectedImageRect() const;

    void drawBackground(QPainter *p);
    void drawSelection(QPainter *p);

public:
    QImage image;
    QSingleImageView::MouseMode mouseMode;

    QPixmap originalPixmap;
    QPixmap pixmap;

    qreal zoomFactor;
    qreal visualZoomFactor;
    ZoomAnimation zoomAnimation;

    QList<AxisAnimation *> runningAnimations;

    bool mousePressed;
    QPoint startPos;
    QPoint pos;
    QPoint prevPos;

    QUndoStack *undoStack;

private:
    QSingleImageView *q_ptr;
};

#endif // QSINGLEIMAGEVIEW_P_H
