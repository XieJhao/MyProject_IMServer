#include"IMServer.h"
#include"ClientSession.h"
#include<sstream>

// IMServer::IMServer()
// {
    
// }

// IMServer::~IMServer()
// {
    
// }

bool IMServer::Init(const string &ip, short port, EventLoop *loop)
{
    InetAddress addr(ip, port);
    m_server.reset(new TcpServer(loop, addr, "chatserver",TcpServer::kReusePort));
    m_server->setConnectionCallback(std::bind(&IMServer::OnConnection, this, std::placeholders::_1));
    // m_server->setCloseCallback();
    m_server->start();
    return true;
}

void IMServer::OnConnection(const TcpConnectionPtr& conn)
{
    if(conn->connected())
    {
        ClientSessionPtr client(new ClientSession(conn));
        {
            lock_guard<mutex> guard(m_sessionlock);
            m_mapclient.insert(ConnPair(*client, client));
            
            // m_listConn.push_back(client);
        }
    }
    else
    {
        OnClose(conn);
    }
}

void IMServer::OnClose(const TcpConnectionPtr &conn)
{
    //TODO:处理连接,找到连接，进行关闭
    stringstream ss;
    ss << (void *)conn.get();
    ConnIter iter = m_mapclient.find(ss.str());
    if(iter != m_mapclient.end())
    {
        //TODO:关闭连接操作
        m_mapclient.erase(iter);
    }
    else
    {
        //TODO:有问题的连接
        cout << conn->name() << endl;
    }
}


ClientSessionPtr IMServer::GetSessionByID(int32_t userid)
{
    lock_guard<mutex> guard(m_sessionlock);
    ClientSessionPtr tmpSession;
    for (const auto& iter : m_mapclient)
    {
        if(iter.second->UserID() == userid)
        {
            tmpSession = iter.second;
            return tmpSession;
        }
    }
    return nullptr;
}


bool IMServer::IsUserSessionExsit(int32_t userid)
{
    std::lock_guard<mutex> guard(m_sessionlock);
    for(const auto& iter : m_mapclient)
    {
        if(iter.second->UserID() == userid)
        {
            return true;
        }
    }

    return false;
}
