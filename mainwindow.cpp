#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "qsingleimageview.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_view = new QSingleImageView(this);
    setCentralWidget(m_view);

    connect(ui->actionZoomIn, SIGNAL(triggered()), m_view, SLOT(zoomIn()));
    connect(ui->actionZoomOut, SIGNAL(triggered()), m_view, SLOT(zoomOut()));
    connect(ui->actionNormalSize, SIGNAL(triggered()), m_view, SLOT(normalSize()));
    connect(ui->actionBestFit, SIGNAL(triggered()), m_view, SLOT(bestFit()));

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(open()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(save()));

    connect(ui->actionRotateLeft, SIGNAL(triggered()), m_view, SLOT(rotateLeft()));
    connect(ui->actionRotateRight, SIGNAL(triggered()), m_view, SLOT(rotateRight()));

    connect(ui->actionFlipHorizontally, SIGNAL(triggered()), m_view, SLOT(flipHorizontally()));

    resize(640, 480);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::open()
{
    QString file = QFileDialog::getOpenFileName(this);
    if (file.isEmpty())
        return;

    m_file = file;
    m_view->setImage(QImage(file));
}

void MainWindow::save()
{
    m_view->image().save(m_file);
}
