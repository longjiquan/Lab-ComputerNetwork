#ifndef SENDTHREAD_H
#define SENDTHREAD_H

#include <QObject>
#include <QThread>
#include <winsock2.h>
#include <stdio.h>

class SendThread : public QThread
{
    Q_OBJECT
public:
    explicit SendThread(QObject *parent=0);
    ~SendThread();
signals:
    void sendDone();
private:
    SOCKET acceptSocket;
    char* fullPath;
};

#endif // SENDTHREAD_H
