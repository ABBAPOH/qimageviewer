#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QPointer>
#include <QSettings>

#include <QFileDialog>
#include <QMessageBox>

#include "qimageview.h"
#include "qimageresizedialog.h"
#include "qimageviewsettings.h"
#include "preferenceswidget.h"

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

    setupConnections();

    resize(800, 600);
    loadSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
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
    QFile *f = new QFile(file);
    if (!f->open(QFile::ReadOnly))
        qWarning() << "Can't open file" << file;
    m_view->read(f);
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

void MainWindow::loadSettings()
{
    QSettings settings;
    settings.beginGroup("Image Viewer");
    QImageViewSettings *imageSettings = QImageViewSettings::globalSettings();

    int type = settings.value("image background type", QImageViewSettings::None).toInt();
    QColor imageColor = settings.value("image background color", QColor(255, 255, 255)).value<QColor>();
    QColor backgroundColor = settings.value("background color", QColor(128, 128, 128)).value<QColor>();
    bool useOpenGL = settings.value("use OpenGL", false).toBool();

    imageSettings->setiImageBackgroundType(QImageViewSettings::ImageBackgroundType(type));
    imageSettings->setImageBackgroundColor(imageColor);
    imageSettings->setBackgroundColor(backgroundColor);
    imageSettings->setUseOpenGL(useOpenGL);
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup("Image Viewer");
    QImageViewSettings *imageSettings = QImageViewSettings::globalSettings();

    settings.setValue("image background type", (int)imageSettings->imageBackgroundType());
    settings.setValue("image background color", imageSettings->imageBackgroundColor());
    settings.setValue("background color", imageSettings->backgroundColor());
    settings.setValue("use OpenGL", imageSettings->useOpenGL());
}
