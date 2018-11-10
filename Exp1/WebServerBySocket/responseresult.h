#ifndef RESPONSERESULT_H
#define RESPONSERESULT_H

#include <QString>

typedef struct ResponseResult
{
    int recvStatus;         //接受客户端是否出错？
    QString methodStr;       //请求方法
    int sendStatus;         //是否发送成功？
    QString urlStr;          //请求的文件路径
}ResponseResult;

#endif // RESPONSERESULT_H
