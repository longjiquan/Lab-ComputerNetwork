#ifndef RECVTHREAD_H
#define RECVTHREAD_H

#include <QObject>
#include <QThread>
#include <string>
#include <winsock2.h>
using std::string;

class RespondThread : public QThread
{
    Q_OBJECT
public:
    explicit RespondThread(QObject *parent=0);
    ~RespondThread();
    void run();
    void setAcceptSocket(SOCKET s);
    void setClientAddr(sockaddr_in addr);
    void setMainPath(string str);
signals:
    void respondDone(QStringList msg);
    void recvFailed();
    void sendFailed();
private:
    SOCKET acceptSocket;
    sockaddr_in clientSock;
    string mainPath;
};

#endif // RECVTHREAD_H
