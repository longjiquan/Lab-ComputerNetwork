#ifndef RESPONDTHREAD_H
#define RESPONDTHREAD_H

#include <QThread>

class RespondThread : public QThread
{
public:
    RespondThread();
    ~RespondThread();
};

#endif // RESPONDTHREAD_H
