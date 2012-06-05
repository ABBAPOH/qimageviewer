#include "application.h"

#include <QDir>
#include <QFileOpenEvent>
#include <QUrl>

#include "mainwindow.h"

Application::Application(const QString &id, int &argc, char **argv) :
    QtSingleApplication(id, argc, argv)
{
    connect(this, SIGNAL(messageReceived(QString)), SLOT(handleMessage(QString)));
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
