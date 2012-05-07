#ifndef QSINGLEIMAGEVIEW_H
#define QSINGLEIMAGEVIEW_H

#include <QAbstractScrollArea>

class QSingleImageViewPrivate;
class QSingleImageView : public QAbstractScrollArea
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSingleImageView)

public:
    explicit QSingleImageView(QWidget *parent = 0);
    ~QSingleImageView();

    QImage image() const;
    void setImage(const QImage &image);

signals:

public slots:
    void zoomIn();
    void zoomOut();
    void bestFit();
    void normalSize();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void paintEvent(QPaintEvent *);
    bool viewportEvent(QEvent *);

protected:
    QSingleImageViewPrivate *d_ptr;

    Q_PRIVATE_SLOT(d_func(), void updateScrollBars())
};

#endif // QSINGLEIMAGEVIEW_H
