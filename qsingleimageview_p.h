#ifndef QSINGLEIMAGEVIEW_P_H
#define QSINGLEIMAGEVIEW_P_H

#include "qsingleimageview.h"

class QSingleImageViewPrivate
{
    Q_DECLARE_PUBLIC(QSingleImageView)

public:
    explicit QSingleImageViewPrivate(QSingleImageView *qq) :
        zoomFactor(1.0),
        q_ptr(qq)
    {}

    void setZoomFactor(qreal factor);
    void updateScrollBars();

public:
    QImage image;
    QPixmap originalPixmap;
    QPixmap pixmap;

    qreal zoomFactor;

    QPoint prevPos;

private:
    QSingleImageView *q_ptr;
};

#endif // QSINGLEIMAGEVIEW_P_H
