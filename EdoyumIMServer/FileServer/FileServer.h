#pragma once

#include<list>
#include<map>
#include<mutex>
#include<iostream>
#include"../net/TcpServer.h"
#include"../net/EventLoop.h"
#include"FileSession.h"

using namespace muduo;
using namespace muduo::net;
using namespace std;

typedef struct 
{
    int32_t         userid;
    std::string     username;
    std::string     password;
    std::string     nickname;
}StoredUserInfo;

typedef shared_ptr<TcpServer> TcpServerPtr;

class FileServer final
{
public:
    FileServer() = default;
    ~FileServer() = default;

    FileServer(const FileServer& rhs) = delete;
    FileServer& operator =(const FileServer& rhs) = delete;

    bool Init(const char* ip, short port, EventLoop* loop);
private:
    //新连接到来调用或连接断开，所以需要通过conn->connected()来判断，一般只在主loop里面调用
    void OnConnection(TcpConnectionPtr conn);
    //断开连接
    void OnClose(const TcpConnectionPtr &conn);
private:
    TcpServerPtr                    m_server;
    map<string,FileSessionPtr>      m_sessions;
    mutex                           m_sessionMutex; //多线程之间保护m_sessions
    int                             m_baseUserId{};
    std::mutex                      m_idMutex;           //多线程之间保护m_baseUserId
};
