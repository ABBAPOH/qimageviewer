#include "qsingleimageview.h"
#include "qsingleimageview_p.h"

#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>
#include <QtGui/QScrollBar>

#include <QDebug>

#include "qimageviewsettings.h"

ZoomAnimation::ZoomAnimation(QSingleImageViewPrivate *dd, QObject *parent) :
    QVariantAnimation(parent),
    d(dd)
{
}

void ZoomAnimation::updateCurrentValue(const QVariant &value)
{
    d->setVisualZoomFactor(value.toReal());
}

QImageViewerRealAnimation::QImageViewerRealAnimation(Func f, QSingleImageViewPrivate *dd, QObject *parent) :
    QVariantAnimation(parent),
    d(dd),
    func(f)
{
}

void QImageViewerRealAnimation::updateCurrentValue(const QVariant &value)
{
    if (state() == Running)
        (d->*func)(value.toReal());
}

QSingleImageViewPrivate::QSingleImageViewPrivate(QSingleImageView *qq) :
    zoomFactor(1.0),
    visualZoomFactor(1.0),
    zoomAnimation(this),
    rotationAngle(0),
    rotateAnimation(&QSingleImageViewPrivate::setRotationAngle, this),
    hFlipAngle(0),
    hFlipAnimation(&QSingleImageViewPrivate::setHFlipAngle, this),
    vFlipAngle(0),
    vFlipAnimation(&QSingleImageViewPrivate::setVFlipAngle, this),
    q_ptr(qq)
{
}

void QSingleImageViewPrivate::setZoomFactor(qreal factor)
{
    if (zoomFactor == factor)
        return;

    if (image.isNull())
        return;

    if (factor < 0.01)
        factor = 0.01;

    zoomFactor = factor;
//    visualZoomFactor = factor;

    if (zoomAnimation.state() == QVariantAnimation::Running)
        zoomAnimation.stop();

    zoomAnimation.setStartValue(visualZoomFactor);
    zoomAnimation.setEndValue(zoomFactor);
    zoomAnimation.setDuration(75);
    zoomAnimation.setEasingCurve(QEasingCurve::Linear);
    zoomAnimation.start();
}

void QSingleImageViewPrivate::setVisualZoomFactor(qreal factor)
{
    visualZoomFactor = factor;

    Q_Q(QSingleImageView);
    q->viewport()->update();
    updateScrollBars();
}

void QSingleImageViewPrivate::updateScrollBars()
{
    Q_Q(QSingleImageView);

    QSize size = pixmap.size() * visualZoomFactor;
    int hmax = size.width() - q->viewport()->width();
    int vmax = size.height() - q->viewport()->height();

    double rh = q->horizontalScrollBar()->maximum() == 0 ?
                0.5 :
                1.0*q->horizontalScrollBar()->value() / q->horizontalScrollBar()->maximum();

    double rv = q->verticalScrollBar()->maximum() == 0 ?
                0.5 :
                1.0*q->verticalScrollBar()->value() / q->verticalScrollBar()->maximum();

    q->horizontalScrollBar()->setRange(0, hmax > 0 ? hmax : 0);
    q->horizontalScrollBar()->setValue(rh * q->horizontalScrollBar()->maximum() + 0.5);

    q->verticalScrollBar()->setRange(0, vmax > 0 ? vmax : 0);
    q->verticalScrollBar()->setValue(rv * q->verticalScrollBar()->maximum() + 0.5);

    q->viewport()->update();
}

void QSingleImageViewPrivate::animationFinished()
{
    if (!hasRunningAnimations())
        syncPixmap();
}

void QSingleImageViewPrivate::stopAnimations()
{
    rotateAnimation.stop();
    vFlipAnimation.stop();
    hFlipAnimation.stop();
}

