#include "qimageview.h"
#include "qimageview_p.h"

#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QListWidget>
#include <QtGui/QImageReader>
#include <QtGui/QImageWriter>
#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>
#include <QtGui/QScrollBar>

#include <QtOpenGL/QGLWidget>

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
    static QPixmap cachedPixmap;

    if (size == previousSize) {
        return cachedPixmap;
    }

    int w = size.width(), h = size.height();

    QPixmap background = ::chessBoardBackground();
    QPixmap m(w, h);
    QPainter p(&m);
    p.translate(w/2.0, h/2.0);
    p.drawTiledPixmap(QRect(-8, -8, w/2 + 8, h/2 + 8), background);
    p.rotate(90);
    p.drawTiledPixmap(QRect(-8, -8, w/2 + 8, h/2 + 8), background);
    p.rotate(90);
    p.drawTiledPixmap(QRect(-8, -8, w/2 + 8, h/2 + 8), background);
    p.rotate(90);
    p.drawTiledPixmap(QRect(-8, -8, w/2 + 8, h/2 + 8), background);
    p.end();

    previousSize = size;
    cachedPixmap = m;

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

ZoomAnimation::ZoomAnimation(QImageViewPrivate *dd, QObject *parent) :
    QVariantAnimation(parent),
    d(dd)
{
}

void ZoomAnimation::updateCurrentValue(const QVariant &value)
{
    d->setVisualZoomFactor(value.toReal());
}

AxisAnimation::AxisAnimation(Qt::Axis axis, QImageViewPrivate *dd, QObject *parent):
    QVariantAnimation(parent),
    d(dd),
    m_axis(axis)
{
}

void AxisAnimation::updateCurrentValue(const QVariant &/*value*/)
{
    d->updateViewport();
}

ImageViewCommand::ImageViewCommand(QImageViewPrivate *dd) :
    QUndoCommand(),
    d(dd)
{
}

RotateCommand::RotateCommand(bool left, QImageViewPrivate *dd) :
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

HFlipCommand::HFlipCommand(QImageViewPrivate *dd) :
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

VFlipCommand::VFlipCommand(QImageViewPrivate *dd) :
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

CutCommand::CutCommand(const QRect &rect, QImageViewPrivate *dd) :
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

ResizeCommand::ResizeCommand(const QSize &size, QImageViewPrivate *dd) :
    ImageViewCommand(dd),
    m_size(size)
{
}

