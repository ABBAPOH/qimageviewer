#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "qsingleimageview.h"

#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_view = new QSingleImageView(this);
    setCentralWidget(m_view);

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(open()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(save()));
    connect(ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));

    connect(ui->actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

    connect(ui->actionCopy, SIGNAL(triggered()), m_view, SLOT(copy()));
    connect(ui->actionCut, SIGNAL(triggered()), m_view, SLOT(cut()));

    connect(ui->actionZoomIn, SIGNAL(triggered()), m_view, SLOT(zoomIn()));
    connect(ui->actionZoomOut, SIGNAL(triggered()), m_view, SLOT(zoomOut()));
    connect(ui->actionNormalSize, SIGNAL(triggered()), m_view, SLOT(normalSize()));
    connect(ui->actionBestFit, SIGNAL(triggered()), m_view, SLOT(bestFit()));

    connect(ui->actionRotateLeft, SIGNAL(triggered()), m_view, SLOT(rotateLeft()));
    connect(ui->actionRotateRight, SIGNAL(triggered()), m_view, SLOT(rotateRight()));

    connect(ui->actionFlipHorizontally, SIGNAL(triggered()), m_view, SLOT(flipHorizontally()));
    connect(ui->actionFlipVertically, SIGNAL(triggered()), m_view, SLOT(flipVertically()));

    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));

    resize(800, 600);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::about()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("About"));
    msgBox.setText(tr("QImageViewer"));
    msgBox.setInformativeText(tr("Based on Qt %1\n\n (c) Ivan Komissarov aka ABBAPOH").arg(QT_VERSION_STR));
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
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

void MainWindow::saveAs()
{
    QString file = QFileDialog::getSaveFileName(this);
    if (file.isEmpty())
        return;

    m_file = file;
    save();
}
