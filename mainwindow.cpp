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

    m_toolGroup = new QActionGroup(this);
    m_toolGroup->setExclusive(true);
    m_toolGroup->addAction(ui->actionMoveTool);
    m_toolGroup->addAction(ui->actionSelectionTool);

    setupConnections();

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
    m_view->setModified(false);
}

void MainWindow::saveAs()
{
    QString file = QFileDialog::getSaveFileName(this);
    if (file.isEmpty())
        return;

    m_file = file;
    save();
}

void MainWindow::onMoveToolTriggered(bool triggered)
{
    if (triggered)
        m_view->setMouseMode(QSingleImageView::MouseModeMove);
}

void MainWindow::onSelectionToolTriggered(bool triggered)
{
    if (triggered)
        m_view->setMouseMode(QSingleImageView::MouseModeSelect);
}

void MainWindow::setupConnections()
{
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(open()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(save()));
    connect(m_view, SIGNAL(modifiedChanged(bool)), ui->actionSave, SLOT(setEnabled(bool)));
    connect(ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));

    connect(ui->actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

    connect(ui->actionRedo, SIGNAL(triggered()), m_view, SLOT(redo()));
    connect(m_view, SIGNAL(canRedoChanged(bool)), ui->actionRedo, SLOT(setEnabled(bool)));
    connect(ui->actionUndo, SIGNAL(triggered()), m_view, SLOT(undo()));
    connect(m_view, SIGNAL(canUndoChanged(bool)), ui->actionUndo, SLOT(setEnabled(bool)));

    connect(ui->actionCopy, SIGNAL(triggered()), m_view, SLOT(copy()));
    connect(m_view, SIGNAL(canCopyChanged(bool)), ui->actionCopy, SLOT(setEnabled(bool)));
    connect(ui->actionCut, SIGNAL(triggered()), m_view, SLOT(cut()));
    connect(m_view, SIGNAL(canCopyChanged(bool)), ui->actionCut, SLOT(setEnabled(bool)));

    connect(ui->actionMoveTool, SIGNAL(triggered(bool)), this, SLOT(onMoveToolTriggered(bool)));
    connect(ui->actionSelectionTool, SIGNAL(triggered(bool)), this, SLOT(onSelectionToolTriggered(bool)));

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
}
