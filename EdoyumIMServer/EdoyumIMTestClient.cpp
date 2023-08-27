#include"base/Logging.h"
#include"net/TcpClient.h"
#include"net/EventLoop.h"
#include"net/InetAddress.h"
#include<iostream>

using namespace std;
using namespace muduo;
using namespace muduo::net;
TcpClient *pclient = NULL;
EventLoop *ploop = nullptr;

void onClose(const TcpConnectionPtr &conn)
{
    cout << "onClose called!" << endl;
    // ploop->quit();
    exit(0);
}

void onConnected(const TcpConnectionPtr &conn)
{
    conn->send("holle");
    // conn->setCloseCallback(onClose);
}

void onMessage(const TcpConnectionPtr& conn,
                Buffer* buffer,Timestamp ts)
{
    cout << buffer->retrieveAllAsString() << endl;
    conn->setCloseCallback(onClose);
    // conn->shutdown();
    // pclient->stop();
    // conn->getLoop()->queueInLoop(onClose);
}

int main(int argc, char const *argv[])
{
    EventLoop loop;
    ploop = &loop;
    InetAddress servAddr("127.0.01", 9527);
    TcpClient client(&loop, servAddr, "echo client");
    pclient = &client;
    client.setConnectionCallback(onConnected);
    client.setMessageCallback(onMessage);
    client.connect();
    
    loop.loop();
    return 0;
}
