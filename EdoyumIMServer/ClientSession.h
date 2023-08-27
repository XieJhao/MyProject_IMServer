#pragma once

#include<iostream>
#include "net/EventLoop.h"
#include "net/EventLoopThread.h"
#include "net/EventLoopThreadPool.h"
#include "net/TcpServer.h"
#include "base/Logging.h"
#include <boost/uuid/uuid_io.hpp>
#include<boost/uuid/uuid.hpp>
#include<boost/uuid/uuid_generators.hpp>
#include "BinaryReader.h"
#include "UserManager.h"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace boost::uuids;
using namespace edoyun;

enum
{
    msg_type_unknown,
    //用户消息
    msg_type_headerbeart = 1000,
    msg_type_register,
    msg_type_login,
    msg_type_getofriendlist,
    msg_type_finduser,
    msg_type_operatefriend,
    msg_type_userstatuschange,
    msg_type_updateuserinfo,
    msg_type_modifypassword,
    msg_type_creategroup,
    msg_type_getgroupmembers,
    //聊天消息
    msg_type_chat = 1100, //单聊消息
    msg_type_multichat   //群发消息

};

class TcpSession
{
public:
    TcpSession() = default;
    ~TcpSession() = default;
    void Send(const TcpConnectionPtr &conn, BinaryWriter &write)
    {
        string out = write.toString();
        write.Clear();
        int len = (int)out.size();//获取包长度
        write.WriteData<int>(len + 6);
        write.WriteData(htonl(len));
        write.WriteData(htons(0));
        out = write.toString() + out;

        if(conn != nullptr)
        {
            // BinaryReader::dump(out);
            // cout<< __FILE__ << "(" << __LINE__ << ")"<<endl;
            conn->send(out.c_str(), out.size());
        }
    }
};

class ClientSession : public TcpSession
{

public:
    ClientSession(const TcpConnectionPtr& conn);
    //为了控制生命周期，防止提前销毁，或重复销毁
    ClientSession(const ClientSession& ) = delete;
    ClientSession &operator=(const ClientSession &) = delete;
    ~ClientSession();

    operator string()
    {
        return m_sessionid;
    }

    void OnRead(const TcpConnectionPtr& conn,Buffer* buf,Timestamp time);
    using TcpSession::Send;
    void Send(BinaryWriter& write);
    
    int32_t UserID()const
    {
        return (m_user != nullptr) ? m_user->userid : -1;
    }

    //业务函数
    bool Process(const TcpConnectionPtr &conn, string msgbuff);
protected:
    void OnHeartbeatResponse(const TcpConnectionPtr &conn,const string& data);
    void OnRegisterResponse(const TcpConnectionPtr &conn,const string& data);
    void OnLoginResponse(const TcpConnectionPtr &conn, const string &data);
    void OnGetFriendListResponse(const TcpConnectionPtr &conn, const string &data);
    void OnFindUserResponse(const TcpConnectionPtr &conn, const string &data);
    void OnOperateFriendResponse(const TcpConnectionPtr &conn, const string &data);
    void OnUpdateUserInfoResponse(const TcpConnectionPtr &conn, const string &data);
    void OnModifyPasswordResponse(const TcpConnectionPtr &conn, const string &data);
    void OnCreateGroupResponse(const TcpConnectionPtr &conn, const string &data);
    void OnGetGroupMemberResponse(const TcpConnectionPtr &conn, const string &data);
    void OnChatResponse(const TcpConnectionPtr &conn, const string &data);
    void OnMultiChatResponse(const TcpConnectionPtr &conn, const string &data);

    void DeleteFriend(const TcpConnectionPtr&conn,int32_t friendid);
    void OnAddGroupResponse(const TcpConnectionPtr &conn, int32_t groupid);
    void SendUserStatusChangeMsg(int32_t userid, int type);

private:
    string m_sessionid;
    int m_seq;//会话的序号
    UserPtr m_user;
    int32_t m_target;//会话消息的时候使用
    string m_targets;//群聊消息处理的时候使用
    TcpConnectionPtr m_conn;
};

typedef shared_ptr<ClientSession> ClientSessionPtr;