void QSingleImageViewPrivate::syncPixmap()
{
    // reset rotate animation
    rotationAngle = 0.0;
    rotateAnimation.setStartValue(0);
    rotateAnimation.setEndValue(0);

    hFlipAngle = 0.0;
    hFlipAnimation.setStartValue(0);
    hFlipAnimation.setEndValue(0);

    vFlipAngle = 0.0;
    vFlipAnimation.setStartValue(0);
    vFlipAnimation.setEndValue(0);

    pixmap = QPixmap::fromImage(image);

    updateViewport();
}

void QSingleImageViewPrivate::updateViewport()
{
    Q_Q(QSingleImageView);
    q->viewport()->update();
}

bool QSingleImageViewPrivate::hasRunningAnimations()
{
    if (rotateAnimation.state() == QVariantAnimation::Running)
        return true;

    if (hFlipAnimation.state() == QVariantAnimation::Running)
        return true;

    if (vFlipAnimation.state() == QVariantAnimation::Running)
        return true;

    return false;
}

void QSingleImageViewPrivate::rotate(bool left)
{
    Q_Q(QSingleImageView);

    QTransform matrix;
    matrix.rotate(left ? -90 : 90, Qt::ZAxis);
    image = this->image.transformed(matrix, Qt::SmoothTransformation);
    q->viewport()->update();

    if (rotateAnimation.state() == QVariantAnimation::Running)
        rotateAnimation.stop();

    rotateAnimation.setStartValue(rotationAngle);
    rotateAnimation.setEndValue(rotateAnimation.endValue().toReal() + (left ? - 90 : 90));
    rotateAnimation.setEasingCurve(QEasingCurve::Linear);
    rotateAnimation.setDuration(200);
    rotateAnimation.start();
}

void QSingleImageViewPrivate::setRotationAngle(qreal angle)
{
    rotationAngle = angle;

    updateViewport();
}

void QSingleImageViewPrivate::setHFlipAngle(qreal angle)
{
    hFlipAngle = angle;

    updateViewport();
}

void QSingleImageViewPrivate::setVFlipAngle(qreal angle)
{
    vFlipAngle = angle;

    updateViewport();
}

