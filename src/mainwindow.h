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
    void open(const QString &file);
    static void openWindow(const QString &file);
    static void openWindow(const QStringList &files);
    static void newWindow();
    void save();
    void saveAs();

    void onMoveToolTriggered(bool);
    void onSelectionToolTriggered(bool);

    static void preferences();

    void resizeImage();

    void updateSaveActions();

protected:
    void closeEvent(QCloseEvent *);

private:
    void setupConnections();

private:
    Ui::MainWindow *ui;
    QImageView *m_view;
    QString m_file;
    QActionGroup *m_toolGroup;
};

#endif // MAINWINDOW_H
