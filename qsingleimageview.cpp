#include "qsingleimageview.h"
#include "qsingleimageview_p.h"

#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>
#include <QtGui/QScrollBar>

#include <QDebug>

#include "qimageviewsettings.h"

QPoint adjustPoint(QPoint p, qreal factor)
{
//    QPointF pf = p;
    return QPoint((int)(p.x()/factor)*factor, (int)(p.y()/factor)*factor);
}

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

AxisAnimation::AxisAnimation(Qt::Axis axis, QSingleImageViewPrivate *dd, QObject *parent):
    QVariantAnimation(parent),
    d(dd),
    m_axis(axis)
{
}

void AxisAnimation::updateCurrentValue(const QVariant &/*value*/)
{
    d->updateViewport();
}

QSingleImageViewPrivate::QSingleImageViewPrivate(QSingleImageView *qq) :
    zoomFactor(1.0),
    visualZoomFactor(1.0),
    zoomAnimation(this),
    mousePressed(false),
    q_ptr(qq)
{
}

void QSingleImageViewPrivate::setZoomFactor(qreal factor)
{
    Q_Q(QSingleImageView);

    if (zoomFactor == factor)
        return;

    if (image.isNull())
        return;

    if (factor < 0.01)
        factor = 0.01;

    q->clearSelection();

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

void QSingleImageViewPrivate::updateViewport()
{
    Q_Q(QSingleImageView);
    q->viewport()->update();
}

void QSingleImageViewPrivate::addAxisAnimation(Qt::Axis axis, qreal endValue, int msecs)
{
    Q_Q(QSingleImageView);

    AxisAnimation *animation = new AxisAnimation(axis, this, q);
    animation->setStartValue(0.0);
    animation->setEndValue(endValue);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->setDuration(msecs);
    animation->start();
    runningAnimations.append(animation);
    QObject::connect(animation, SIGNAL(finished()), q, SLOT(animationFinished()));
}

bool QSingleImageViewPrivate::hasRunningAnimations()
{
    foreach (AxisAnimation *animation, runningAnimations) {
        if (animation->state() == QVariantAnimation::Running)
            return true;
    }

    return false;
}

void QSingleImageViewPrivate::stopAnimations()
{
    foreach (AxisAnimation *animation, runningAnimations) {
        animation->stop();
    }
}

void QSingleImageViewPrivate::syncPixmap()
{
    pixmap = QPixmap::fromImage(image);

    qDeleteAll(runningAnimations);
    runningAnimations.clear();

    updateViewport();
}

QPointF QSingleImageViewPrivate::getCenter() const
{
    Q_Q(const QSingleImageView);

    qreal factor = visualZoomFactor;

    int hvalue = q->horizontalScrollBar()->value();
    int vvalue = q->verticalScrollBar()->value();

    if (q->horizontalScrollBar()->maximum() == 0)
        hvalue = -(q->viewport()->width() - factor*pixmap.width())/2;

    if (q->verticalScrollBar()->maximum() == 0)
        vvalue = -(q->viewport()->height() - factor*pixmap.height())/2;

    return QPointF(factor*pixmap.width()/2.0 - hvalue, factor*pixmap.height()/2.0 - vvalue);
}

static QPoint containingPoint(QPoint pos, const QRect &rect)
{
    pos.setX(qMax(pos.x(), rect.left()));
    pos.setY(qMax(pos.y(), rect.top()));
    pos.setX(qMin(pos.x(), rect.right()));
    pos.setY(qMin(pos.y(), rect.bottom()));
    return pos;
}

void QSingleImageViewPrivate::drawSelection(QPainter *p, const QRect &rect, const QRect &imageRect)
{
    if (mouseMode != QSingleImageView::MouseModeSelect)
        return;

    QPointF center = getCenter();

    // rect in painter's coordinates
    QRect selectionRect(::adjustPoint(startPos, zoomFactor), ::adjustPoint(pos, zoomFactor));
    selectionRect.translate(-center.x(), -center.y());
    selectionRect = selectionRect.intersect(rect);

    if (selectionRect.isNull())
        return;

    p->setPen(QPen(Qt::lightGray, 1, Qt::DashLine, Qt::RoundCap));
    p->drawRect(selectionRect);

    QRect imageSelectionRect = imageRect.intersect(selectionRect);

    p->setPen(QPen(Qt::black, 1, Qt::DashLine, Qt::RoundCap));
    p->drawRect(imageSelectionRect);

    QString text = QSingleImageView::tr("%1 x %2").
            arg(abs(imageSelectionRect.width()/visualZoomFactor)).
            arg(abs(imageSelectionRect.height()/visualZoomFactor));

    int textWidth = p->fontMetrics().width(text);
    int textHeight = p->fontMetrics().height();

    QPoint textPos = pos + rect.topLeft();
    textPos = containingPoint(textPos, rect);

    textPos.setX(qMax(textPos.x(), rect.left()));
    textPos.setY(qMax(textPos.y(), rect.top() + textHeight));
    textPos.setX(qMin(textPos.x(), rect.right() - textWidth));
    textPos.setY(qMin(textPos.y(), rect.bottom() - textHeight));

    p->setPen(Qt::black);
    p->drawText(textPos, text);
}

void QSingleImageViewPrivate::rotate(bool left)
{
    Q_Q(QSingleImageView);

    QTransform matrix;
    matrix.rotate(left ? -90 : 90, Qt::ZAxis);
    image = this->image.transformed(matrix, Qt::SmoothTransformation);
    q->viewport()->update();

    addAxisAnimation(Qt::ZAxis, left ? - 90.0 : 90.0, 150);
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

    setMouseMode(MouseModeSelect);
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

QSingleImageView::MouseMode QSingleImageView::mouseMode() const
{
    Q_D(const QSingleImageView);

    return d->mouseMode;
}

void QSingleImageView::setMouseMode(QSingleImageView::MouseMode mode)
{
    Q_D(QSingleImageView);

    if (d->mouseMode != mode) {
        if (mode == MouseModeMove)
            viewport()->setCursor(Qt::OpenHandCursor);
        else
            viewport()->setCursor(Qt::ArrowCursor);

        d->mouseMode = mode;
        emit mouseModeChanged(mode);
    }
}

QRect QSingleImageView::selectedImageRect() const
{
    Q_D(const QSingleImageView);

    if (d->startPos == d->pos)
        return QRect();

    QPointF center = d->getCenter();

    QRectF selectionRect(d->startPos, d->pos);
    selectionRect = selectionRect.normalized();
    selectionRect.translate(-center.x(), -center.y());

    qreal factor = d->visualZoomFactor;
    selectionRect = QRectF(selectionRect.topLeft()/factor, selectionRect.bottomRight()/factor);

    QRect pixmapRect(QPoint(0, 0), d->pixmap.size());
    pixmapRect.translate(-pixmapRect.center());

    QRect result = QRectF(selectionRect.intersect(pixmapRect)).toAlignedRect();
    result.translate(d->pixmap.width()/2, d->pixmap.height()/2);
    return result;
}

QImage QSingleImageView::selectedImage() const
{
    Q_D(const QSingleImageView);

    return d->image.copy(selectedImageRect());
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

    d->addAxisAnimation(Qt::XAxis, 180.0, 200);
}

void QSingleImageView::flipVertically()
{
    Q_D(QSingleImageView);

    QTransform matrix;
    matrix.rotate(180, Qt::YAxis);
    d->image = d->image.transformed(matrix, Qt::SmoothTransformation);

    d->addAxisAnimation(Qt::YAxis, 180.0, 200);
}

void QSingleImageView::clearSelection()
{
    Q_D(QSingleImageView);

    d->startPos = d->pos = QPoint();
    viewport()->update();
}

void QSingleImageView::copy()
{
    QImage image = selectedImage();

    QClipboard *clipboard = qApp->clipboard();
    clipboard->clear();
    clipboard->setImage(image);
}

void QSingleImageView::mousePressEvent(QMouseEvent *e)
{
    Q_D(QSingleImageView);

    d->mousePressed = true;
    d->startPos = e->pos();
    d->pos = e->pos();
    d->prevPos = e->pos();

    if (d->mouseMode == MouseModeMove)
        viewport()->setCursor(Qt::ClosedHandCursor);

    viewport()->update();
}

void QSingleImageView::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(QSingleImageView);

    QPoint pos = e->pos();

    int dx = d->prevPos.x() - pos.x();
    int dy = d->prevPos.y() - pos.y();

    if (d->mouseMode == MouseModeMove) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + dx);
        verticalScrollBar()->setValue(verticalScrollBar()->value() + dy);
    }

    d->pos = pos;
    d->prevPos = pos;

    viewport()->update();
}

