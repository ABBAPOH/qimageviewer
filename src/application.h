#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include "qtsingleapplication/qtsingleapplication.h"

class Application : public QtSingleApplication
{
    Q_OBJECT
public:
    explicit Application(const QString &id, int &argc, char **argv);

    void handleArguments(const QStringList &arguments);

    void restoreSession();
    void storeSession();

protected slots:
    void handleMessage(const QString &message);
    void onAboutToQuit();

protected:
    bool notify(QObject *object, QEvent *event);

    void loadSettings();
    void saveSettings();

    QByteArray saveState() const;
    bool restoreState(const QByteArray &state);
};

#endif // APPLICATION_H
