#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <WinSock2.h>
using namespace std;

class Config
{
public:
    static const int DEFAULT_PORT;          /*默认端口*/
    static const int BUF_LENGTH;            /*缓冲区大小*/
    static const int MIN_BUF;               /*最小缓冲区，由报文格式决定的*/
    static const int USER_ERROR;            /*错误码*/
    static const string SERVER_STR;         /*服务端字符串*/
    static const u_long BLOCK_MODE;         /*非租塞模式*/
    static const int MAXCONNECTION;         /*最大连接数*/
private:
    Config();
    ~Config();
};

#endif // CONFIG_H