void QSingleImageView::mouseReleaseEvent(QMouseEvent *)
{
    Q_D(QSingleImageView);

//    d->startPos = QPoint();
//    d->pos = QPoint();
    d->prevPos = QPoint();

    if (d->mouseMode == MouseModeMove)
        viewport()->setCursor(Qt::OpenHandCursor);
    d->mousePressed = false;

    viewport()->update();
}

void QSingleImageView::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Escape :
        clearSelection();
        break;
    case Qt::Key_C :
        if (e->modifiers() & Qt::ControlModifier)
            copy();
    default:
        break;
    }

    QAbstractScrollArea::keyPressEvent(e);
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

static QPixmap chessBoardBackground(const QSize &size)
{
    if (size.isEmpty())
        return QPixmap();

    static QSize previousSize;
    static QPixmap cachedPismap;

    if (size == previousSize) {
        return cachedPismap;
    }

    int w = size.width(), h = size.height();

    QPixmap m(w, h);
    QPainter p(&m);
    p.translate(w/2.0, h/2.0);
    p.drawTiledPixmap(QRect(-8, -8, w/2 + 8, h/2 + 8), ::chessBoardBackground());
    p.rotate(90);
    p.drawTiledPixmap(QRect(-8, -8, w/2 + 8, h/2 + 8), ::chessBoardBackground());
    p.rotate(90);
    p.drawTiledPixmap(QRect(-8, -8, w/2 + 8, h/2 + 8), ::chessBoardBackground());
    p.rotate(90);
    p.drawTiledPixmap(QRect(-8, -8, w/2 + 8, h/2 + 8), ::chessBoardBackground());
    p.end();

    previousSize = size;
    cachedPismap = m;

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

    QPointF center = d->getCenter();
    p.setPen(Qt::red);
    p.drawEllipse(center, 500, 500);

    QTransform matrix;
    matrix.translate(center.x(), center.y());

    for (int i = d->runningAnimations.count() - 1; i >= 0; i--) {
        AxisAnimation *animation = d->runningAnimations.at(i);
        matrix.rotate(animation->angle(), animation->axis());
    }

    p.setTransform(matrix);

    qreal factor = d->visualZoomFactor;
    QSize backgroundSize = d->pixmap.size()*factor;
    QRectF backgroundRect(QPoint(0, 0), backgroundSize);
    backgroundRect.translate(-backgroundRect.center());

    switch (type) {
    case QImageViewSettings::None :
        break;
    case QImageViewSettings::Chess :
        p.drawPixmap(backgroundRect, chessBoardBackground(backgroundSize), QRectF(QPointF(0, 0), backgroundSize));
        break;
    case QImageViewSettings::SolidColor :
        p.fillRect(backgroundRect, imageBackgroundColor);
        break;
    }

    p.save();
    p.scale(factor, factor);

    QRectF pixmapRect(QPointF(0, 0), d->pixmap.size());
    pixmapRect.translate(-pixmapRect.center());
    p.drawPixmap(pixmapRect, d->pixmap, QRectF(QPointF(0, 0), d->pixmap.size()));

    p.restore();

    rect.translate(-center.x(), -center.y());

    if (d->startPos == d->pos)
        return;

    d->drawSelection(&p, rect, backgroundRect.toAlignedRect());
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
