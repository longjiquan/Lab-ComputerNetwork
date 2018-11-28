#include "respondthread.h"
#include <QDebug>
#include "config.h"
#include <sstream>
#include <string>
#include <regex>
#include <QDebug>
#include <windows.h>//系统底层，判断目录是否存在
#include <QTime>

static bool endWith(const std::string &fullStr,const std::string &endStr)
{
    if(endStr.size()>fullStr.size())
    {
        return false;
    }
    int indexFull=fullStr.size()-1;
    int indexEnd=endStr.size()-1;
    while(indexEnd>=0)
    {
        if(fullStr[indexFull]!=endStr[indexEnd])
        {
            return false;
        }
        indexFull--;
        indexEnd--;
    }
    return true;
}

RespondThread::RespondThread(QObject *parent)
    :QThread(parent)
{
    qDebug()<<"recv thread:"<<QThread::currentThreadId();
}
RespondThread::~RespondThread()
{
    requestInterruption();
    terminate();
    quit();
    wait();
    qDebug()<<"recv thread析构";
    closesocket(acceptSocket);
    qDebug()<<"recv thread 析构，socket关闭";
    qDebug()<<"recv thread running:"<<isRunning();
}
void RespondThread::setAcceptSocket(SOCKET s)
{
    acceptSocket=s;
}
void RespondThread::setClientAddr(sockaddr_in addr)
{
    clientSock=addr;
}
void RespondThread::setMainPath(string str)
{
    mainPath=str;
}

//核心代码
void RespondThread::run()
{
    while(!isInterruptionRequested())
    {
        char recvBuf[Config::BUF_LENGTH];
        char sendBuf[Config::BUF_LENGTH];
    //    std::string method,url;
        std::string fullPath;
        std::string respondHttpHeader,clientHttpHeader;
        std::string statusCode,firstHeader,typeStr,lengthStr;
        std::regex regRequest(R"(([A-Z]+) (.*?) HTTP/\d\.\d)");
        std::smatch matchRst;
    //    std::string httpVersion;
        std::string strRecv,strSend;
    //    std::stringstream recvSS;
        QStringList msg;
        clientAddrStr.assign(inet_ntoa(clientSock.sin_addr));
        clientPort=clientSock.sin_port;

        int rtn;
        int i,j;
        int bufReadNum;

        DWORD ftyp;

        //缓存清零
        j=0;
        respondHttpHeader="";
        statusCode=Config::OK_STATUS_STR;
        firstHeader="HTTP/1.1 200 OK\r\n";

        memset(recvBuf,0,sizeof(recvBuf));
        rtn=recv(acceptSocket,recvBuf,sizeof(recvBuf),0);
        if(rtn==SOCKET_ERROR)
        {//接收失败
            return;
        }
        if(rtn==0)
        {//对方断开连接
            return;
        }
//        qDebug()<<"recv buf:"<<endl<<QString(recvBuf);

        //找到HTTP报文头部
        strRecv.assign(recvBuf);
        size_t headerEnd=strRecv.find("\r\n\r\n");
        clientHttpHeader=strRecv.substr(0,headerEnd);
        qDebug()<<"client http header:"<<clientHttpHeader.c_str();

        //正则表达式提取关键字method(GET,POST)，url
        if(std::regex_search(clientHttpHeader,matchRst,regRequest))
        {
            method=matchRst[1].str();
            url=matchRst[2].str();
        }
        qDebug()<<"method:"<<method.c_str();
        qDebug()<<"url:"<<url.c_str();

        for(i=0;i<url.size();i++)
        {//替换成Windows路径
            if(url[i]=='/')
                url[i]='\\';
        }
        fullPath=mainPath+url;//完整路径

        ftyp=GetFileAttributesA(fullPath.c_str());
        if((ftyp&FILE_ATTRIBUTE_DIRECTORY)&&(!INVALID_FILE_ATTRIBUTES))
        {//是一个目录，打开该目录下的索引index.html
            fullPath=fullPath+"\\index.html";
        }

        FILE* infile=fopen(fullPath.c_str(),"rb");


        if(endWith(url,".html")||endWith(url,"htm"))
        {
            typeStr="Content-Type: text/html\r\n";
        }
        else if(endWith(url,".jpg"))
        {
            typeStr="Content-Type: image/jpeg\r\n";
        }
        else if(endWith(url,".jpeg")||endWith(url,".png"))
        {
            typeStr="Content-Type: image/"
                    +(url.substr(url.rfind('.')+1))
                    +"\r\n";
        }
        else if(endWith(url,".ico"))
        {
            typeStr="Content-Type: image/x-icon\r\n";
        }
        else if(!infile)
        {//文件不存在
            fclose(infile);
            //发送自定义404页面
            infile=fopen("404.html","rb");
            statusCode=Config::NOT_FOUND_STR;
            firstHeader="HTTP/1.1 404 Not Found\r\n";
            typeStr="Content-Type: text/html\r\n";
        }
        if(!infile)
        {
            fclose(infile);
            //发送自定义501页面
            infile=fopen("501.html","rb");
            statusCode=Config::ERROR_METHOD_STR;
            firstHeader="HTTP/1.1 501 Not Inplemented\r\n";
            typeStr="Content-Type: text/html\r\n";
        }
        //获取文件大小
        fseek(infile,0,SEEK_SET);
        fseek(infile,0,SEEK_END);
        fileLength=ftell(infile);
        //文件指针归位
        fseek(infile,0,SEEK_SET);

        respondHttpHeader=firstHeader
                +typeStr
                +"Content-Length: "+std::to_string(fileLength)+"\r\n"
                +"Server: csr_http1.1\r\n"
                +"Connection: close\r\n"
                +"\r\n";

        qDebug()<<"respond http header:"<<respondHttpHeader.c_str();
        //发送报头
        rtn=send(acceptSocket,
                 respondHttpHeader.c_str(),
                 (int)(respondHttpHeader.length()),
                 0);

        //发送请求的文件        
        while(true)
        {
            //缓存清零
            memset(sendBuf,0,sizeof(sendBuf));
            bufReadNum=fread(sendBuf,1,Config::BUF_LENGTH,infile);
//            fgets(sendBuf,sizeof(sendBuf),infile);不对，坑人
//            qDebug()<<"send buf"<<sendBuf;
            if(SOCKET_ERROR==(send(acceptSocket,sendBuf,bufReadNum,0)))
            {//发送失败
                rtn=SOCKET_ERROR;
                break;
            }
            if(feof(infile))
                break;
        }
        fclose(infile);
        msg.clear();
        msg<<QTime::currentTime().toString("hh:mm:ss")
          <<QString::fromStdString(clientAddrStr)
          <<QString::fromStdString(std::to_string(clientPort))
         <<QString::fromStdString(method)
        <<QString::fromStdString(url)
        <<QString::fromStdString(std::to_string(fileLength));
        if(rtn==SOCKET_ERROR)
        {
            msg<<QString::fromStdString(std::to_string(WSAGetLastError()));
        }
        else
        {
            msg<<QString::fromStdString(statusCode);
        }
        emit respondDone(msg);
        return;
    }
}
