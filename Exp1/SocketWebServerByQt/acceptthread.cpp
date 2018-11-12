#include "acceptthread.h"
#include <QDebug>

AcceptThread::AcceptThread(QObject *parent)
    :QThread(parent)
{
    qDebug()<<"accept thread: "<<QThread::currentThreadId();
}
AcceptThread::~AcceptThread()
{
    requestInterruption();
    terminate();
    quit();
    wait();
    qDebug()<<"析构: accept thread";
    closesocket(this->listenSocket);
    qDebug()<<"accept thread析构，socket关闭";
    qDebug()<<"accept thread running:"<<isRunning();
    emit acceptDone();
}
void AcceptThread::setListenSocket(SOCKET s)
{
    this->listenSocket=s;
}
void AcceptThread::setMainPath(string str)
{
    this->mainPath=str;
}

//核心代码
void AcceptThread::run()
{
    SOCKET sAccept;
    sockaddr_in clientAddr;
    int iLen=sizeof(clientAddr);
    int i=0;//for debug
    while(!isInterruptionRequested())
    {
        //缓存清零
        memset(&clientAddr,0,sizeof(clientAddr));
        //等待新客户端连接，阻塞函数必须用terminate打断，requestinterruption不管用
        sAccept=accept(this->listenSocket,(sockaddr*)&clientAddr,&iLen);
        if(sAccept==INVALID_SOCKET)
        {
            qDebug()<<"accept error:"<<WSAGetLastError();
            return;
        }
        qDebug()<<"accept start i:"<<i;
        i++;
        //如果不是监听端口或者监听地址，直接断开连接，发送拒绝请求页面

        //开启新线程与客户端通信
        this->rpdThread=new RespondThread;
        this->rpdThread->setAcceptSocket(sAccept);
        this->rpdThread->setMainPath(this->mainPath);
        this->rpdThread->setClientAddr(clientAddr);
        rpdThread->start();

        connect(rpdThread,&RespondThread::respondDone,this,[=](QStringList msg){
            emit acceptOK(msg);
        });

        connect(this,&AcceptThread::acceptDone,this,[=](){
            delete rpdThread;
            rpdThread=nullptr;
        });
    }
}
