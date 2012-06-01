#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <QWidget>

namespace Ui {
class WelcomeWindow;
}

class WelcomeWindow : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomeWindow(QWidget *parent = 0);
    ~WelcomeWindow();

    static WelcomeWindow *window();

public slots:
    void open();
    static void preferences();
    void quit();
    static void showWelcomeWindow();

private:
    void loadSettings();
    void saveSettings();

private:
    Ui::WelcomeWindow *ui;
};

#endif // WELCOMEWINDOW_H
