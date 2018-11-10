#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <stdio.h>
#include <QString>
#include <QObject>
#include "responseresult.h"

/*核心底层实现*/
class Server : public QObject
{
    Q_OBJECT
private:
    QString clientIP;
    int clientPort;
    QString mainPath;
    char* recvBuf;              //接受缓冲区
    SOCKET serverSocket;        //服务器socket
    sockaddr_in serverAddr;     //服务器端IP地址
    int serverPort;             //服务器端口号
    int maxClientNum;           //最大连接数
protected:
    int respond(SOCKET sAccept);                  //单线程处理
    int file_not_found(SOCKET sAccept);
    int file_ok(SOCKET sAccept, long flen);
    int send_file(SOCKET sAccept, FILE *resource);
    int send_not_found(SOCKET sAccept);
public:
    Server();
    ~Server();
    virtual int WinsockStartup();		//初始化Winsock
    virtual int ServerStartup();		//初始化Server，包括创建SOCKET，绑定到IP和PORT
    virtual int ListenStartup();        //通过监听套接字开始监听
    virtual int Loop();                 //循环等待、响应客户端请求
private slots:
    void handleResponseResult(const ResponseResult& rspRst);
};

#endif // SERVER_H