QSingleImageView::QSingleImageView(QWidget *parent) :
    QAbstractScrollArea(parent),
    d_ptr(new QSingleImageViewPrivate(this))
{
    Q_D(QSingleImageView);
//    setImage(QImage("/Users/arch/Pictures/anti112 .jpg"));
//    setImage(QImage("/Users/arch/Pictures/archon.jpg"));
    setImage(QImage("/Users/arch/Pictures/2048px-Smiley.svg.png"));

    horizontalScrollBar()->setSingleStep(10);
    verticalScrollBar()->setSingleStep(10);

    viewport()->setCursor(Qt::OpenHandCursor);

    connect(&d->rotateAnimation, SIGNAL(finished()), this, SLOT(animationFinished()));
    connect(&d->hFlipAnimation, SIGNAL(finished()), this, SLOT(animationFinished()));
    connect(&d->vFlipAnimation, SIGNAL(finished()), this, SLOT(animationFinished()));
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

    d->stopAnimations();
    d->syncPixmap();

//    d->pixmap = QPixmap::fromImage(d->image);

    if (d->image.isNull()) {
        d->zoomFactor = 1.0;
        d->visualZoomFactor = 1.0;
        d->updateScrollBars();
        return;
    }

    bestFit();
    viewport()->update();
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

    if (d->image.isNull())
        return;

    QSize imageSize = d->image.size();
    QSize size = this->size();
    size.rwidth() -= verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
    size.rheight() -= verticalScrollBar()->isVisible() ? horizontalScrollBar()->height() : 0;

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

void QSingleImageView::rotateLeft()
{
    Q_D(QSingleImageView);

    d->rotate(true);
}

void QSingleImageView::rotateRight()
{
    Q_D(QSingleImageView);

    d->rotate(false);
}

void QSingleImageView::flipHorizontally()
{
    Q_D(QSingleImageView);

    QTransform matrix;
    matrix.rotate(180, Qt::XAxis);
    d->image = d->image.transformed(matrix, Qt::SmoothTransformation);
//    d->syncPixmap();
//    return;

    if (d->hFlipAnimation.state() == QVariantAnimation::Running)
        d->hFlipAnimation.stop();

    d->hFlipAnimation.setStartValue(d->hFlipAngle);
    d->hFlipAnimation.setEndValue(d->hFlipAnimation.endValue().toReal() + 180);
    d->hFlipAnimation.setEasingCurve(QEasingCurve::Linear);
    d->hFlipAnimation.setDuration(200);
    d->hFlipAnimation.start();
}

void QSingleImageView::flipVertically()
{
    Q_D(QSingleImageView);

    QTransform matrix;
    matrix.rotate(180, Qt::YAxis);
    d->image = d->image.transformed(matrix, Qt::SmoothTransformation);

    if (d->vFlipAnimation.state() == QVariantAnimation::Running)
        d->vFlipAnimation.stop();

    d->vFlipAnimation.setStartValue(d->vFlipAngle);
    d->vFlipAnimation.setEndValue(d->vFlipAnimation.endValue().toReal() + 180);
    d->vFlipAnimation.setEasingCurve(QEasingCurve::Linear);
    d->vFlipAnimation.setDuration(200);
    d->vFlipAnimation.start();
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

static QPixmap chessBoardBackground(const QRect &rect)
{
    int max = qMax(rect.width(), rect.height());
    QRect r(0, 0, max, max);
    QPixmap m(r.size());
    QPainter p(&m);
    p.drawTiledPixmap(r, ::chessBoardBackground());
    return m;
}

void QSingleImageView::paintEvent(QPaintEvent *)
{
    Q_D(QSingleImageView);

    QImageViewSettings *settings = QImageViewSettings::globalSettings();

    QImageViewSettings::ImageBackgroundType type = settings->imageBackgroundType();
    QColor imageBackgroundColor = settings->imageBackgroundColor();
    QColor backgroundColor = settings->backgroundColor();

    QPainter p(viewport());
    QRect rect = viewport()->rect();

    p.fillRect(rect, backgroundColor);

    if (d->image.isNull())
        return;

    qreal factor = d->visualZoomFactor;

    int hvalue = horizontalScrollBar()->value();
    int vvalue = verticalScrollBar()->value();

    if (horizontalScrollBar()->maximum() == 0)
        hvalue = -(viewport()->width() - factor*d->pixmap.width())/2;

    if (verticalScrollBar()->maximum() == 0)
        vvalue = -(viewport()->height() - factor*d->pixmap.height())/2;

    QPoint center = QPoint(factor*d->pixmap.width()/2 - hvalue, factor*d->pixmap.height()/2 - vvalue);

    QTransform matrix;
    matrix.translate(center.x(), center.y());

    matrix.rotate(d->hFlipAngle, Qt::XAxis);
    matrix.rotate(d->vFlipAngle, Qt::YAxis);
    matrix.rotate(d->rotationAngle, Qt::ZAxis);

    matrix.translate(-center.x(), -center.y());
    p.setTransform(matrix);

    QRect pixmapRect(QPoint(0,0), d->pixmap.size());

    p.translate(-hvalue, -vvalue);
    p.scale(factor, factor);

    switch (type) {
    case QImageViewSettings::None : break;
    case QImageViewSettings::Chess : p.drawPixmap(pixmapRect, chessBoardBackground(rect)); break;
    case QImageViewSettings::SolidColor : p.fillRect(pixmapRect, imageBackgroundColor); break;
    }

    p.drawPixmap(pixmapRect, d->pixmap);
}

bool QSingleImageView::viewportEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Resize : {
        Q_D(QSingleImageView);

        d->updateScrollBars();
    }
    default:
        break;
    }

    return QAbstractScrollArea::viewportEvent(e);
}

#include "moc_qsingleimageview.cpp"
