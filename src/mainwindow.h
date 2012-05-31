#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QActionGroup;
class QImageView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void about();

    void open();
    static void open(const QString &file);
    static void open(const QStringList &files);
    void save();
    void saveAs();

    void onMoveToolTriggered(bool);
    void onSelectionToolTriggered(bool);

    static void preferences();

    void resizeImage();

    void updateSaveActions();

    void showWelcomeWindow();

private:
    void setupConnections();

    void setFile(const QString &file);

private:
    Ui::MainWindow *ui;
    QImageView *m_view;
    QString m_file;
    QActionGroup *m_toolGroup;
};

#endif // MAINWINDOW_H
