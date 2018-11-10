#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QStringList>
#include <QString>
#include <QDebug>
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tableWidget->setColumnCount(6);
    QStringList tableHeader;
    tableHeader<<"IP"<<"Port"<<"Method"<<"Url"<<"Response"<<"Error";
    ui->tableWidget->setHorizontalHeaderLabels(tableHeader);


}

MainWindow::~MainWindow()
{
    delete ui;
}

/*弹出作者信息*/
void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,"作者","by CS1603 龙际全");
}

void MainWindow::on_startPushBotton_clicked()
{
    this->server=new Server();
    server->WinsockStartup();
    server->ServerStartup();
    server->ListenStartup();
    server->Loop();
}
