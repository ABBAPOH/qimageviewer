#include "qimageresizedialog.h"
#include "ui_qimageresizedialog.h"

QImageResizeDialog::QImageResizeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QImageResizeDialog),
    m_ignoreSignals(false)
{
    ui->setupUi(this);

    setWindowModality(Qt::WindowModal);

    connect(ui->fitIn, SIGNAL(currentIndexChanged(int)), SLOT(fitIn(int)));
    connect(ui->width, SIGNAL(valueChanged(int)), SLOT(widthChanged(int)));
    connect(ui->height, SIGNAL(valueChanged(int)), SLOT(heightChanged(int)));
    connect(ui->saveProportions, SIGNAL(clicked(bool)), SLOT(saveProportionsClicked(bool)));
}

QImageResizeDialog::~QImageResizeDialog()
{
    delete ui;
}

QSize QImageResizeDialog::imageSize() const
{
    return m_currentSize;
}

void QImageResizeDialog::setImageSize(const QSize &size)
{
    if (m_currentSize == size)
        return;

    m_size = size;
    m_currentSize = m_size;

    m_ignoreSignals = true;
    ui->width->setValue(m_currentSize.width());
    ui->height->setValue(m_currentSize.height());
    m_ignoreSignals = false;

    emit imageSizeChanged(m_currentSize);
}

void QImageResizeDialog::widthChanged(int value)
{
    if (m_ignoreSignals)
        return;

    if (ui->saveProportions->isChecked()) {
        m_ignoreSignals = true;
        ui->height->setValue(value/factor());
        m_currentSize.setHeight(ui->height->value());
        m_ignoreSignals = false;
    }
    ui->fitIn->setCurrentIndex(0);
    m_currentSize.setWidth(value);

    emit imageSizeChanged(m_currentSize);
}

void QImageResizeDialog::heightChanged(int value)
{
    if (m_ignoreSignals)
        return;

    if (ui->saveProportions->isChecked()) {
        m_ignoreSignals = true;
        ui->width->setValue(value*factor());
        m_currentSize.setWidth(ui->width->value());
        m_ignoreSignals = false;
    }
    ui->fitIn->setCurrentIndex(0);
    m_currentSize.setHeight(value);

    emit imageSizeChanged(m_currentSize);
}

void QImageResizeDialog::saveProportionsClicked(bool clicked)
{
    if (clicked) {
        ui->fitIn->setCurrentIndex(0);
        setImageSize(m_size);
    }
}

qreal QImageResizeDialog::factor() const
{
    return (qreal)m_size.width()/m_size.height();
}

QSize getFitInSize(int index)
{
    switch (index) {
    case 0 : return QSize();
    case 1 : return QSize(320, 240);
    case 2 : return QSize(640, 480);
    case 3 : return QSize(800, 600);
    case 4 : return QSize(1024, 768);
    case 5 : return QSize(1280, 1024);
    case 6 : return QSize(1280, 720);
    case 7 : return QSize(1920, 1080);
    default:
        break;
    }
    return QSize();
}

void QImageResizeDialog::fitIn(int index)
{
    QSize size = getFitInSize(index);
    if (size.isEmpty())
        return;

    m_ignoreSignals = true;
    m_currentSize = size;
    ui->width->setValue(size.width());
    ui->height->setValue(size.height());
    ui->saveProportions->setChecked(false);
    m_ignoreSignals = false;
}
