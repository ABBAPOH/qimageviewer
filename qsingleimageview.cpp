#include "qsingleimageview.h"
#include "qsingleimageview_p.h"

#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>
#include <QtGui/QScrollBar>

#include <QDebug>

#include "qimageviewsettings.h"

void QSingleImageViewPrivate::setZoomFactor(qreal factor)
{
    if (zoomFactor == factor)
        return;

    qDebug() << "setZoomFactor" << factor;
    if (factor < 0.01)
        factor = 0.01;

    zoomFactor = factor;

    pixmap = originalPixmap.scaled(originalPixmap.size()*zoomFactor);

    updateScrollBars();
}

void QSingleImageViewPrivate::updateScrollBars()
{
    Q_Q(QSingleImageView);

    QSize size = pixmap.size();
    int hmax = size.width() - q->viewport()->width();
    int vmax = size.height() - q->viewport()->height();

    double rh = q->horizontalScrollBar()->maximum() == 0 ?
                0.5 :
                1.0*q->horizontalScrollBar()->value() / q->horizontalScrollBar()->maximum();

    double rv = q->verticalScrollBar()->maximum() == 0 ?
                0.5 :
                1.0*q->verticalScrollBar()->value() / q->verticalScrollBar()->maximum();

    q->horizontalScrollBar()->setRange(0, hmax > 0 ? hmax : 0);
    q->horizontalScrollBar()->setValue(rh * q->horizontalScrollBar()->maximum());

    q->verticalScrollBar()->setRange(0, vmax > 0 ? vmax : 0);
    q->verticalScrollBar()->setValue(rv * q->verticalScrollBar()->maximum());
    q->viewport()->update();
}

QSingleImageView::QSingleImageView(QWidget *parent) :
    QAbstractScrollArea(parent),
    d_ptr(new QSingleImageViewPrivate(this))
{
//    setImage(QImage("/Users/arch/Pictures/anti112 .jpg"));
//    setImage(QImage("/Users/arch/Pictures/archon.jpg"));
    setImage(QImage("/Users/arch/Pictures/2048px-Smiley.svg.png"));

    horizontalScrollBar()->setSingleStep(10);
    verticalScrollBar()->setSingleStep(10);

    viewport()->setCursor(Qt::OpenHandCursor);
}

QSingleImageView::~QSingleImageView()
{
    delete d_ptr;
}

QImage QSingleImageView::image() const
{
    Q_D(const QSingleImageView);

    return d->image;
}

void QSingleImageView::setImage(const QImage &image)
{
    Q_D(QSingleImageView);

    d->image = image;
    d->pixmap = QPixmap::fromImage(image);
    d->originalPixmap = d->pixmap;

    d->updateScrollBars();
    bestFit();
}

void QSingleImageView::zoomIn()
{
    Q_D(QSingleImageView);

    d->setZoomFactor(d->zoomFactor*1.2);
}

void QSingleImageView::zoomOut()
{
    Q_D(QSingleImageView);

    d->setZoomFactor(d->zoomFactor*0.8);
}

void QSingleImageView::bestFit()
{
    Q_D(QSingleImageView);

    QSize imageSize = d->image.size();
    QSize size = viewport()->size();

    int w = imageSize.width(), mw = size.width();
    int h = imageSize.height(), mh = size.height();

    double rw = 1.0*mw/w, rh = 1.0*mh/h, factor = 1;

    if (rw < 1 && rh > 1)
        factor = rw;
    else if (rw > 1 && rh < 1)
        factor = rh;
    else if (rw < 1 && rh < 1)
        factor = qMin(rw, rh);
    else
        factor = qMin(rw, rh);

    d->setZoomFactor(factor);
}

void QSingleImageView::normalSize()
{
    Q_D(QSingleImageView);

    d->setZoomFactor(1.0);
}

void QSingleImageView::mousePressEvent(QMouseEvent *e)
{
    Q_D(QSingleImageView);

    d->prevPos = e->pos();

    viewport()->setCursor(Qt::ClosedHandCursor);
}

void QSingleImageView::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(QSingleImageView);

    QPoint pos = e->pos();

    int dx = d->prevPos.x() - pos.x();
    int dy = d->prevPos.y() - pos.y();

    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + dx);
    verticalScrollBar()->setValue(verticalScrollBar()->value() + dy);

    d->prevPos = pos;
}

void QSingleImageView::mouseReleaseEvent(QMouseEvent *)
{
    Q_D(QSingleImageView);

    d->prevPos = QPoint();

    viewport()->setCursor(Qt::OpenHandCursor);
}

static QPixmap chessBoardBackground()
{
    //cbsSize is the size of each square side
    static const int cbsSize = 16;

    QPixmap m = QPixmap(cbsSize*2,cbsSize*2);
    QPainter p(&m);
    p.fillRect(m.rect(), QColor(128,128,128));
    QColor light = QColor(192,192,192);
    p.fillRect(0,0,cbsSize,cbsSize,light);
    p.fillRect(cbsSize,cbsSize,cbsSize,cbsSize, light);
    p.end();
    return m;
}

void QSingleImageView::paintEvent(QPaintEvent *)
{
    Q_D(QSingleImageView);

    QImageViewSettings *settings = QImageViewSettings::globalSettings();

    QImageViewSettings::ImageBackgroundType type = settings->imageBackgroundType();
    QColor imageBackground = settings->imageBackgroundColor();
    QColor background = settings->backgroundColor();

    QPainter p(viewport());

    p.fillRect(viewport()->rect(), background);

    int hvalue = horizontalScrollBar()->value();
    int vvalue = verticalScrollBar()->value();

    if (horizontalScrollBar()->maximum() == 0)
        hvalue = -(viewport()->width() - d->pixmap.width())/2;

    if (verticalScrollBar()->maximum() == 0)
        vvalue = -(viewport()->height() - d->pixmap.height())/2;

    QRect r(QPoint(-hvalue, -vvalue), d->pixmap.size());

    switch (type) {
    case QImageViewSettings::None : break;
    case QImageViewSettings::Chess : p.drawTiledPixmap(r, ::chessBoardBackground()); break;
    case QImageViewSettings::SolidColor : p.fillRect(r, imageBackground); break;
    }

    p.drawPixmap(r, d->pixmap);
}

bool QSingleImageView::viewportEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Resize : {
        Q_D(QSingleImageView);

        QSize size = d->pixmap.size();
        verticalScrollBar()->setRange(0, size.height() - viewport()->height());
        horizontalScrollBar()->setRange(0, size.width() - viewport()->width());
        viewport()->update();
    }
    default:
        break;
    }

    return QAbstractScrollArea::viewportEvent(e);
}
