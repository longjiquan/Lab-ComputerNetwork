#include "server.h"
#include "config.h"
#include <winsock2.h>
#include <QMessageBox>
#include <QDebug>
#include "responsethread.h"
#include "responseresult.h"
#include <direct.h>//函数获取当前路径
#include <pthread.h>
#include <vector>
//#pragma comment(lib,"Ws2_32.lib")
//在pro文件中添加LIBS += -lpthread libwsock32 libws2_32

using namespace std;

//初始化配置
Server::Server()
{
    this->serverPort=Config::DEFAULT_PORT;
    this->maxClientNum=Config::MAXCONNECTION;
    char tempStr[_MAX_PATH];
    _getcwd(tempStr,_MAX_PATH);//获取的当前路径
    this->mainPath=tempStr;
    delete tempStr;
}
Server::~Server()
{
    //释放socket资源
    closesocket(this->serverSocket);
    WSACleanup();
}
/*初始化winsock环境*/
int Server::WinsockStartup()
{
#define MAJORVERSION 2  //Winsock主版本号
#define MINORVERSION 2	//Winsock次版本号

    WSADATA wsaData;//用于返回Winsock的环境信息
    int rtn;		//Winsock初始化函数的返回值
    WORD  wVersionRequested = MAKEWORD( MAJORVERSION, MINORVERSION ); //Winsock版本号，高字节次版本，低字节主版本号


    //初始化 winsock
    rtn = WSAStartup(wVersionRequested, &wsaData);

    if(rtn)
    { //如果返回值不是0，则Winsock初始化失败
        qDebug()<<"Winsock startup error"<<endl;
        return -1;
    }

    //判断返回的Winsock版本号
    if(LOBYTE(wsaData.wVersion) != MAJORVERSION || HIBYTE(wsaData.wVersion) != MINORVERSION)
    { //如果版本不对
        WSACleanup();  //清除Winsock
        qDebug() << "Winsock version error!\n";
        return -1;
    }

    qDebug() << "Winsock startup ok!\n";
    return 0;
}
/*创建监听socket，bind初始化server*/
int Server::ServerStartup()
{
    this->serverSocket=socket(AF_INET,SOCK_STREAM,0);
    if(this->serverSocket==INVALID_SOCKET)
    {
        qDebug()<<"创建监听套接字失败"<<endl;
        return Config::USER_ERROR;
    }

    //创建服务器地址：IP+端口号
    //htons和htonl函数把主机字节顺序转换为网络字节顺序，分别用于短整型和长整型数据
    this->serverAddr.sin_family=AF_INET;
    this->serverAddr.sin_port=htons(this->serverPort);
    this->serverAddr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);

    //绑定监听套接字和服务器地址
    if((bind(this->serverSocket,(LPSOCKADDR)&(this->serverAddr),sizeof(this->serverAddr)))==SOCKET_ERROR)
    {
        qDebug()<<"绑定套接字失败Error:"<<WSAGetLastError()<<endl;
        return Config::USER_ERROR;
    }
    return 0;
}
/*通过监听套接字监听*/
int Server::ListenStartup()
{
    if((listen(this->serverSocket,this->maxClientNum))==SOCKET_ERROR)
    {
        qDebug()<<"监听失败"<<endl;
        return Config::USER_ERROR;
    }
    return 0;
}
/*循环等待客户端请求、响应客户端请求*/
int Server::Loop()
{
    SOCKET sAccept;        //连接套接字
    sockaddr_in clientAddr;     //客户端地址
    int i=0;
    while(true)
    {
        qDebug()<<"i:"<<i<<endl;
        //接受客户端的连接请求，返回与该客户建立的连接套接字
        int clientLen=sizeof(clientAddr);
        sAccept=accept(this->serverSocket,(sockaddr*)&clientAddr,&clientLen);
        if(sAccept==INVALID_SOCKET)
        {
            //转错误处理
            qDebug()<<"连接失败Error:"<<WSAGetLastError()<<endl;
            break;
//            continue;
        }
        //多线程处理，接受多个浏览器请求，待实现

    }
    return 0;
}
//处理多线程响应中发来的响应结果
//待实现
//加油啊，dragondriver
void Server::handleResponseResult(const ResponseResult &rspRst)
{

}
int Server::respond(SOCKET sAccept)
{
    char recv_buf[BUF_LENGTH];
    char method[MIN_BUF];
    char url[MIN_BUF];
    char path[_MAX_PATH];
    int i, j;

    // 缓存清0，每次操作前都要记得清缓存，养成习惯；
    // 不清空可能出现的现象：输出乱码、换台机器乱码还各不相同
    // 原因：不清空会输出遇到 '\0'字符为止，所以前面的不是'\0' 也会一起输出
    memset(recv_buf, 0, sizeof(recv_buf));
    if (recv(sAccept, recv_buf, sizeof(recv_buf), 0) == SOCKET_ERROR)   //接收错误
    {
        qDebug()<<"recv() Failed:"<<WSAGetLastError();
        return USER_ERROR;
    }
    else
        qDebug()<<"recv data from client:"<<recv_buf; //接收成功，打印请求报文

                                                        //处理接收数据
    i = 0; j = 0;
    // 取出第一个单词，一般为HEAD、GET、POST
    while (!(' ' == recv_buf[j]) && (i < sizeof(method) - 1))
    {
        method[i] = recv_buf[j];
        i++; j++;
    }
    method[i] = '\0';   // 结束符，这里也是初学者很容易忽视的地方

    // 如果不是GET或HEAD方法，则直接断开本次连接
    // 如果想做的规范些可以返回浏览器一个501未实现的报头和页面
    if (stricmp(method, "GET") && stricmp(method, "HEAD"))
    {
        closesocket(sAccept); //释放连接套接字，结束与该客户的通信
        qDebug()<<"not get or head method.\nclose ok.\n";
        qDebug()<<"***********************\n\n\n\n";
        return USER_ERROR;
    }
    qDebug()<<"method:"<<method<<endl;

    // 提取出第二个单词(url文件路径，空格结束)，并把'/'改为windows下的路径分隔符'\'
    // 这里只考虑静态请求(比如url中出现'?'表示非静态，需要调用CGI脚本，'?'后面的字符串表示参数，多个参数用'+'隔开
    // 例如：www.csr.com/cgi_bin/cgi?arg1+arg2 该方法有时也叫查询，早期常用于搜索)
    i = 0;
    while ((' ' == recv_buf[j]) && (j < sizeof(recv_buf)))
        j++;
    while (!(' ' == recv_buf[j]) && (i < sizeof(recv_buf) - 1) && (j < sizeof(recv_buf)))
    {
        if (recv_buf[j] == '/')
            url[i] = '\\';
        else if (recv_buf[j] == ' ')
            break;
        else
            url[i] = recv_buf[j];
        i++; j++;
    }
    url[i] = '\0';
    qDebug()<<"url:"<<url<<endl;

    // 将请求的url路径转换为本地路径
    _getcwd(path, _MAX_PATH);
    strcat(path, url);
    qDebug()<<"path:"<<path<<endl;

    // 打开本地路径下的文件，网络传输中用r文本方式打开会出错
    FILE *resource = fopen(path, "rb");

    // 没有该文件则发送一个简单的404-file not found的html页面，并断开本次连接
    if (resource == NULL)
    {
        file_not_found(sAccept);
        // 如果method是GET，则发送自定义的file not found页面
        if (0 == stricmp(method, "GET"))
            send_not_found(sAccept);

        closesocket(sAccept); //释放连接套接字，结束与该客户的通信
        qDebug()<<"file not found.\nclose ok.\n";
        qDebug()<<"***********************\n\n\n\n";
        return USER_ERROR;
    }

    // 求出文件长度，记得重置文件指针到文件头
    fseek(resource, 0, SEEK_SET);
    fseek(resource, 0, SEEK_END);
    long flen = ftell(resource);
    qDebug()<<"file length:"<<flen<<endl;
    fseek(resource, 0, SEEK_SET);

    // 发送200 OK HEAD
    file_ok(sAccept, flen);

    // 如果是GET方法则发送请求的资源
    if (0 == stricmp(method, "GET"))
    {
        if (0 == send_file(sAccept, resource))
            qDebug()<<"file send ok.\n";
        else
            qDebug()<<"file send fail.\n";
    }
    fclose(resource);

    closesocket(sAccept); //释放连接套接字，结束与该客户的通信
    qDebug()<<"close ok.\n";
    qDebug()<<"***********************\n\n\n\n";
}
int Server::file_not_found(SOCKET sAccept)
{
    char send_buf[MIN_BUF];
    //  time_t timep;
    //  time(&timep);
    sprintf(send_buf, "HTTP/1.1 404 NOT FOUND\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    //  sprintf(send_buf, "Date: %s\r\n", ctime(&timep));
    //  send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "Connection: keep-alive\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, SERVER);
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "Content-Type: text/html\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    return 0;
}
int Server::file_ok(SOCKET sAccept, long flen)
{
    char send_buf[MIN_BUF];
    //  time_t timep;
    //  time(&timep);
    sprintf(send_buf, "HTTP/1.1 200 OK\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "Connection: keep-alive\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    //  sprintf(send_buf, "Date: %s\r\n", ctime(&timep));
    //  send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, SERVER);
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "Content-Length: %ld\r\n", flen);
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "Content-Type: text/html\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    return 0;
}
int Server::send_file(SOCKET sAccept, FILE *resource)
{
    char send_buf[BUF_LENGTH];
    while (1)
    {
        memset(send_buf, 0, sizeof(send_buf));       //缓存清0
        fgets(send_buf, sizeof(send_buf), resource);
        //  printf("send_buf: %s\n",send_buf);
        if (SOCKET_ERROR == send(sAccept, send_buf, strlen(send_buf), 0))
        {
            printf("send() Failed:%d\n", WSAGetLastError());
            return USER_ERROR;
        }
        if (feof(resource))
            return 0;
    }
}
int Server::send_not_found(SOCKET sAccept)
{
    char send_buf[MIN_BUF];
    sprintf(send_buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "<BODY><h1 align='center'>404</h1><br/><h1 align='center'>file not found.</h1>\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "</BODY></HTML>\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    return 0;
}
