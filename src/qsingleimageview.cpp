#include "qsingleimageview.h"
#include "qsingleimageview_p.h"

#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>
#include <QtGui/QScrollBar>

#include "qimageviewsettings.h"
#include "qimageviewsettings_p.h"

static QPoint adjustPoint(QPoint p, qreal factor)
{
    return QPoint((int)(p.x()/factor)*factor, (int)(p.y()/factor)*factor);
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

static QPoint containingPoint(QPoint pos, const QRect &rect)
{
    pos.setX(qMax(pos.x(), rect.left()));
    pos.setY(qMax(pos.y(), rect.top()));
    pos.setX(qMin(pos.x(), rect.right()));
    pos.setY(qMin(pos.y(), rect.bottom()));
    return pos;
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

ImageViewCommand::ImageViewCommand(QSingleImageViewPrivate *dd) :
    QUndoCommand(),
    d(dd)
{
}

RotateCommand::RotateCommand(bool left, QSingleImageViewPrivate *dd) :
    ImageViewCommand(dd),
    m_left(left)
{
}

void RotateCommand::redo()
{
    d->rotate(m_left);
}

void RotateCommand::undo()
{
    d->rotate(!m_left);
}

HFlipCommand::HFlipCommand(QSingleImageViewPrivate *dd) :
    ImageViewCommand(dd)
{
}

void HFlipCommand::redo()
{
    d->flipHorizontally();
}

void HFlipCommand::undo()
{
    d->flipHorizontally();
}

VFlipCommand::VFlipCommand(QSingleImageViewPrivate *dd) :
    ImageViewCommand(dd)
{
}

void VFlipCommand::redo()
{
    d->flipVertically();
}

void VFlipCommand::undo()
{
    d->flipVertically();
}

CutCommand::CutCommand(const QRect &rect, QSingleImageViewPrivate *dd):
    ImageViewCommand(dd),
    m_rect(rect)
{
}

void CutCommand::redo()
{
    m_image = d->image.copy(m_rect);

    QColor color = QColor(255, 255, 255, d->image.hasAlphaChannel() ? 0 : 255);
    for (int x = 0; x < m_rect.width(); x++) {
        for (int y = 0; y < m_rect.height(); y++) {
            d->image.setPixel(x + m_rect.x(), y + m_rect.y(), color.rgba());
        }
    }

    d->syncPixmap();
}

void CutCommand::undo()
{
    for (int x = 0; x < m_rect.width(); x++) {
        for (int y = 0; y < m_rect.height(); y++) {
            QRgb color = m_image.pixel(x, y);
            d->image.setPixel(m_rect.x() + x, m_rect.y() + y, color);
        }
    }

    d->syncPixmap();
}

QSingleImageViewPrivate::QSingleImageViewPrivate(QSingleImageView *qq) :
    zoomFactor(1.0),
    visualZoomFactor(1.0),
    zoomAnimation(this),
    axisAnimationCount(0),
    mousePressed(false),
    canCopy(false),
    undoStack(new QUndoStack(qq)),
    undoStackIndex(0),
    modified(0),
    q_ptr(qq)
{
    Q_Q(QSingleImageView);

    QObject::connect(undoStack, SIGNAL(canRedoChanged(bool)), q, SIGNAL(canRedoChanged(bool)));
    QObject::connect(undoStack, SIGNAL(canUndoChanged(bool)), q, SIGNAL(canUndoChanged(bool)));

    QObject::connect(undoStack, SIGNAL(indexChanged(int)), q, SLOT(undoIndexChanged(int)));
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

    updateScrollBars();
}

void QSingleImageViewPrivate::setCanCopy(bool can)
{
    Q_Q(QSingleImageView);

    if (canCopy != can) {
        canCopy = can;
        emit q->canCopyChanged(canCopy);
    }
}

void QSingleImageViewPrivate::setModified(bool m)
{
    Q_Q(QSingleImageView);

    if (modified != m) {
        modified = m;
        emit q->modifiedChanged(modified);
    }
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

void QSingleImageViewPrivate::flipHorizontally()
{
    QTransform matrix;
    matrix.rotate(180, Qt::YAxis);
    image = image.transformed(matrix, Qt::SmoothTransformation);

    addAxisAnimation(Qt::YAxis, 180.0, 200);
}

void QSingleImageViewPrivate::flipVertically()
{
    QTransform matrix;
    matrix.rotate(180, Qt::XAxis);
    image = image.transformed(matrix, Qt::SmoothTransformation);

    addAxisAnimation(Qt::XAxis, 180.0, 200);
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

void QSingleImageViewPrivate::updateViewport()
{
    Q_Q(QSingleImageView);
    q->viewport()->update();
}

void QSingleImageViewPrivate::animationFinished()
{
    axisAnimationCount--;
    if (!axisAnimationCount)
        syncPixmap();
}

void QSingleImageViewPrivate::undoIndexChanged(int index)
{
    if (index == undoStackIndex)
        setModified(false);
    else
        setModified(true);
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
    axisAnimationCount++;
    QObject::connect(animation, SIGNAL(finished()), q, SLOT(animationFinished()));
}

bool QSingleImageViewPrivate::hasRunningAnimations() const
{
    return axisAnimationCount || (zoomAnimation.state() == QVariantAnimation::Running);
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

QRect QSingleImageViewPrivate::selectedImageRect() const
{
    if (startPos == pos)
        return QRect();

    QPointF center = getCenter();

    QRectF selectionRect(startPos, pos);
    selectionRect = selectionRect.normalized();
    selectionRect.translate(-center.x(), -center.y());

    qreal factor = visualZoomFactor;
    selectionRect = QRectF(selectionRect.topLeft()/factor, selectionRect.bottomRight()/factor);

    QRect pixmapRect(QPoint(0, 0), pixmap.size());
    pixmapRect.translate(-pixmapRect.center());

    QRect result = QRectF(selectionRect.intersect(pixmapRect)).toAlignedRect();
    result.translate(pixmap.width()/2, pixmap.height()/2);
    return result;
}

void QSingleImageViewPrivate::drawBackground(QPainter *p)
{
    QImageViewSettings *settings = QImageViewSettings::globalSettings();

    QImageViewSettings::ImageBackgroundType type = settings->imageBackgroundType();
    QColor imageBackgroundColor = settings->imageBackgroundColor();

    QSize size = pixmap.size()*visualZoomFactor;
    QRectF rect(QPointF(0, 0), size);
    rect.translate(-rect.center());

    switch (type) {
    case QImageViewSettings::None :
        break;
    case QImageViewSettings::Chess :
        p->drawPixmap(rect, chessBoardBackground(size), QRectF(QPointF(0, 0), size));
        break;
    case QImageViewSettings::SolidColor :
        p->fillRect(rect, imageBackgroundColor);
        break;
    }
}

void QSingleImageViewPrivate::drawSelection(QPainter *p)
{
    Q_Q(QSingleImageView);

    if (mouseMode != QSingleImageView::MouseModeSelect)
        return;

    if (startPos == pos)
        return;

    QPointF center = getCenter();

    QRect rect = q->viewport()->rect();
    rect.translate(-center.x(), -center.y());

    qreal factor = visualZoomFactor;
    QSize backgroundSize = pixmap.size()*factor;
    QRect imageRect(QPoint(0, 0), backgroundSize);
    imageRect.translate(-imageRect.center());

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

QSingleImageView::QSingleImageView(QWidget *parent) :
    QAbstractScrollArea(parent),
    d_ptr(new QSingleImageViewPrivate(this))
{
    setImage(QImage("/Users/arch/Pictures/2048px-Smiley.svg.png"));

    horizontalScrollBar()->setSingleStep(10);
    verticalScrollBar()->setSingleStep(10);

    setMouseMode(MouseModeMove);

    QImageViewSettings::globalSettings()->d_func()->addView(this);
}

QSingleImageView::~QSingleImageView()
{
    QImageViewSettings::globalSettings()->d_func()->removeView(this);
    delete d_ptr;
}

bool QSingleImageView::canCopy() const
{
    Q_D(const QSingleImageView);

    return d->canCopy;
}

bool QSingleImageView::canRedo() const
{
    Q_D(const QSingleImageView);

    return d->undoStack->canRedo();
}

bool QSingleImageView::canUndo() const
{
    Q_D(const QSingleImageView);

    return d->undoStack->canUndo();
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

    if (d->image.isNull()) {
        d->zoomFactor = 1.0;
        d->visualZoomFactor = 1.0;
        d->updateScrollBars();
        return;
    }

    bestFit();
    viewport()->update();
}

bool QSingleImageView::isModified() const
{
    Q_D(const QSingleImageView);

    return d->undoStackIndex == d->undoStack->index();
}

void QSingleImageView::setModified(bool modified)
{
    Q_D(QSingleImageView);

    if (modified)
        d->undoStackIndex = 0;
    else
        d->undoStackIndex = d->undoStack->index();

    d->setModified(modified);
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

        clearSelection();

        d->mouseMode = mode;
        emit mouseModeChanged(mode);
    }
}

QRect QSingleImageView::selectedImageRect() const
{
    Q_D(const QSingleImageView);

    return d->selectedImageRect();
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

    d->undoStack->push(new RotateCommand(true, d));
}

void QSingleImageView::rotateRight()
{
    Q_D(QSingleImageView);

    d->undoStack->push(new RotateCommand(false, d));
}

void QSingleImageView::flipHorizontally()
{
    Q_D(QSingleImageView);

    d->undoStack->push(new HFlipCommand(d));
}

void QSingleImageView::flipVertically()
{
    Q_D(QSingleImageView);

    d->undoStack->push(new VFlipCommand(d));
}

void QSingleImageView::clearSelection()
{
    Q_D(QSingleImageView);

    d->startPos = d->pos = QPoint();
    d->setCanCopy(false);
    viewport()->update();
}

void QSingleImageView::copy()
{
    QImage image = selectedImage();

    QClipboard *clipboard = qApp->clipboard();
    clipboard->clear();
    clipboard->setImage(image);
}

void QSingleImageView::cut()
{
    Q_D(QSingleImageView);

    copy();

    d->undoStack->push(new CutCommand(selectedImageRect(), d));
}

void QSingleImageView::redo()
{
    Q_D(QSingleImageView);

    d->undoStack->redo();
}

void QSingleImageView::undo()
{
    Q_D(QSingleImageView);

    d->undoStack->undo();
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
    else
        d->setCanCopy(false);

    viewport()->update();
}

void QSingleImageView::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(QSingleImageView);

    QPoint pos = e->pos();

    int dx = d->prevPos.x() - pos.x();
    int dy = d->prevPos.y() - pos.y();

    d->pos = pos;
    d->prevPos = pos;

    if (d->mouseMode == MouseModeMove) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + dx);
        verticalScrollBar()->setValue(verticalScrollBar()->value() + dy);
    } else {
        d->setCanCopy(d->pos != d->startPos);
    }

    viewport()->update();
}

void QSingleImageView::mouseReleaseEvent(QMouseEvent *)
{
    Q_D(QSingleImageView);

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
    case Qt::Key_X :
        if (e->modifiers() & Qt::ControlModifier)
            cut();
    default:
        break;
    }

    QAbstractScrollArea::keyPressEvent(e);
}

void QSingleImageView::paintEvent(QPaintEvent *)
{
    Q_D(QSingleImageView);

    QPainter p(viewport());
    if (!d->hasRunningAnimations())
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QRect rect = viewport()->rect();

    // Draw viewport background
    QColor backgroundColor = QImageViewSettings::globalSettings()->backgroundColor();
    p.fillRect(rect, backgroundColor);

    if (d->image.isNull())
        return;

    // Move and rotate painter
    QPointF center = d->getCenter();

    QTransform matrix;
    matrix.translate(center.x(), center.y());

    for (int i = d->runningAnimations.count() - 1; i >= 0; i--) {
        AxisAnimation *animation = d->runningAnimations.at(i);
        matrix.rotate(animation->angle(), animation->axis());
    }

    p.setTransform(matrix);

    // Draw image background
    d->drawBackground(&p);

    // Draw scaled pixmap
    p.save();
    p.scale(d->visualZoomFactor, d->visualZoomFactor);

    QRectF pixmapRect(QPointF(0, 0), d->pixmap.size());
    pixmapRect.translate(-pixmapRect.center());
    p.drawPixmap(pixmapRect, d->pixmap, QRectF(QPointF(0, 0), d->pixmap.size()));

    p.restore();

    // Draw vieport's and pixmap's selections
    d->drawSelection(&p);
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
