#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config.h"
#include <QMessageBox>
#include <QDebug>
#include <direct.h>
#include <QFileDialog>
#include "config.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //设置addrLineEdit
    ui->addrLineEdit->setText(QString::fromStdString(Config::DEFAULT_SERVER_ADDR_STR));

    //设置portSpinBox最大最小值
    ui->portSpinBox->setMinimum(-1);
    ui->portSpinBox->setMaximum(65535);
    ui->portSpinBox->setWrapping(true);
    ui->portSpinBox->setValue(Config::DEFAULT_SERVER_PORT);

    //设置maxSpinBox
    ui->maxConnectionSpinBox->setMinimum(0);
    ui->maxConnectionSpinBox->setValue(Config::MAX_CONNECTION_NUM);

    //设置pathLineEdit
    char *tempPath=new char[_MAX_PATH];
    _getcwd(tempPath,_MAX_PATH);
    QString tempQstr(tempPath);
    ui->pathLineEdit->setText(tempQstr);
    delete tempPath;

    //设置showTableWidget
    ui->showTableWidget->setColumnCount(7);//设置列数
    ui->showTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    QStringList tableHeaders;//设置列名
    tableHeaders<<"Time"<<"IP"<<"Port"<<"Method"<<"Url"<<"File Length"<<"Respond";
    ui->showTableWidget->setHorizontalHeaderLabels(tableHeaders);

#define MAJORVERSION 2  //Winsock主版本号
#define MINORVERSION 2	//Winsock次版本号
    //socket编程第一步，初始化winsock环境
    if((WSAStartup(MAKEWORD(MAJORVERSION,MINORVERSION),&(this->mWsaData))))
    {
        qDebug()<<"winsock初始化失败:"<<WSAGetLastError();
    }
}

MainWindow::~MainWindow()
{
    closesocket(this->listenSocket);
    WSACleanup();
    delete ui;
}

void MainWindow::on_startPushBotton_clicked()
{
    ui->startPushBotton->setDisabled(true);//不按下结束按钮不能再开始
    ui->pathPushBotton->setDisabled(true);

    //第二步，创建服务器套接字
    this->listenSocket=socket(AF_INET,SOCK_STREAM,0);
    if(this->listenSocket==INVALID_SOCKET)
    {
        QMessageBox::about(this,"Error","创建服务器套接字失败");
        qDebug()<<"socket error:"<<WSAGetLastError();
        ui->startPushBotton->setEnabled(true);
        ui->pathPushBotton->setEnabled(true);
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port=htons(ui->portSpinBox->value());
    serverAddr.sin_addr.S_un.S_addr=inet_addr(ui->addrLineEdit->text().toStdString().c_str());
    //第三步，绑定套接字和地址信息
    if((bind(this->listenSocket,(sockaddr*)&serverAddr,sizeof(serverAddr)))==SOCKET_ERROR)
    {
        qDebug()<<"port:"<<ui->portSpinBox->value();
        qDebug()<<"addr:"<<inet_addr(ui->addrLineEdit->text().toStdString().c_str());
        qDebug()<<"in addr any:"<<htonl(INADDR_ANY);
        QMessageBox::about(this,"Error","绑定套接字失败\n请重试");
        qDebug()<<"bind error:"<<WSAGetLastError();
        ui->startPushBotton->setEnabled(true);
        ui->pathPushBotton->setEnabled(true);
        return;
    }

    //第四步，端口监听
    if((listen(this->listenSocket,ui->maxConnectionSpinBox->value()))==SOCKET_ERROR)
    {
        QMessageBox::about(this,"Error","监听失败\n请重试");
        qDebug()<<"listen error:"<<WSAGetLastError();
        ui->startPushBotton->setEnabled(true);
        ui->pathPushBotton->setEnabled(true);
        return;
    }

    //后台启动线程与客户端连接，防止阻塞ui线程
    this->acptThread=new AcceptThread;
    this->acptThread->setListenSocket(this->listenSocket);
    this->acptThread->setMainPath(ui->pathLineEdit->text().toStdString());
    this->acptThread->start();

    //接收线程rpdThread发来的信号，更新UI，待实现
    connect(this->acptThread,&AcceptThread::acceptOK,[=](QStringList msg){
        qDebug()<<endl<<"msg length:"<<msg.length()<<endl;
        ui->showTableWidget->insertRow(ui->showTableWidget->rowCount());//插入一行
        QString tempStr;
        int rowIndex=ui->showTableWidget->rowCount();
        for(int i=0;i<msg.length();i++)
        {
            tempStr=msg.at(i);
            qDebug()<<"msg "<<i<<":"<<tempStr;
            QTableWidgetItem *item=new QTableWidgetItem(tempStr);
            ui->showTableWidget->setItem(rowIndex-1,i,item);
        }
    });
}
//用户关闭程序
void MainWindow::closeEvent(QCloseEvent *event)
{
    if(this->acptThread!=nullptr)
    {
        delete this->acptThread;
        this->acptThread=nullptr;
    }
}

void MainWindow::on_endPushBotton_clicked()
{
    //关闭线程
    if(this->acptThread!=nullptr)
    {
        delete this->acptThread;
        this->acptThread=nullptr;
    }
    QMessageBox::about(this,"reminder","关闭服务完成");
    ui->startPushBotton->setEnabled(true);
    ui->pathPushBotton->setEnabled(true);
//    ui->showTableWidget->clear();
}

void MainWindow::on_pathPushBotton_clicked()
{
    QString str=QFileDialog::getExistingDirectory(this,"打开主目录",".");
    ui->pathLineEdit->setText(str);
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,"作者","by CS1603 龙际全");
}
