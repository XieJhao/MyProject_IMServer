#include"TcpSession.h"
#include"FileMsg.h"
// #include"../base/Logging.h"

TcpSession::TcpSession(TcpConnectionPtr& conn)
: tmpConn_(conn)
{
    
}

void TcpSession::Send(const TcpConnectionPtr &conn, BinaryWriter &write)
{
    string out = write.toString();
    write.Clear();
    int64_t len = (int64_t)out.size();//获取包长度
    write.WriteData(len + 6);
    write.WriteData(htonl(len));
    write.WriteData(htons(0));
    out = write.toString() + out;

    if(conn != nullptr)
    {
        // BinaryReader::dump(out);
        cout<< __FILE__ << "(" << __LINE__ << ") conn.port= "<<conn->peerAddress().toIpPort()<<"  conn.str"<<string((char*)(void*)conn.get())<<endl;
        conn->send(out.c_str(), out.size());
    }
}

void TcpSession::Send(BinaryWriter& write)
{
    TcpConnectionPtr conn = tmpConn_.lock();
    if (conn == nullptr)
    {
        cout << __FILE__ << "(" << __LINE__ << ")"
                << "ClientSession::Send" << endl;
        return;
    }
    Send(conn, write);
}

TcpSession::~TcpSession()
{
    
}
