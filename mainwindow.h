#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QActionGroup;
class QSingleImageView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void about();

    void open();
    void save();
    void saveAs();

    void onMoveToolTriggered(bool);
    void onSelectionToolTriggered(bool);

private:
    Ui::MainWindow *ui;
    QSingleImageView *m_view;
    QString m_file;
    QActionGroup *m_toolGroup;
};

#endif // MAINWINDOW_H
