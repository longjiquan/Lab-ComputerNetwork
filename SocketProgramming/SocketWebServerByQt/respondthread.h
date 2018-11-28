#ifndef RECVTHREAD_H
#define RECVTHREAD_H

#include <QObject>
#include <QThread>
#include <string>
#include <winsock2.h>

class RespondThread : public QThread
{
    Q_OBJECT
public:
    explicit RespondThread(QObject *parent=0);
    ~RespondThread();
    void run();
    void setAcceptSocket(SOCKET s);
    void setClientAddr(sockaddr_in addr);
    void setMainPath(std::string str);
protected:
//    int sendFile(FILE* infile,
//                  const std::string &httpHeader);
//    int sendNotFound();
//    int sendNotImplemented();
//    int sendHtml();
//    int sendPic();
signals:
    void respondDone(QStringList msg);
private:
    SOCKET acceptSocket;
    sockaddr_in clientSock;
    std::string mainPath;
    std::string clientAddrStr;
    int clientPort;
    std::string method;
    std::string url;
    std::string statusCode;
    int fileLength=0;
};

#endif // RECVTHREAD_H
