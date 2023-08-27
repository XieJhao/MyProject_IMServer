#include"FileServer.h"
#include"../net/InetAddress.h"
#include"../base/Singleton.h"
#include"FileSession.h"
#include"../base/Logging.h"
#include<sstream>

bool FileServer::Init(const char* ip, short port, EventLoop* loop)
{
    // InetAddress addr(ip, port);
    m_server.reset(new TcpServer(loop, InetAddress(ip,port), "edoyun-fileservre",TcpServer::kReusePort));

    m_server->setConnectionCallback(std::bind(&FileServer::OnConnection, this, std::placeholders::_1));

    //启动监听
    m_server->start();

    return true;
}

void FileServer::OnConnection(TcpConnectionPtr conn)
{
    if(conn->connected())
    {
        //  cout <<__FILE__<<"("<<__LINE__<<")"
        LOG_INFO<< "client connected:" << conn->peerAddress().toIpPort();
        cout <<__FILE__<<"("<<__LINE__<<")"<< "client connected:" << conn->peerAddress().toIpPort();
        // ++ m_baseUserId;
        FileSessionPtr spSession(new FileSession(conn));
        conn->setMessageCallback(std::bind(&FileSession::OnRead, spSession.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        
        std::lock_guard<mutex> guard(m_sessionMutex);
        stringstream ss;
        ss << (void *)conn.get();
        m_sessions.insert(pair<string,FileSessionPtr>(ss.str(),spSession));
    }
    else
    {
        OnClose(conn);
    }
}

void FileServer::OnClose(const TcpConnectionPtr &conn)
{
    //TODO: 这样的代码逻辑太混乱，需要优化
    std::lock_guard<mutex> guard(m_sessionMutex);
    stringstream ss;
    ss << (void *)conn.get();
    auto it = m_sessions.find(ss.str());
    if(it != m_sessions.end())
    {
        m_sessions.erase(it);
    }
    cout <<__FILE__<<"("<<__LINE__<<")"<< "client disconnected:" << conn->peerAddress().toIpPort();
    // for (auto iter = m_sessions.begin(); iter != m_sessions.end();iter++)
    // {
    //     if((*iter)->GetConnectionPtr() == NULL)
    //     {
    //         //  cout <<__FILE__<<"("<<__LINE__<<")"
    //         LOG_ERROR<< "connection is NULL";
    //         break;
    //     }

    //     //用户下线
    //     m_sessions.erase(iter);
    //     //  cout <<__FILE__<<"("<<__LINE__<<")"
    //     LOG_INFO<< "client disconnected: " << conn->peerAddress().toIpPort();
    //     break;    
    // }
}
