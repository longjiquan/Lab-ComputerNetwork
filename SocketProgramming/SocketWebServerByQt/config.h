#ifndef CONFIG_H
#define CONFIG_H

#include <string>
using std::string;

class Config
{
public:
    static const int DEFAULT_SERVER_PORT;       //服务器默认端口号
    static const int DEFAULT_CLIENT_PORT;       //客户端默认监听端口
    static const string DEFAULT_SERVER_ADDR_STR; //服务器默认监听地址
    static const int MAX_CONNECTION_NUM;        //最大连接数
    static const int BUF_LENGTH;                //缓冲区大小
    static const string OK_STATUS_STR;          //客户端请求成功
    static const string ERROR_METHOD_STR;        //客户端语法错误，服务端尚未实现
    static const string DENY_STR;               //该客户端不在服务范围
    static const string NOT_FOUND_STR;          //文件不存在
    static const string DEFAULT_HTTP_VERSION_STR;       //HTTP版本
    static const string SERVER_STR;
private:
    Config();
    ~Config();
};

#endif // CONFIG_H