void ResizeCommand::redo()
{
    m_image = d->image;
    d->image = d->image.scaled(m_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    d->syncPixmap();
}

void ResizeCommand::undo()
{
    d->image = m_image;
    d->syncPixmap();
}

QImageViewPrivate::QImageViewPrivate(QImageView *qq) :
    currentImageNumber(-1),
    zoomFactor(1.0),
    visualZoomFactor(1.0),
    zoomAnimation(this),
    axisAnimationCount(0),
    mousePressed(false),
    canCopy(false),
    undoStack(new QUndoStack(qq)),
    undoStackIndex(0),
    modified(0),
    listWidget(new QListWidget(qq)),
    thumbnailsPosition(QImageView::East),
    q_ptr(qq)
{
    Q_Q(QImageView);

    listWidget->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    QPalette palette = listWidget->palette();
    palette.setColor(QPalette::Base, QColor(128, 128, 128));
    listWidget->setPalette(palette);
    listWidget->setGridSize(QSize(100, 100));
    listWidget->setIconSize(QSize(64, 64));
    listWidget->setFlow(QListView::LeftToRight);
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setFocusPolicy(Qt::NoFocus);
    QObject::connect(listWidget, SIGNAL(currentRowChanged(int)), q, SLOT(jumpToImage(int)));

    QObject::connect(undoStack, SIGNAL(canRedoChanged(bool)), q, SIGNAL(canRedoChanged(bool)));
    QObject::connect(undoStack, SIGNAL(canUndoChanged(bool)), q, SIGNAL(canUndoChanged(bool)));

    QObject::connect(undoStack, SIGNAL(indexChanged(int)), q, SLOT(undoIndexChanged(int)));
}

void QImageViewPrivate::recreateViewport(bool useOpenGL)
{
    Q_Q(QImageView);

    if (useOpenGL) {
        QGLFormat glFormat(QGL::SampleBuffers); // antialiasing
//        glFormat.setSwapInterval(1); // vsync
        q->setViewport(new QGLWidget(glFormat, q));
    } else {
        q->setViewport(new QWidget);
    }
}

void QImageViewPrivate::setZoomFactor(qreal factor)
{
    Q_Q(QImageView);

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

void QImageViewPrivate::setVisualZoomFactor(qreal factor)
{
    visualZoomFactor = factor;

    updateScrollBars();
}

void QImageViewPrivate::setCanCopy(bool can)
{
    Q_Q(QImageView);

    if (canCopy != can) {
        canCopy = can;
        emit q->canCopyChanged(canCopy);
    }
}

void QImageViewPrivate::setModified(bool m)
{
    Q_Q(QImageView);

    if (modified != m) {
        modified = m;
        emit q->modifiedChanged(modified);
    }
}

void QImageViewPrivate::rotate(bool left)
{
    Q_Q(QImageView);

    QTransform matrix;
    matrix.rotate(left ? -90 : 90, Qt::ZAxis);
    image = this->image.transformed(matrix, Qt::SmoothTransformation);
    q->viewport()->update();

    addAxisAnimation(Qt::ZAxis, left ? - 90.0 : 90.0, 150);
}

void QImageViewPrivate::flipHorizontally()
{
    QTransform matrix;
    matrix.rotate(180, Qt::YAxis);
    image = image.transformed(matrix, Qt::SmoothTransformation);

    addAxisAnimation(Qt::YAxis, 180.0, 200);
}

void QImageViewPrivate::flipVertically()
{
    QTransform matrix;
    matrix.rotate(180, Qt::XAxis);
    image = image.transformed(matrix, Qt::SmoothTransformation);

    addAxisAnimation(Qt::XAxis, 180.0, 200);
}

void QImageViewPrivate::updateScrollBars()
{
    Q_Q(QImageView);

    QSize size = pixmap.size() * visualZoomFactor;
    int hmax = size.width() - q->viewport()->width();
    int vmax = size.height() - q->viewport()->height();

    hmax = qMax(0, hmax);
    vmax = qMax(0, vmax);

    q->horizontalScrollBar()->setRange(-hmax/2.0, hmax/2.0 + 0.5);
    q->verticalScrollBar()->setRange(-vmax/2.0, vmax/2.0 + 0.5);

    q->viewport()->update();
}

void QImageViewPrivate::updateViewport()
{
    Q_Q(QImageView);
    q->viewport()->update();
}

void QImageViewPrivate::animationFinished()
{
    axisAnimationCount--;
    if (!axisAnimationCount)
        syncPixmap();
}

void QImageViewPrivate::undoIndexChanged(int index)
{
    if (index == undoStackIndex)
        setModified(false);
    else
        setModified(true);
}

void QImageViewPrivate::addAxisAnimation(Qt::Axis axis, qreal endValue, int msecs)
{
    Q_Q(QImageView);

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

bool QImageViewPrivate::hasRunningAnimations() const
{
    return axisAnimationCount || (zoomAnimation.state() == QVariantAnimation::Running);
}

void QImageViewPrivate::stopAnimations()
{
    foreach (AxisAnimation *animation, runningAnimations) {
        animation->stop();
    }

    qDeleteAll(runningAnimations);
    runningAnimations.clear();
    axisAnimationCount = 0;
}

void QImageViewPrivate::syncPixmap()
{
    pixmap = QPixmap::fromImage(image);

    updateViewport();
}

void QImageViewPrivate::setImage(const QImage &image)
{
    this->image = image;

    stopAnimations();
    syncPixmap();
}

void QImageViewPrivate::updateThumbnailsState()
{
    Q_Q(QImageView);

    switch (thumbnailsPosition) {
    case QImageView::North:
    case QImageView::South:
        listWidget->setFlow(QListView::LeftToRight);
        listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        break;
    case QImageView::West:
    case QImageView::East:
        listWidget->setFlow(QListView::TopToBottom);
        listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        break;
    default:
        break;
    }

    if (q->imageCount() > 1) {
        QMargins margins(0, 0, 0, 0);
        switch (thumbnailsPosition) {
        case QImageView::North: margins.setTop(100); break;
        case QImageView::South: margins.setBottom(100); break;
        case QImageView::West: margins.setLeft(100); break;
        case QImageView::East: margins.setRight(100); break;
        default: break;
        }
        q->setViewportMargins(margins);
        listWidget->setVisible(true);
    } else {
        q->setViewportMargins(0, 0, 0, 0);
        listWidget->setVisible(false);
    }
}

void QImageViewPrivate::updateThumbnailsGeometry()
{
    Q_Q(QImageView);

    QRect rect = q->rect();
    switch (thumbnailsPosition) {
    case QImageView::North: rect.setHeight(100); break;
    case QImageView::South: rect.setY(rect.y() + rect.height() - 100); break;
    case QImageView::West: rect.setWidth(100); break;
    case QImageView::East: rect.setX(rect.x() + rect.width() - 100); break;
    default: break;
    }
    listWidget->setGeometry(rect);
}

QPointF QImageViewPrivate::getCenter() const
{
    Q_Q(const QImageView);

    int hvalue = q->horizontalScrollBar()->value();
    int vvalue = q->verticalScrollBar()->value();

    QSizeF size = q->viewport()->size()/2.0 - QSizeF(hvalue, vvalue);

    return QPointF(size.width(), size.height());
}

QRect QImageViewPrivate::selectedImageRect() const
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

qreal QImageViewPrivate::getFitInViewFactor() const
{
    Q_Q(const QImageView);

    QSize imageSize = image.size();
    if (imageSize.isEmpty())
        return 1.0;

    QSize size = q->maximumViewportSize();

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

    return factor;
}

void QImageViewPrivate::drawBackground(QPainter *p)
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

void QImageViewPrivate::drawSelection(QPainter *p)
{
    Q_Q(QImageView);

    if (mouseMode != QImageView::MouseModeSelect)
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

    QString text = QImageView::tr("%1 x %2").
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

QImageView::QImageView(QWidget *parent) :
    QAbstractScrollArea(parent),
    d_ptr(new QImageViewPrivate(this))
{
    Q_D(QImageView);

    setImage(QImage("/Users/arch/Pictures/2048px-Smiley.svg.png"));

    horizontalScrollBar()->setSingleStep(10);
    verticalScrollBar()->setSingleStep(10);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setMouseMode(MouseModeMove);

    QImageViewSettings *settings = QImageViewSettings::globalSettings();
    if (settings->useOpenGL())
        d->recreateViewport(true);
    settings->d_func()->addView(this);

    d->updateThumbnailsState();
}

QImageView::~QImageView()
{
    QImageViewSettings::globalSettings()->d_func()->removeView(this);
    delete d_ptr;
}

bool QImageView::canCopy() const
{
    Q_D(const QImageView);

    return d->canCopy;
}

bool QImageView::canRedo() const
{
    Q_D(const QImageView);

    return d->undoStack->canRedo();
}

bool QImageView::canUndo() const
{
    Q_D(const QImageView);

    return d->undoStack->canUndo();
}

void QImageView::read(QIODevice *device, const QByteArray &format)
{
    Q_D(QImageView);

    d->images.clear();
    d->listWidget->clear();

    QImageReader reader(device, format);
    for (int i = 0; i < reader.imageCount(); ++i) {
        QImageViewPrivate::ImageData data;
        data.image = reader.read();
        data.nextImageDelay = reader.nextImageDelay();
        d->images.append(data);

        QListWidgetItem *item = new QListWidgetItem(d->listWidget);
        item->setIcon(QIcon(QPixmap::fromImage(data.image)));
        d->listWidget->addItem(item);
    }

    if (d->images.isEmpty()) {
        d->zoomFactor = 1.0;
        d->visualZoomFactor = 1.0;
        d->updateScrollBars();
        return;
    }

    d->setImage(d->images.first().image);

    d->updateThumbnailsState();
    bestFit();
    viewport()->update();
}

void QImageView::write(QIODevice *device, const QByteArray &format)
{
    Q_D(QImageView);

    QImageWriter writer(device, format);
    writer.write(d->image);
}

QImage QImageView::image() const
{
    Q_D(const QImageView);

    return d->image;
}

void QImageView::setImage(const QImage &image)
{
    Q_D(QImageView);

    d->images.clear();

    if (image.isNull()) {
        d->setImage(QImage());
        d->currentImageNumber = -1;
        d->zoomFactor = 1.0;
        d->visualZoomFactor = 1.0;
        d->updateScrollBars();
        return;
    }

    d->setImage(image);
    QImageViewPrivate::ImageData data;
    data.image = image;
    data.nextImageDelay = 0;
    d->images.append(data);
    d->currentImageNumber = 0;

    d->updateThumbnailsState();
    bestFit();
    viewport()->update();
}

int QImageView::currentImageNumber() const
{
    Q_D(const QImageView);

    return d->currentImageNumber;
}

int QImageView::imageCount() const
{
    Q_D(const QImageView);

    return d->images.count();
}

bool QImageView::isModified() const
{
    Q_D(const QImageView);

    return d->modified;
}

void QImageView::setModified(bool modified)
{
    Q_D(QImageView);

    if (modified)
        d->undoStackIndex = -1; // we always have modified file
    else
        d->undoStackIndex = d->undoStack->index();

    d->setModified(modified);
}

QImageView::MouseMode QImageView::mouseMode() const
{
    Q_D(const QImageView);

    return d->mouseMode;
}

void QImageView::setMouseMode(QImageView::MouseMode mode)
{
    Q_D(QImageView);

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

QRect QImageView::selectedImageRect() const
{
    Q_D(const QImageView);

    return d->selectedImageRect();
}

QImage QImageView::selectedImage() const
{
    Q_D(const QImageView);

    return d->image.copy(selectedImageRect());
}

QImageView::Position QImageView::thumbnailsPosition() const
{
    Q_D(const QImageView);

    return d->thumbnailsPosition;
}

void QImageView::setThumbnailsPosition(QImageView::Position position)
{
    Q_D(QImageView);

    if (d->thumbnailsPosition == position)
        return;

    d->thumbnailsPosition = position;
    d->updateThumbnailsState();
}

void QImageView::zoomIn()
{
    Q_D(QImageView);

    d->setZoomFactor(d->zoomFactor*1.2);
}

void QImageView::zoomOut()
{
    Q_D(QImageView);

    d->setZoomFactor(d->zoomFactor*0.8);
}

void QImageView::bestFit()
{
    Q_D(QImageView);

    if (d->image.isNull())
        return;

    qreal factor = d->getFitInViewFactor();
    factor = qMin(factor, 1.0);
    d->setZoomFactor(factor);
}

void QImageView::fitInView()
{
    Q_D(QImageView);

    if (d->image.isNull())
        return;

    qreal factor = d->getFitInViewFactor();
    d->setZoomFactor(factor);
}

void QImageView::normalSize()
{
    Q_D(QImageView);

    d->setZoomFactor(1.0);
}

void QImageView::jumpToImage(int imageNumber)
{
    Q_D(QImageView);

    if (d->currentImageNumber == imageNumber)
        return;

    d->currentImageNumber = imageNumber;
    d->listWidget->setCurrentIndex(d->listWidget->model()->index(imageNumber, 0, QModelIndex()));
    d->setImage(d->images.at(imageNumber).image);
}

void QImageView::nextImage()
{
    int count = imageCount();
    if (!count)
        return;

    jumpToImage((currentImageNumber() + 1) % count);
}

void QImageView::prevImage()
{
    int count = imageCount();
    if (!count)
        return;

    jumpToImage((currentImageNumber() - 1 + count) % count);
}

void QImageView::resizeImage(const QSize &size)
{
    Q_D(QImageView);

    if (size.isEmpty())
        return;

    d->undoStack->push(new ResizeCommand(size, d));
}

void QImageView::rotateLeft()
{
    Q_D(QImageView);

    d->undoStack->push(new RotateCommand(true, d));
}

void QImageView::rotateRight()
{
    Q_D(QImageView);

    d->undoStack->push(new RotateCommand(false, d));
}

void QImageView::flipHorizontally()
{
    Q_D(QImageView);

    d->undoStack->push(new HFlipCommand(d));
}

void QImageView::flipVertically()
{
    Q_D(QImageView);

    d->undoStack->push(new VFlipCommand(d));
}

void QImageView::clearSelection()
{
    Q_D(QImageView);

    d->startPos = d->pos = QPoint();
    d->setCanCopy(false);
    viewport()->update();
}

void QImageView::copy()
{
    QImage image = selectedImage();

    QClipboard *clipboard = qApp->clipboard();
    clipboard->clear();
    clipboard->setImage(image);
}

void QImageView::cut()
{
    Q_D(QImageView);

    copy();

    d->undoStack->push(new CutCommand(selectedImageRect(), d));
}

void QImageView::redo()
{
    Q_D(QImageView);

    d->undoStack->redo();
}

void QImageView::undo()
{
    Q_D(QImageView);

    d->undoStack->undo();
}

void QImageView::mousePressEvent(QMouseEvent *e)
{
    Q_D(QImageView);

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

void QImageView::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(QImageView);

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

void QImageView::mouseReleaseEvent(QMouseEvent *)
{
    Q_D(QImageView);

    d->prevPos = QPoint();

    if (d->mouseMode == MouseModeMove)
        viewport()->setCursor(Qt::OpenHandCursor);
    d->mousePressed = false;

    viewport()->update();
}

void QImageView::keyPressEvent(QKeyEvent *e)
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

void QImageView::paintEvent(QPaintEvent *)
{
    Q_D(QImageView);

    QPainter p(viewport());
    if (!d->hasRunningAnimations())
        p.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

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

void QImageView::resizeEvent(QResizeEvent *)
{
    Q_D(QImageView);

    d->updateThumbnailsGeometry();
}

bool QImageView::viewportEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Resize : {
        Q_D(QImageView);

        d->updateScrollBars();
    }
    default:
        break;
    }

    return QAbstractScrollArea::viewportEvent(e);
}

#include "moc_qimageview.cpp"
