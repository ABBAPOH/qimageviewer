#ifndef QSINGLEIMAGEVIEW_H
#define QSINGLEIMAGEVIEW_H

#include <QAbstractScrollArea>

class QSingleImageViewPrivate;
class QSingleImageView : public QAbstractScrollArea
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSingleImageView)

    Q_PROPERTY(MouseMode mouseMode READ mouseMode WRITE setMouseMode NOTIFY mouseModeChanged)
    Q_PROPERTY(bool canCopy READ canRedo NOTIFY canCopyChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool modified READ isModified WRITE setModified NOTIFY modifiedChanged)

public:
    enum MouseMode {
        MouseModeMove,
        MouseModeSelect
    };
    Q_ENUMS(MouseMode)

    explicit QSingleImageView(QWidget *parent = 0);
    ~QSingleImageView();

    bool canCopy() const;

    bool canRedo() const;
    bool canUndo() const;

    QImage image() const;
    void setImage(const QImage &image);

    bool isModified() const;
    void setModified(bool modified);

    MouseMode mouseMode() const;
    void setMouseMode(MouseMode mode);

    QRect selectedImageRect() const;
    QImage selectedImage() const;

signals:
    void mouseModeChanged(MouseMode mode);
    void canCopyChanged(bool canCopy);
    void canRedoChanged(bool canRedo);
    void canUndoChanged(bool canUndo);
    void modifiedChanged(bool modified);

public slots:
    void zoomIn();
    void zoomOut();
    void bestFit();
    void fitInView();
    void normalSize();

    void rotateLeft();
    void rotateRight();

    void flipHorizontally();
    void flipVertically();

    void clearSelection();

    void copy();
    void cut();

    void redo();
    void undo();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void keyPressEvent(QKeyEvent *);

    void paintEvent(QPaintEvent *);
    bool viewportEvent(QEvent *);

protected:
    QSingleImageViewPrivate *d_ptr;

    friend class QImageViewSettings;

    Q_PRIVATE_SLOT(d_func(), void updateScrollBars())
    Q_PRIVATE_SLOT(d_func(), void animationFinished())
    Q_PRIVATE_SLOT(d_func(), void undoIndexChanged(int))
};

#endif // QSINGLEIMAGEVIEW_H
