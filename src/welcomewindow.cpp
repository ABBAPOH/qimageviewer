#include "welcomewindow.h"
#include "ui_welcomewindow.h"

#include <QPointer>
#include <QSettings>
#include <QUrl>

#include <QApplication>
#include <QFileDialog>

#include "mainwindow.h"
#include "qimageviewsettings.h"
#include <QFileOpenEvent>

WelcomeWindow *m_window = 0;

WelcomeWindow::WelcomeWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WelcomeWindow)
{
    ui->setupUi(this);

    connect(ui->buttonOpen, SIGNAL(clicked()), SLOT(open()));
    connect(ui->buttonPreferences, SIGNAL(clicked()), SLOT(preferences()));
    connect(ui->buttonQuit, SIGNAL(clicked()), SLOT(quit()));

    m_window = this;
    loadSettings();
    qApp->installEventFilter(this);
}

WelcomeWindow::~WelcomeWindow()
{
    saveSettings();
    delete ui;
    m_window = 0;
}

WelcomeWindow *WelcomeWindow::window()
{
    return m_window;
}

void WelcomeWindow::open()
{
    QStringList files = QFileDialog::getOpenFileNames(this);
    if (files.isEmpty())
        return;

    MainWindow::open(files);
    close();
}

void WelcomeWindow::preferences()
{
    MainWindow::preferences();
}

void WelcomeWindow::quit()
{
    qApp->quit();
}

void WelcomeWindow::showWelcomeWindow()
{
    if (m_window) {
        m_window->show();
        m_window->raise();
        m_window->activateWindow();
    }
}

bool WelcomeWindow::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::FileOpen) {
        QFileOpenEvent *foe = static_cast<QFileOpenEvent*>(e);
        MainWindow::open(foe->url().toLocalFile());
        return true;
    }
    return QWidget::eventFilter(o, e);
}

void WelcomeWindow::loadSettings()
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

void WelcomeWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup("Image Viewer");
    QImageViewSettings *imageSettings = QImageViewSettings::globalSettings();

    settings.setValue("image background type", (int)imageSettings->imageBackgroundType());
    settings.setValue("image background color", imageSettings->imageBackgroundColor());
    settings.setValue("background color", imageSettings->backgroundColor());
    settings.setValue("use OpenGL", imageSettings->useOpenGL());
}
