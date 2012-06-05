#include <QDir>
#include <QUrl>

#include "application.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    Application app("QImageViewer", argc, argv);

    QStringList arguments = app.arguments();
    arguments[0] = QDir::currentPath();

    if (app.isRunning()) {
        app.sendMessage(arguments.join("\n"));
        return 0;
    }

    app.handleArguments(arguments);

    return app.exec();
}
