#ifndef QSINGLEIMAGEVIEW_H
#define QSINGLEIMAGEVIEW_H

#include <QAbstractScrollArea>

class QSingleImageViewPrivate;
class QSingleImageView : public QAbstractScrollArea
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSingleImageView)

    Q_PROPERTY(MouseMode mouseMode READ mouseMode WRITE setMouseMode NOTIFY mouseModeChanged)

public:
    enum MouseMode {
        MouseModeMove
    };
    Q_ENUMS(MouseMode)

    explicit QSingleImageView(QWidget *parent = 0);
    ~QSingleImageView();

    QImage image() const;
    void setImage(const QImage &image);

    MouseMode mouseMode() const;
    void setMouseMode(MouseMode mode);

signals:
    void mouseModeChanged(MouseMode mode);

public slots:
    void zoomIn();
    void zoomOut();
    void bestFit();
    void normalSize();

    void rotateLeft();
    void rotateRight();

    void flipHorizontally();
    void flipVertically();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void paintEvent(QPaintEvent *);
    bool viewportEvent(QEvent *);

protected:
    QSingleImageViewPrivate *d_ptr;

    Q_PRIVATE_SLOT(d_func(), void updateScrollBars())
    Q_PRIVATE_SLOT(d_func(), void animationFinished())
};

#endif // QSINGLEIMAGEVIEW_H
