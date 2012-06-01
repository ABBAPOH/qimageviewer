#include <QUrl>

#include "application.h"
#include "mainwindow.h"
#include "welcomewindow.h"

int main(int argc, char *argv[])
{
    Application app(argc, argv);

    QStringList files;
    foreach (const QString &argument, app.arguments().mid(1)) {
        QUrl url = QUrl::fromUserInput(argument);
        if (url.isLocalFile()) {
            QString file = url.toLocalFile();
            if (!file.isEmpty())
                files.append(file);
        }
    }

    if (!files.isEmpty()) {
        MainWindow::open(files);
        if (!app.topLevelWidgets().isEmpty())
            return app.exec();
    }

    WelcomeWindow w;
    w.show();

    return app.exec();
}
