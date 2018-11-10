#include "responsethread.h"
#include <QDebug>
#include "config.h"
#include "responseresult.h"
#include <winsock2.h>
//#pragma comment(lib,"Ws2_32.lib")
//在pro文件中添加LIBS += -lpthread libwsock32 libws2_32


//处理连接套接字
void* ResponseThread(void* ps)
{
    SOCKET *psAccept=(SOCKET*)(ps);
    char recvBuf[Config::BUF_LENGTH];
    char tempBuf[Config::BUF_LENGTH];
    QString methodStr;
    QString urlStr;
    QString fullPath;
    string statusLine;
    int i,j;
    ResponseResult rpdRst;

    //缓存清零
    memset(recvBuf,0,sizeof(recvBuf));

    //从接受套接字接收来自客户端的数据并放入缓冲区
    if((recv(*psAccept,recvBuf,sizeof(recvBuf),0))==SOCKET_ERROR)
    {
        //接收数据失败，转错误处理
        qDebug()<<"接收数据失败Error:"<<WSAGetLastError()<<endl;
    }
    else
    {//接收成功
        //待实现：将请求报文输出到日志文件
        qDebug()<<"接收数据成功"<<endl;
    }


    //处理接收数据
    i=j=0;
    //取出第一个单词，一般为HEAD,GET,POST
    qDebug()<<"get first word"<<endl;
    while(!(' '==recvBuf[j]))
    {
        methodStr.push_back(recvBuf[j]);
        j++;
    }
    qDebug()<<"method:"<<methodStr<<endl;

    //如果不是get或者head方法，断开本次连接
    if(methodStr!="HEAD" && methodStr!="GET")
    {
        //返回一个501未实现的报头和页面，功能待实现
        qDebug()<<"该方法待实现"<<endl;
    }

    //获取第二个单词，URL文件路径
    //注意把'/'改为Windows下路径'\'
    //只考虑静态请求
    qDebug()<<"get second word"<<endl;
    i=0;
    while((' '==recvBuf[j])&&(j<sizeof(recvBuf))) j++;
    while(!(' ' == recvBuf[j]) && (i < sizeof(recvBuf) - 1) && (j < sizeof(recvBuf)))
    {
        if(recvBuf[j]=='/')urlStr.push_back('\\');
        else if(recvBuf[j]==' ')break;
        else urlStr.push_back(recvBuf[j]);
        j++;
    }
    qDebug()<<"url:"<<urlStr<<endl;

    // 打开本地路径下的文件，网络传输中用r文本方式打开会出错
    qDebug()<<"send file ready"<<endl;
    FILE *resource = fopen(fullPath.toStdString().c_str(), "rb");

    //没有该文件，发送404，断开本次连接
    if(resource==NULL)
    {
        statusLine.assign("HTTP/1.1 404 NOT FOUND\r\n");
        if((sendHttpStatus(*psAccept, statusLine.c_str()))==Config::USER_ERROR)
        {
            qDebug()<<"发送数据失败"<<endl;
        }
        //发送自定义404文件，待实现

    }

    //求出文件长度，重置文件指针到文件头
    fseek(resource,0,SEEK_SET);
    fseek(resource,0,SEEK_END);
    long flen=ftell(resource);
    fseek(resource,0,SEEK_SET);//文件指针归位

    //发送200 OK HEAD
    qDebug()<<"send status"<<endl;
    statusLine.assign("HTTP/1.1 200 OK\r\n");
    sendHttpStatus(*psAccept, statusLine.c_str());

    //get方法，发送请求的资源
    if(methodStr=="GET")
    {
        if((sendFile(*psAccept,resource))==Config::USER_ERROR)
        {
            //文件发送错误，转错误处理
            qDebug()<<"文件发送失败Error:"<<WSAGetLastError()<<endl;
        }
    }
    fclose(resource);
    qDebug()<<"respond thread done"<<endl;
    return (void*)(&rpdRst);
}
//发送HTTP状态
//报文详见
//https://www.cnblogs.com/biyeymyhjob/archive/2012/07/28/2612910.html
int sendHttpStatus(SOCKET sAccept, const char *statusLine)
{
    char sendBuf[Config::MIN_BUF];
    sprintf(sendBuf, statusLine);
    send(sAccept, sendBuf, strlen(sendBuf), 0);
    sprintf(sendBuf, "Connection: keep-alive\r\n");
    send(sAccept, sendBuf, strlen(sendBuf), 0);
    sprintf(sendBuf, Config::SERVER_STR.c_str());
    send(sAccept, sendBuf, strlen(sendBuf), 0);
    sprintf(sendBuf, "Content-Type: text/html\r\n");
    send(sAccept, sendBuf, strlen(sendBuf), 0);
    sprintf(sendBuf, "\r\n");
    send(sAccept, sendBuf, strlen(sendBuf), 0);
    return 0;
}
//发送请求的文件
int sendFile(SOCKET sAccept, FILE *resource)
{
    char sendBuf[Config::BUF_LENGTH];
    while(true)
    {
        memset(sendBuf,0,sizeof(sendBuf));//缓存清零
        fgets(sendBuf,sizeof(sendBuf),resource);
        if((send(sAccept,sendBuf,strlen(sendBuf),0))==SOCKET_ERROR)
        {
            //转异常处理
            return Config::USER_ERROR;
        }
        if(feof(resource)) return 0;
    }
}
