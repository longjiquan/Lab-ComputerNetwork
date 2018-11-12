#ifndef ACCEPTTHREAD_H
#define ACCEPTTHREAD_H

#include <QThread>
#include <winsock2.h>
#include <QObject>
#include "respondthread.h"
#include <string>
using std::string;

class AcceptThread : public QThread
{
    Q_OBJECT
public:
    explicit AcceptThread(QObject *parent=0);
    ~AcceptThread();
    void setListenSocket(SOCKET s);
    void setMainPath(string str);
    void run();
signals:
    void acceptOK(QStringList msg);
    void acceptDone();
//    void acceptFailed();
private:
    SOCKET listenSocket;
    string mainPath;
    RespondThread *rpdThread;
};

#endif // ACCEPTTHREAD_H
