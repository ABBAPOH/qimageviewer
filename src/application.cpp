#include "application.h"

#include <QDir>
#include <QFileOpenEvent>
#include <QSettings>
#include <QUrl>

#include "mainwindow.h"
#include "qimageviewsettings.h"

Application::Application(const QString &id, int &argc, char **argv) :
    QtSingleApplication(id, argc, argv)
{
    connect(this, SIGNAL(messageReceived(QString)), SLOT(handleMessage(QString)));
    connect(this, SIGNAL(aboutToQuit()), SLOT(onAboutToQuit()));

    loadSettings();
}

void Application::handleArguments(const QStringList &arguments)
{
    if (arguments.isEmpty())
        return;

    QDir appDir(arguments.first());
    QStringList files;
    foreach (const QString &argument, arguments.mid(1)) {
        QUrl url;
        if (argument.startsWith("file://")) {
            url = QUrl(argument);
        } else {
            QFileInfo info(argument);
            if (info.isAbsolute())
                url = QUrl::fromLocalFile(info.absoluteFilePath());
            else {
                url = QUrl::fromLocalFile(appDir.absoluteFilePath(argument));
            }
        }

        if (url.isLocalFile()) {
            QString file = url.toLocalFile();
            if (!file.isEmpty())
                files.append(file);
        }
    }

    if (!files.isEmpty()) {
        MainWindow::openWindow(files);
        if (!topLevelWidgets().isEmpty())
            return;
    }

    MainWindow::newWindow();
}

void Application::handleMessage(const QString &message)
{
    QStringList arguments = message.split('\n');
    handleArguments(arguments);
}

void Application::onAboutToQuit()
{
    saveSettings();
}

bool Application::notify(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *foe = static_cast<QFileOpenEvent*>(event);
        MainWindow::openWindow(foe->url().toLocalFile());
        return true;
    }

    return QApplication::notify(object, event);
}

void Application::loadSettings()
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

void Application::saveSettings()
{
    QSettings settings;
    settings.beginGroup("Image Viewer");
    QImageViewSettings *imageSettings = QImageViewSettings::globalSettings();

    settings.setValue("image background type", (int)imageSettings->imageBackgroundType());
    settings.setValue("image background color", imageSettings->imageBackgroundColor());
    settings.setValue("background color", imageSettings->backgroundColor());
    settings.setValue("use OpenGL", imageSettings->useOpenGL());
}
