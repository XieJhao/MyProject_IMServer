#pragma once

#include<list>
#include<iostream>
#include<map>
#include "net/EventLoop.h"
#include "net/EventLoopThread.h"
#include "net/EventLoopThreadPool.h"
#include "net/TcpServer.h"
#include "base/Logging.h"
#include "ClientSession.h"
#include <mutex>

using namespace std;
using namespace muduo;
using namespace muduo::net;

class IMServer final //final表示该类不肯派生子类
{
public:
    IMServer() = default;
    // 禁用此函数
    IMServer(const IMServer &) = delete;
    ~IMServer() = default;
    IMServer &operator=(const IMServer &) = delete;
    bool Init(const string &ip, short port, EventLoop *loop);

    ClientSessionPtr GetSessionByID(int32_t userid);
    bool IsUserSessionExsit(int32_t userid);

protected:
    void OnConnection(const TcpConnectionPtr& conn);
    void OnClose(const TcpConnectionPtr &conn);

private:
    shared_ptr<TcpServer> m_server;
    map<string, ClientSessionPtr> m_mapclient;
    // list<TcpConnectionPtr> m_listConn;
    //主要保护m_mapclient的操作
    mutex m_sessionlock;
};

typedef pair<string, ClientSessionPtr> ConnPair;
typedef map<string, ClientSessionPtr>::iterator ConnIter;