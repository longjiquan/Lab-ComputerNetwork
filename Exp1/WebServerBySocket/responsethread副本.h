#ifndef RESPONSETHREAD_H
#define RESPONSETHREAD_H

#include <QThread>
#include "responseresult.h"
#include <WinSock2.h>
#include <pthread.h>

void* ResponseThread(void* ps);
int sendHttpStatus(SOCKET sAccept, const char *statusLine);
int sendFile(SOCKET sAccept, FILE *resource);

#endif // RESPONSETHREAD_H
