#pragma once

#include<memory>
#include"../net/TcpConnection.h"
#include"../BinaryReader.h"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace edoyun;

typedef shared_ptr<TcpConnection> TcpConnectionPtr;

//为了让业务与逻辑分开，实际应该新增一个子类继承自TcpSession，让TcpSession中只有逻辑代码，其子类存放业务代码
class TcpSession
{
public:
    TcpSession(TcpConnectionPtr& conn);
    ~TcpSession();

    TcpSession(const TcpSession &rhs) = delete;
    TcpSession &operator=(const TcpSession &rhs) = delete;

    TcpConnectionPtr GetConnectionPtr()
    {
        return tmpConn_.lock();
    }

    void Send(const TcpConnectionPtr &conn, BinaryWriter &write);
    void Send(BinaryWriter& write);

private:
    // TcpConnectionPtr tmpConn;
    weak_ptr<TcpConnection> tmpConn_;
};
