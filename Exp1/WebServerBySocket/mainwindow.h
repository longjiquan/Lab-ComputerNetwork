void on_startPushBotton_clicked();
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "server.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionAbout_triggered();                /*弹出作者信息*/
    void on_startPushBotton_clicked();

private:
    Ui::MainWindow *ui;                             /*界面*/
    Server* server;                                 //服务器
    void setTableWidget();                           /*显示请求、响应*/
};

#endif // MAINWINDOW_H
