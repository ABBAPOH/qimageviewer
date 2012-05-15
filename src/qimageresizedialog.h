#ifndef QIMAGERESIZEDIALOG_H
#define QIMAGERESIZEDIALOG_H

#include <QDialog>

namespace Ui {
class QImageResizeDialog;
}

class QImageResizeDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QSize imageSize READ imageSize WRITE setImageSize NOTIFY imageSizeChanged)

public:
    explicit QImageResizeDialog(QWidget *parent = 0);
    ~QImageResizeDialog();

    QSize imageSize() const;
    void setImageSize(const QSize &size);

signals:
    void imageSizeChanged(const QSize &);

private slots:
    void fitIn(int);
    void widthChanged(int);
    void heightChanged(int);
    void saveProportionsClicked(bool);

private:
    qreal factor() const;

private:
    Ui::QImageResizeDialog *ui;

    QSize m_size;
    QSize m_currentSize;
    bool m_ignoreSignals;
};

#endif // QIMAGERESIZEDIALOG_H
