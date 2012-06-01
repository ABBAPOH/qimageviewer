#include "application.h"

#include <QFileOpenEvent>
#include <QUrl>

#include "mainwindow.h"

Application::Application(int &argc, char **argv, int flags) :
    QApplication(argc, argv, flags)
{
}

bool Application::notify(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *foe = static_cast<QFileOpenEvent*>(event);
        MainWindow::open(foe->url().toLocalFile());
        return true;
    }

    return QApplication::notify(object, event);
}
