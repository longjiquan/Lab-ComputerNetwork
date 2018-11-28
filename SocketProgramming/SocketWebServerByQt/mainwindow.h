#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <winsock2.h>
#include "acceptthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void closeEvent(QCloseEvent *event);
private slots:
    void on_startPushBotton_clicked();

    void on_endPushBotton_clicked();

    void on_pathPushBotton_clicked();

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
    WSADATA mWsaData;
    SOCKET listenSocket=INVALID_SOCKET;
    AcceptThread *acptThread;
};

#endif // MAINWINDOW_H
