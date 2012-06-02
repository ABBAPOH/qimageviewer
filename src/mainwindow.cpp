#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QPointer>

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>

#include "qimageview.h"
#include "qimageresizedialog.h"
#include "qimageviewsettings.h"
#include "preferenceswidget.h"
#include "windowsmenu.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_view = new QImageView(this);
    setCentralWidget(m_view);

    m_toolGroup = new QActionGroup(this);
    m_toolGroup->setExclusive(true);
    m_toolGroup->addAction(ui->actionMoveTool);
    m_toolGroup->addAction(ui->actionSelectionTool);
    ui->menubar->insertMenu(ui->menuHelp->menuAction(), new WindowsMenu(ui->menubar));

    setupConnections();

    resize(800, 600);
//    m_view->setImage(QImage("/Users/arch/Pictures/2048px-Smiley.svg.png"));
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
    QStringList files = QFileDialog::getOpenFileNames(this);
    if (files.isEmpty())
        return;

    if (m_view->image().isNull()) {
        open(files.first());
        files = files.mid(1);
        if (!files.isEmpty())
            openWindow(files);
    } else {
        openWindow(files);
    }
}

void MainWindow::open(const QString &file)
{
    m_file = file;
    QFile *f = new QFile(file);
    if (!f->open(QFile::ReadOnly))
        qWarning() << "Can't open file" << file;
    m_view->read(f);
}

void MainWindow::openWindow(const QString &file)
{
    MainWindow *window = new MainWindow;
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
    window->open(file);
}

void MainWindow::openWindow(const QStringList &files)
{
    if (files.count() > 10) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Open"));
        msgBox.setText(tr("You are opening %1 files. Would you like to continue?").arg(files.count()));
        msgBox.setInformativeText(tr("Opening large amount of files can cause drop in performance"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        if (msgBox.exec() == QMessageBox::No)
            return;
    }

    foreach (const QString &file, files) {
        openWindow(file);
    }
}

void MainWindow::newWindow()
{
    MainWindow *window = new MainWindow;
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
}

void MainWindow::save()
{
    QFile f(m_file);
    m_view->write(&f, QFileInfo(m_file).suffix().toUtf8());
//    m_view->image().save(m_file);
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
        m_view->setMouseMode(QImageView::MouseModeMove);
}

void MainWindow::onSelectionToolTriggered(bool triggered)
{
    if (triggered)
        m_view->setMouseMode(QImageView::MouseModeSelect);
}

void MainWindow::preferences()
{
    static QPointer<PreferencesWidget> widget;

    if (!widget) {
        widget = new PreferencesWidget;
        widget->setAttribute(Qt::WA_DeleteOnClose);
        widget->show();
    } else {
        widget->show();
        widget->raise();
        widget->activateWindow();
    }
}

void MainWindow::resizeImage()
{
    QImageResizeDialog d(this);
    d.setImageSize(m_view->image().size());
    if (d.exec()) {
        m_view->resizeImage(d.imageSize());
    }
}

void MainWindow::updateSaveActions()
{
    bool canSaveAs = m_view->canWrite();
    bool canSave = canSaveAs && m_view->isModified();

    ui->actionSave->setEnabled(canSave);
    ui->actionSaveAs->setEnabled(canSaveAs);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    // TODO: add auto saving on quit
    if (!m_view->isModified())
        return;

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Save"));
    msgBox.setText(tr("File has been modified. Would you like to save it?"));
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard);
    msgBox.setWindowModality(Qt::WindowModal);
    int button = msgBox.exec();

    switch (button) {
    case QMessageBox::Save: {
        if (m_file.isEmpty())
            saveAs();
        else
            save();

        if (m_view->isModified())
            e->ignore();
        else
            e->accept();
        break;
    }
    case QMessageBox::Cancel: {
        e->ignore();
        break;
    }
    case QMessageBox::Discard: {
        e->accept();
        break;
    }
    default:
        break;
    }
}

void MainWindow::setupConnections()
{
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(open()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(save()));
    connect(m_view, SIGNAL(modifiedChanged(bool)), this, SLOT(updateSaveActions()));
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

    connect(m_view, SIGNAL(canWriteChanged(bool)), this, SLOT(updateSaveActions()));
    connect(m_view, SIGNAL(canWriteChanged(bool)), this, SLOT(updateSaveActions()));

    connect(ui->actionMoveTool, SIGNAL(triggered(bool)), this, SLOT(onMoveToolTriggered(bool)));
    connect(ui->actionSelectionTool, SIGNAL(triggered(bool)), this, SLOT(onSelectionToolTriggered(bool)));

    connect(ui->actionPreferences, SIGNAL(triggered(bool)), this, SLOT(preferences()));

    connect(ui->actionResizeImage, SIGNAL(triggered()), this, SLOT(resizeImage()));

    connect(ui->actionZoomIn, SIGNAL(triggered()), m_view, SLOT(zoomIn()));
    connect(ui->actionZoomOut, SIGNAL(triggered()), m_view, SLOT(zoomOut()));
    connect(ui->actionNormalSize, SIGNAL(triggered()), m_view, SLOT(normalSize()));
    connect(ui->actionFitInView, SIGNAL(triggered()), m_view, SLOT(fitInView()));

    connect(ui->actionPreviousImage, SIGNAL(triggered()), m_view, SLOT(prevImage()));
    connect(ui->actionNextImage, SIGNAL(triggered()), m_view, SLOT(nextImage()));

    connect(ui->actionRotateLeft, SIGNAL(triggered()), m_view, SLOT(rotateLeft()));
    connect(ui->actionRotateRight, SIGNAL(triggered()), m_view, SLOT(rotateRight()));

    connect(ui->actionFlipHorizontally, SIGNAL(triggered()), m_view, SLOT(flipHorizontally()));
    connect(ui->actionFlipVertically, SIGNAL(triggered()), m_view, SLOT(flipVertically()));

    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
}
