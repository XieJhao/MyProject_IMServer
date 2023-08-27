#include"ClientSession.h"
#include<sstream>
#include "newJson/json/json.h"
// #include "jsoncpp-0.5.0/json.h"
#include "UserManager.h"
#include "MsgCacheManager.h"
#include "IMServer.h"
#include<string.h>

using namespace edoyun;

ClientSession::ClientSession(const TcpConnectionPtr& conn)
{
    m_seq = 0;
    // uuid_generate(m_sessionid);
    // 第一个()表示构造，第二个()表示运算符重载
    m_user.reset(new User);
    m_conn = conn;
    stringstream ss;
    ss << (void*)conn.get();
    // m_sessionid = to_string(random_generator()());
    m_sessionid = ss.str();
    TcpConnectionPtr *client = const_cast<TcpConnectionPtr *>(&conn);
    // *const_cast<string *>(&conn->name()) = m_sessionid;
    (*client)->setMessageCallback(std::bind(&ClientSession::OnRead,this,placeholders::_1,placeholders::_2,placeholders::_3));
}

ClientSession::~ClientSession()
{
    
}

void ClientSession::OnRead(const TcpConnectionPtr& conn,Buffer* buf,Timestamp time)
{
    //TODO:业务代码
    while (buf->readableBytes() >= sizeof(int32_t))
    {
        int32_t packagesize = 0;
        // BinaryReader::dump(buf->peek(),buf->readableBytes());
        packagesize = *(int32_t*)buf->peek();
        if(buf->readableBytes() < (sizeof(int32_t) + packagesize))
        {
            return;
        }
        buf->retrieve(sizeof(int32_t));
        string msgbuff;

        // BinaryReader::dump(buf->peek(),buf->readableBytes());
        msgbuff.assign(buf->peek() + 6, packagesize - 6);
        // BinaryReader::dump(msgbuff);
        // buf->retrieve(6);
        // cout << " 00 00 03 EA = " << htonl(*(int32_t *)buf->peek());
        buf->retrieve(packagesize);
        //TODO:处理消息
        // cout << "收到消息 ： " << msgbuff << endl;
        if (Process(conn, msgbuff) != true)
        {
            cout << "process error,close connect!" << endl;
            conn->forceClose();
        }
    }
    

}

void ClientSession::Send(BinaryWriter& write)
{
    if(m_conn == nullptr)
    {
        cout << __FILE__ << "(" << __LINE__ << ")"
             << "ClientSession::Send" << endl;
        return;
    }
    Send(m_conn, write);
}

bool ClientSession::Process(const TcpConnectionPtr &conn, string msgbuff)
{
    BinaryReader reader(msgbuff);
    int cmd = -1;
    //解码 获取请求类型
    if(reader.ReadData<decltype(cmd)>(cmd) == false)
    {
        return false;
    }
    if(reader.ReadData<int>(m_seq) == false)
    {
        return false;
    }
    // BinaryReader::dump(msgbuff);
    // ss >> m_seq;
    // size_t datalength = 0;
    // if(reader.ReadData<size_t>(datalength) == false)
    // {
    //     return false;
    // }
    string data;
    // data.resize(datalength);
    if(reader.ReadData(data) == false)
    {
        return false;
    }
    // cout << __FILE__ << "(" << __LINE__ << ")" 
    //         <<"cmd:"<<cmd<<" m_seq:"<<m_seq
    //         <<"  data.size:"<<data.size()
    //         <<"data: "<<data<< endl;

    switch (cmd)
    {
        case msg_type_headerbeart://心跳包
            OnHeartbeatResponse(conn, data);
            break;
        case msg_type_register: //注册消息
            OnRegisterResponse(conn, data);
            break;
        case msg_type_login: //登录消息
            OnLoginResponse(conn,data);
            break;
        case msg_type_getofriendlist://获取好友列表
            OnGetFriendListResponse(conn,data);
            break;
        case msg_type_finduser://查找用户
            OnFindUserResponse(conn,data);
            break;
        case msg_type_operatefriend: //操作好友
            OnOperateFriendResponse(conn,data);
            break;
        case msg_type_updateuserinfo://更新用户信息
            OnUpdateUserInfoResponse(conn,data);
            break;
        case msg_type_modifypassword://修改密码
            OnModifyPasswordResponse(conn,data);
            break;
        case msg_type_creategroup: //创建群
            OnCreateGroupResponse(conn,data);
            break;
        case msg_type_getgroupmembers: //获取群成员
            OnGetGroupMemberResponse(conn,data);
            break;
        case msg_type_chat: //聊天消息
            reader.ReadData<int>(m_target);
            cout << "m_target=" << m_target << endl;
            OnChatResponse(conn, data);
            break;
        case msg_type_multichat: //群发消息
            reader.ReadData(m_targets);
            
            OnMultiChatResponse(conn,data);
            break;
        
        default:
            break;
    }
    return true;
}

void ClientSession::OnHeartbeatResponse(const TcpConnectionPtr &conn,const string& data)
{
    //包的长度 4字节 不能压缩的，固定格式
    // 命令类型 4字节 不能压缩的，固定格式
    // 包的序号 4字节 不能压缩的，固定格式
    // 包的数据 (4字节 可以压缩)+包的内容(长度由前面一项来定)
    BinaryWriter write;
    int cmd = msg_type_headerbeart;
    write.WriteData(htonl(cmd));
    write.WriteData(htonl(m_seq));
    string empty;
    write.WriteData(empty);
    Send(conn,write);
    
    // string out = write.toString();
    // write.Clear();
    // cmd = (int)out.size();//获取包长度
    // write.WriteData<int>(cmd);
    // out = write.toString() + out;

    // if(conn != nullptr)
    // {
    //     conn->send(out.c_str(), out.size());
    // }
}

void ClientSession::OnRegisterResponse(const TcpConnectionPtr &conn,const string& data)
{
    //json数据包{"username":"手机号","nickname":"昵称","password":"密码"}
    Json::Reader reader;
    Json::Value root,response;
    BinaryWriter write;
    string result;
    int cmd = msg_type_register;
    write.WriteData<int>(htonl(cmd));
    write.WriteData<int>(htonl(m_seq));
    if(reader.parse(data,root) == false)
    {
        cout << "error json: " << data << endl;
        response["code"] = 101;
        response["msg"] = "json pares failed!";
        result = response.toStyledString();
        write.WriteData(result);
        Send(conn, write);
        return;
    }
    if(!root["username"].isString() || !root["nickname"].isString() ||!root["password"].isString())
    {
        cout << "error type : " << data << endl;
        response["code"] = 102;
        response["msg"] = "json data type error!";
        result = response.toStyledString();
        write.WriteData(result);
        Send(conn, write);
        return;
    }
    User user;
    user.username = root["username"].asString();
    user.nickname = root["nickname"].asString();
    user.password = root["password"].asString();
    
    

    if(!Singleton<UserManager>::instance().AddUser(user))
    {
        cout << "add user failed!" << endl;
        response["code"] = 100;
        response["msg"] = "register failed!";
        result = response.toStyledString();
        write.WriteData(result);
        Send(conn, write);
        return;
    }
    else
    {
        response["code"] = 0;
        response["msg"] = "ok";
        result = response.toStyledString();
        
        write.WriteData(result);
        Send(conn, write);
    }
    // 注册的本质就是在用户表中插入新的数据
}

void ClientSession::OnLoginResponse(const TcpConnectionPtr &conn, const string &data)
{
    //json消息包{"username":"用户","password":"密码","clienttype":1,"status":1}
    // cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    BinaryWriter write;
    string result;
    Json::Value root,response;
    //TODO:登录操作
    Json::Reader reader;
    int cmd = msg_type_login;
    write.WriteData(htonl(cmd));
    write.WriteData(htonl(m_seq));
    // 如果失败，返回错误
    // cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    if(reader.parse(data,root) == false)
    {
        response["code"] = 101;
        response["msg"] = "json pares failed!";
        result = response.toStyledString();
        write.WriteData(result);
        Send(conn, write);
        return;
    }
    if(!root["username"].isString() || !root["password"].isString() || !root["clienttype"].isInt() || !root["status"].isInt())
    {
        response["code"] = 102;
        response["msg"] = "json data type error!";
        result = response.toStyledString();
        write.WriteData(result);
        Send(conn, write);
        return;
    }
    // cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    string username = root["username"].asString();
    // cout << __FILE__ << "(" << __LINE__ << ") username:" << username <<endl;
    string password = root["password"].asString();
    User user;
    // cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    if(Singleton<UserManager>::instance().GetUserInfoByUsername(username,user) == false)
    {
        // cout << __FILE__ << "(" << __LINE__ << ")" << endl;
        response["code"] = 103;
        response["msg"] = "user is not exist or password is incorrest!";
        result = response.toStyledString();
        write.WriteData(result);
        Send(conn, write);
        return;
    }
    // cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    if(password != user.password)
    {
        response["code"] = 104;
        response["msg"] = "user is not exist or password is incorrest!";
        result = response.toStyledString();
        write.WriteData(result);
        Send(conn, write);
        return;
    }

    //如果成功则应答
    m_user->userid = user.userid;
    m_user->username = username;
    m_user->nickname = user.nickname;
    m_user->password = password;
    m_user->clienttype = root["clienttype"].asInt();
    // cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    response["code"] = 0;
    response["msg"] = "ok";
    response["userid"] = user.userid;
    response["nickname"] = user.nickname;
    response["facetype"] = user.facetype;
    response["customface"] = user.custimface;
    response["gender"] = user.gender;
    response["birthday"] = user.birthday;
    response["signature"] = user.signature;
    response["address"] = user.address;
    response["phonenumber"] = user.phonenumber;
    response["mail"] = user.mail;
    response["username"] = username;
    result = response.toStyledString();
    m_user->status = 1;//状态切换为线上
    write.WriteData(result);
    // cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    Send(conn, write);
    // cout << "send():write: = " << write.toString() << endl;

    //推送通知消息
    list<NotifyMsgCache> listNotifyCache;
    Singleton<MsgCacheManager>::instance().GetNotifyMsgCache(m_user->userid, listNotifyCache);
    for(const auto& iter : listNotifyCache)
    {
        // write = iter.notifymsg;
        write.Clear();
        write.WriteData(iter.notifymsg);

        Send(write);
        BinaryReader::dump(write.toString());
        cout << __FILE__ << "(" << __LINE__ << ")" << endl;
        cout << "推送通知消息" << endl;
    }

    //推送聊天消息
    list<ChatMsgCache> listChatCache;
    Singleton<MsgCacheManager>::instance().GetChatMagCache(m_user->userid, listChatCache);
    for(const auto& iter : listChatCache)
    {
        // write = iter.chatmsg;
        write.Clear();
        write.WriteData(iter.chatmsg);
        Send(write);
        BinaryReader::dump(write.toString());
        cout << __FILE__ << "(" << __LINE__ << ")" << endl;
        cout << "推送聊天消息" << endl;
    }

    //给其他用户推送上线消息
    list<User> friends;
    Singleton<UserManager>::instance().GetFriendInfoByUserId(m_user->userid, friends);
    IMServer &imserver = Singleton<IMServer>::instance();
    for(const auto& iter : friends)
    {
        //先看用户是否在线
        ClientSessionPtr targetSession = imserver.GetSessionByID(iter.userid);

        if(targetSession)
        {
            // cout << __FILE__ << "(" << __LINE__ << ")" 
            //      <<"OnLoginResponse userid="<<m_user->userid<<"  target="
            //      <<targetSession->UserID()<< endl;

            targetSession->SendUserStatusChangeMsg(m_user->userid, 1);
            cout << __FILE__ << "(" << __LINE__ << ")" << endl;
            cout << "给其他用户推送上线消息" << endl;
        }
    }
}

void ClientSession::OnGetFriendListResponse(const TcpConnectionPtr &conn, const string &data)
{
    // cout << __FILE__ << "(" << __LINE__ << ") OnGetFriendListResponse 获取好友列表" << endl;
    list<User> friends;
    Json::Value response;
    if(!Singleton<UserManager>::instance().GetFriendInfoByUserId(m_user->userid, friends))
    {
        cout << " the  GetFriendInfoByUserId is error" << endl;
    }
    if(friends.size() == 0)
    {
        cout << "无法获取好友列表" << endl;
    }

    string strUserInfo;
    bool userOnline = false;
    IMServer &imserver = Singleton<IMServer>::instance();
    response["code"] = 0;
    response["msg"] = "ok";
    for(const auto& iter : friends)
    {
        userOnline = imserver.IsUserSessionExsit(iter.userid);
        /*
        {"code": 0, "msg": "ok", "userinfo":[{"userid": 1,"username":"qqq, 
        "nickname":"qqq, "facetype": 0, "customface":"", "gender":0, "birthday":19900101, 
        "signature":", "address": "", "phonenumber": "", "mail":", "clienttype": 1, "status":1"]}
        */
        Json::Value re;

        re["userid"] = iter.userid;
        re["username"] = iter.username;
        re["nickname"] = iter.nickname;
        re["facetype"] = iter.facetype;
        re["customface"] = iter.custimface;
        re["gender"] = iter.gender;
        re["birthday"] = iter.birthday;
        re["signature"] = iter.signature;
        re["address"] = iter.address;
        re["phonenumber"] = iter.phonenumber;
        re["mail"] = iter.mail;
        re["clienttype"] = 1;
        re["status"] = (userOnline ? 1 : 0);

        response["userinfo"].append(re);
    }
    string result;
    BinaryWriter writer;
    writer.WriteData(htonl(msg_type_getofriendlist));
    writer.WriteData(htonl(m_seq));
    result = response.toStyledString();
    writer.WriteData(result);

    LOG_INFO << "Response to client: cmd=msg_type_getofriendlist, data=" << result<< ", userid=" << m_user->userid;

    Send(writer);
}
void ClientSession::OnFindUserResponse(const TcpConnectionPtr &conn, const string &data)
{
    cout << "查找用户 （OnFindUserResponse）" << endl;
    cout << "data = " << data << endl;
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if(!JsonReader.parse(data,JsonRoot))
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        cout << __FILE__ << "(" << __LINE__ << ") OnFindUserResponse" << endl;
        
        return;
    }

    if(!JsonRoot["type"].isInt() || !JsonRoot["username"].isString())
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        cout << __FILE__ << "(" << __LINE__ << ") OnFindUserResponse" << endl;
        return;
    }

    //TODO:目前只支持查找单个用户
    string username = JsonRoot["username"].asString();
    cout << __FILE__ << "(" << __LINE__ << ")  username = "<<username << endl;
    JsonRoot.clear();
    User cachedUser;
    if(!Singleton<UserManager>::instance().GetUserInfoByUsername(username,cachedUser))
    {
        JsonRoot["code"] = 0;
        JsonRoot["msg"] = "ok";
        JsonRoot["userinfo"] = "[]";
    }
    else
    {
        JsonRoot["code"] = 0;
        JsonRoot["msg"] = "ok";
        Json::Value re;
        re["userid"] = cachedUser.userid;
        re["username"] = cachedUser.username;
        re["nickname"] = cachedUser.nickname;
        re["facetype"] = cachedUser.facetype;
        JsonRoot["userinfo"].append(re);
    }

    string result;
    BinaryWriter writer;
    writer.WriteData(htonl(msg_type_finduser));
    writer.WriteData(htonl(m_seq));
    result = JsonRoot.toStyledString();
    writer.WriteData(result);

    LOG_INFO << "Response to client: cmd=msg_type_finduser, data=" << result << ", userid=" << m_user->userid;

    Send(writer);
}
void ClientSession::OnOperateFriendResponse(const TcpConnectionPtr &conn, const string &data)
{
    // cout << __FILE__ << "(" << __LINE__ << ") OnOperateFriendResponse" << endl;
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(data, JsonRoot))
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        // cout << __FILE__ << "(" << __LINE__ << ") OnOperateFriendResponse" << endl;
        return;
    }
    // cout << __FILE__ << "(" << __LINE__ << ")解析成功  JsonRoot.string = " << JsonRoot.toStyledString() << endl;
    if (!JsonRoot["type"].isInt() || !JsonRoot["userid"].isInt())
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        // cout << __FILE__ << "(" << __LINE__ << ") OnOperateFriendResponse" << endl;
        return;
    }

    int type = JsonRoot["type"].asInt();
    int32_t targetUserid = JsonRoot["userid"].asInt();
    if (targetUserid >= GROUPID_BOUBDARY)
    {
        if (type == 4)
        {
            //退群
            DeleteFriend(conn, targetUserid);
            return;
        }

        //加群直接同意
        OnAddGroupResponse(conn, targetUserid);
        return;
    }

    Json::Value root;
    //删除好友
    if (type == 4)
    {
        DeleteFriend(conn, targetUserid);
        return;
    }
    //发出加好友申请
    if(type == 1)
    {
        //{"userid": 9, "type": 1, }
        root["userid"] = m_user->userid;
        root["type"] = 2;
        root["username"] = m_user->username;
    }
    else if (type == 3) //答应添加好友
    {
        if(!JsonRoot["accept"].isInt())
        {
            LOG_WARN << "invalid json: " << data << ", userid: " << m_user->userid << "client: " << conn->peerAddress().toIpPort();
            // cout << __FILE__ << "(" << __LINE__ << ") OnOperateFriendResponse" << endl;
            return;
        }

        int _accept = JsonRoot["accept"].asInt();
        //接受加好友申请后，建立好友关系
        if(_accept == 1)
        {
            int smallid = m_user->userid;
            int greatid = targetUserid;
            //数据库里面互为好友的两个id，小者在先，大者在后
            if (smallid > greatid)
            {
                smallid = targetUserid;
                greatid = m_user->userid;
            }
            // cout << __FILE__ << "(" << __LINE__ << ")  即将进入MakeFriendRelationship函数  smallid="<<smallid<<"  greatid=" <<greatid<< endl;
            if (!Singleton<UserManager>::instance().MakeFriendRelationship(smallid, greatid))
            {
                LOG_ERROR << "make relationship error: " << data << ", userid: " << m_user->userid << "client: " << conn->peerAddress().toIpPort();
                // cout << __FILE__ << "(" << __LINE__ << ") OnOperateFriendResponse" << endl;
                return;
            }
        }

        //{ "userid": 9, "type" : 3, "userid" : 9, "username" : "xxx", "accept" : 1 }
        root["userid"] = m_user->userid;
        root["type"] = 3;
        root["username"] = m_user->username;
        root["accept"] = _accept;

        //提示自己当前用户加好友成功
        User targetUser;
        if(!Singleton<UserManager>::instance().GetUserInfoByUserId(targetUserid,targetUser))
        {
            LOG_ERROR << "Get Userinfo by id error, targetuserid: " << targetUserid << ", userid: " << m_user->userid << ", data: "<< data << ", client: " << conn->peerAddress().toIpPort();
            // cout << __FILE__ << "(" << __LINE__ << ") OnOperateFriendResponse" << endl;
            return;
        }

        Json::Value re;
        re["userid"] = targetUser.userid;
        re["type"] = 3;
        re["username"] = targetUser.username;
        re["accept"] = _accept;
        string outbufx;
        outbufx = re.toStyledString();
        BinaryWriter writer;
        writer.WriteData(htonl(msg_type_operatefriend));
        writer.WriteData(htonl(m_seq));
        writer.WriteData(outbufx);

        Send(writer);
        LOG_INFO << "Response to client: cmd=msg_type_addfriend, data=" << outbufx << ", userid=" << m_user->userid;
        // cout << __FILE__ << "(" << __LINE__ << ") OnOperateFriendResponse" << endl;
    }

    //提示对方加好友成功
    string outbuf;
    outbuf = root.toStyledString();
    BinaryWriter writer;
    writer.WriteData(htonl(msg_type_operatefriend));
    writer.WriteData(htonl(m_seq));
    writer.WriteData(outbuf);

    //先看目标用户是否在线
    ClientSessionPtr targetSession;
    targetSession = Singleton<IMServer>::instance().GetSessionByID(targetUserid);
    //目标用户不在线，缓存这个消息
    if (!targetSession)
    {
        Singleton<MsgCacheManager>::instance().AddNotifyMsgCache(targetUserid, outbuf);
        LOG_INFO << "userid: " << targetUserid << " is not online, cache notify msg, msg: " << outbuf;
        // cout << __FILE__ << "(" << __LINE__ << ") OnOperateFriendResponse" << endl;
        return;
    }

    targetSession->Send(writer);
    LOG_INFO << "Response to client: cmd=msg_type_addfriend, data=" << data << ", userid=" << targetUserid;
    // cout << __FILE__ << "(" << __LINE__ << ") OnOperateFriendResponse" << endl;
}
void ClientSession::OnUpdateUserInfoResponse(const TcpConnectionPtr &conn, const string &data)
{
    // cout << __FILE__ << "(" << __LINE__ << ") OnUpdateUserInfoResponse" << endl;
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(data, JsonRoot))
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if (!JsonRoot["nickname"].isString() || !JsonRoot["facetype"].isInt() || 
        !JsonRoot["customface"].isString() || !JsonRoot["gender"].isInt() || 
        !JsonRoot["birthday"].isInt() || !JsonRoot["signature"].isString() || 
        !JsonRoot["address"].isString() || !JsonRoot["phonenumber"].isString() || 
        !JsonRoot["mail"].isString())
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    User newuserinfo;
    newuserinfo.nickname = JsonRoot["nickname"].asString();
    newuserinfo.facetype = JsonRoot["facetype"].asInt();
    newuserinfo.custimface = JsonRoot["customface"].asString();
    newuserinfo.gender = JsonRoot["gender"].asInt();
    newuserinfo.birthday = JsonRoot["birthday"].asInt();
    newuserinfo.signature = JsonRoot["signature"].asString();
    newuserinfo.address = JsonRoot["address"].asString();
    newuserinfo.phonenumber = JsonRoot["phonenumber"].asString();
    newuserinfo.mail = JsonRoot["mail"].asString();

    JsonRoot.clear();
    if(!Singleton<UserManager>::instance().UpdateUserInfo(m_user->userid,newuserinfo))
    {
        JsonRoot["code"] = 104;
        JsonRoot["msg"] = "update user info failed";
        // cout << __FILE__ << "(" << __LINE__ << ") OnUpdateUserInfoResponse" <<" the func UpdateUserInfo is error"<< endl;
    }
    else
    {
        /*
        { "code": 0, "msg" : "ok", "userid" : 2, "username" : "xxxx", 
         "nickname":"zzz", "facetype" : 26, "customface" : "", "gender" : 0, "birthday" : 19900101, 
         "signature" : "xxxx", "address": "", "phonenumber": "", "mail":""}
        */
        JsonRoot["code"] = 0;
        JsonRoot["msg"] = "ok";
        JsonRoot["userid"] = m_user->userid;
        JsonRoot["username"] = m_user->username;
        JsonRoot["nickname"] = newuserinfo.nickname;
        JsonRoot["facetype"] = newuserinfo.facetype;
        JsonRoot["customface"] = newuserinfo.custimface;
        JsonRoot["gender"] = newuserinfo.gender;
        JsonRoot["birthday"] = newuserinfo.birthday;
        JsonRoot["signature"] = newuserinfo.signature;
        JsonRoot["address"] = newuserinfo.address;
        JsonRoot["phonenumber"] = newuserinfo.phonenumber;
        JsonRoot["mail"] = newuserinfo.mail;
    }

    string outbuf;
    outbuf = JsonRoot.toStyledString();
    BinaryWriter writer;
    writer.WriteData(htonl(msg_type_updateuserinfo));
    writer.WriteData(htonl(m_seq));
    writer.WriteData(outbuf);

    //应答客户端
    Send(writer);

    LOG_INFO << "Response to client: cmd=msg_type_updateuserinfo, data=" << outbuf << ", userid=" << m_user->userid;
    // cout << __FILE__ << "(" << __LINE__ << ") OnUpdateUserInfoResponse" <<" 信息修改发送成功"<< endl;

    //给其他在线好友推送个人信息发生改变消息
    std::list<User> friends;
    Singleton<UserManager>::instance().GetFriendInfoByUserId(m_user->userid, friends);
    IMServer& imserver = Singleton<IMServer>::instance();
    for (const auto& iter : friends)
    {
        //先看目标用户是否在线
        ClientSessionPtr targetSession;
        targetSession = imserver.GetSessionByID( iter.userid);
        if (targetSession)
            targetSession->SendUserStatusChangeMsg(m_user->userid, 3);
    }
}
void ClientSession::OnModifyPasswordResponse(const TcpConnectionPtr &conn, const string &data)
{
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(data, JsonRoot))
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if (!JsonRoot["oldpassword"].isString() || !JsonRoot["newpassword"].isString())
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    string oldpass = JsonRoot["oldpassword"].asString();
    string newPass = JsonRoot["newpassword"].asString();
    JsonRoot.clear();

    User cachedUser;
    if(!Singleton<UserManager>::instance().GetUserInfoByUserId(m_user->userid,cachedUser))
    {
        LOG_ERROR << "get userinfo error, userid: " << m_user->userid << ", data: " << data << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if(cachedUser.password != oldpass)
    {
        JsonRoot["code"] = 103;
        JsonRoot["msg"] = "incorrect old password";
        cout << __FILE__ << "(" << __LINE__ << ") (OnModifyPasswordResponse) incorrect old password" << endl;
    }
    else
    {
        if(!Singleton<UserManager>::instance().ModifyUserPassword(m_user->userid,newPass))
        {
            JsonRoot["code"] = 105;
            JsonRoot["msg"] = "modify password error";
            LOG_ERROR << "modify password error, userid: " << m_user->userid << ", data: " << data << ", client: " << conn->peerAddress().toIpPort();
            cout << __FILE__ << "(" << __LINE__ << ") (OnModifyPasswordResponse) modify password error" << endl;
        }
        else
        {
            JsonRoot["code"] = 0;
            JsonRoot["msg"] = "ok";
        }
    }

    string outbuf;
    outbuf = JsonRoot.toStyledString();
    BinaryWriter writer;
    writer.WriteData(htonl(msg_type_modifypassword));
    writer.WriteData(htonl(m_seq));
    writer.WriteData(outbuf);

    //应答客户端
    Send(writer);

    LOG_INFO << "Response to client: cmd=msg_type_modifypassword, data=" << data << ", userid=" << m_user->userid;
}

void ClientSession::OnCreateGroupResponse(const TcpConnectionPtr &conn, const string &data)
{
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(data, JsonRoot))
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if (!JsonRoot["groupname"].isString())
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    string groupname = JsonRoot["groupname"].asString();

    JsonRoot.clear();
    int32_t groupid;
    if(!Singleton<UserManager>::instance().AddGroup(groupname.c_str(),m_user->userid,groupid))
    {
        LOG_WARN << "Add group error, data: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        JsonRoot["code"] = 106;
        JsonRoot["msg"] = "create group error";
    }
    else
    {
        JsonRoot["code"] = 0;
        JsonRoot["msg"] = "ok";
        JsonRoot["groupid"] = groupid;
        JsonRoot["groupname"] = groupname;
    }

    //创建成功以后该用户自动加群
    if(!Singleton<UserManager>::instance().MakeFriendRelationship(m_user->userid,groupid))
    {
        LOG_ERROR << "join in group, errordata: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    string outbuf;
    outbuf = JsonRoot.toStyledString();
    BinaryWriter writer;
    writer.WriteData(htonl(msg_type_creategroup));
    writer.WriteData(htonl(m_seq));
    writer.WriteData(outbuf);

    //应答客户端，建群成功
    Send(writer);

    LOG_INFO << "Response to client: cmd=msg_type_creategroup, data=" << outbuf << ", userid=" << m_user->userid;

    //应答客户端，成功加群
    {
        Json::Value re;
        re["userid"] = groupid;
        re["type"] = 3;
        re["username"] = groupname;
        string outbufx;
        outbufx = re.toStyledString();
        writer.Clear();
        writer.WriteData(htonl(msg_type_operatefriend));
        writer.WriteData(htonl(m_seq));
        writer.WriteData(outbufx);

        Send(writer);
        LOG_INFO << "Response to client: cmd=msg_type_addfriend, data=" << outbufx << ", userid=" << m_user->userid;
    }
}

void ClientSession::OnGetGroupMemberResponse(const TcpConnectionPtr &conn, const string &data)
{
    //{"groupid": 群id}
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(data, JsonRoot))
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if (!JsonRoot["groupid"].isInt())
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    int32_t groupid = JsonRoot["groupid"].asInt();
    JsonRoot.clear();

    JsonRoot["code"] = 0;
    JsonRoot["msg"] = "ok";
    JsonRoot["groupid"] = groupid;

    list<User> friends;
    Singleton<UserManager>::instance().GetFriendInfoByUserId(groupid, friends);
    bool userOnline = false;
    IMServer &imserver = Singleton<IMServer>::instance();
    for(const auto& iter : friends)
    {
        userOnline = imserver.IsUserSessionExsit(iter.userid);
        /*
        {"code": 0, "msg": "ok", "members":[{"userid": 1,"username":"qqq,
        "nickname":"qqq, "facetype": 0, "customface":"", "gender":0, "birthday":19900101,
        "signature":", "address": "", "phonenumber": "", "mail":", "clienttype": 1, "status":1"]}
        */
        Json::Value re;
        re["userid"] = iter.userid;
        re["username"] = iter.username;
        re["nickname"] = iter.nickname;
        re["facetype"] = iter.facetype;
        re["customface"] = iter.custimface;
        re["gender"] = iter.gender;
        re["birthday"] = iter.birthday;
        re["signature"] = iter.signature;
        re["phonenumber"] = iter.phonenumber;
        re["mail"] = iter.mail;
        re["clienttype"] = 1;
        re["status"] = (userOnline ? 1 : 0);

        JsonRoot["members"].append(re);
    }

    string outbuf;
    outbuf = JsonRoot.toStyledString();

    BinaryWriter writer;
    writer.WriteData(htonl(msg_type_getgroupmembers));
    writer.WriteData(htonl(m_seq));
    writer.WriteData(outbuf);

    LOG_INFO << "Response to client: cmd=msg_type_getgroupmembers, data=" << outbuf << ", userid=" << m_user->userid;
    Send(writer);
}

void ClientSession::OnChatResponse(const TcpConnectionPtr &conn, const string &data)
{
    BinaryWriter writer;
    writer.WriteData(htonl(msg_type_chat));
    writer.WriteData(htonl(m_seq));
    writer.WriteData(data);
    //消息发送者
    writer.WriteData(htonl(m_user->userid));
    // 消息接收者
    writer.WriteData(htonl(m_target));

    // BinaryReader::dump(data);
    // cout << __FILE__ << "(" << __LINE__ << ")OnChatResponse  userid=" << m_user->userid << "  m_target=" << m_target << endl;
    // BinaryReader::dump(writer.toString());

    UserManager &userMgr = Singleton<UserManager>::instance();
    //写入消息记录
    if(!userMgr.SaveChatMsgToDb(m_user->userid,m_target,data))
    {
        LOG_ERROR << "Write chat msg to db error, , senderid = " << m_user->userid << ", targetid = " << m_target << ", chatmsg:" << data;
    }

    IMServer& imserver = Singleton<IMServer>::instance();
    MsgCacheManager& msgCacheMgr = Singleton<MsgCacheManager>::instance();
    //单聊消息
    if (m_target < GROUPID_BOUBDARY)
    {
        //先看目标用户是否在线
        ClientSessionPtr targetSession;
        targetSession = imserver.GetSessionByID(m_target);
        //目标用户不在线，缓存这个消息
        if (!targetSession)
        {
            msgCacheMgr.AddChatMsgCache(m_target, writer.toString());
            return;
        }

        targetSession->Send(writer);
        // cout << __FILE__ << "(" << __LINE__ << ") 单聊消息已发送m_target="<<m_target<<"  userid="<<m_user->userid << endl;
        return;
    }

    //群聊消息
    std::list<User> friends;
    userMgr.GetFriendInfoByUserId(m_target, friends);
    std::string strUserInfo;
    bool userOnline = false;
    for (const auto& iter : friends)
    {
        //先看目标用户是否在线
        ClientSessionPtr targetSession;
        targetSession = imserver.GetSessionByID(iter.userid);
        //目标用户不在线，缓存这个消息
        if (!targetSession)
        {
            msgCacheMgr.AddChatMsgCache(iter.userid, writer.toString());
           continue;
        }

        targetSession->Send(writer);
    }
}
void ClientSession::OnMultiChatResponse(const TcpConnectionPtr &conn, const string &data)
{
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(m_targets, JsonRoot))
    {
        LOG_ERROR << "invalid json: targets: " << m_targets  << "data: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if (!JsonRoot["targets"].isArray())
    {
        LOG_ERROR << "invalid json: targets: " << m_targets << "data: " << data << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    for (uint32_t i = 0; i < JsonRoot["targets"].size(); ++i)
    {
        m_target = JsonRoot["targets"][i].asInt();
        OnChatResponse(conn, data);
    }

    LOG_INFO << "Send to client: cmd=msg_type_multichat, targets: " << m_targets << "data : " << data << ", userid : " << m_user->userid << ", client : " << conn->peerAddress().toIpPort();
}

void ClientSession::DeleteFriend(const TcpConnectionPtr&conn,int32_t friendid)
{
    int32_t smallerid = friendid;
    int32_t greaterid = m_user->userid;
    if (smallerid > greaterid)
    {
        smallerid = m_user->userid;
        greaterid = friendid;
    }

    if (!Singleton<UserManager>::instance().ReleaseFriendRelationship(smallerid, greaterid))
    {
        LOG_ERROR << "Delete friend error, friendid: " << friendid << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    User cachedUser;
    if (!Singleton<UserManager>::instance().GetUserInfoByUserId(friendid, cachedUser))
    {
        LOG_ERROR << "Delete friend - Get user error, friendid: " << friendid << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    Json::Value root;
    //发给主动删除的一方
    //{"userid": 9, "type": 1, }
    root["userid"] = friendid;
    root["type"] = 5;
    root["username"] = cachedUser.username;

    string outbuf;
    outbuf = root.toStyledString();
    BinaryWriter writer;
    writer.WriteData(htonl(msg_type_operatefriend));
    writer.WriteData(htonl(m_seq));
    writer.WriteData(outbuf);

    Send(writer);
    LOG_INFO << "Send to client: cmd=msg_type_operatefriend, data=" << outbuf << ", userid=" << m_user->userid;

    //发给被删除的一方
    //删除好友消息
    if(friendid < GROUPID_BOUBDARY)
    {
        outbuf.clear();
        //判断目标用户是否在线
        ClientSessionPtr targetSession;
        targetSession = Singleton<IMServer>::instance().GetSessionByID(friendid);
        //仅给在线用户推送这个消息
        if(targetSession)
        {
            root.clear();
            root["userid"] = m_user->userid;
            root["type"] = 5;
            root["username"] = m_user->username;
            writer.Clear();
            writer.WriteData(htonl(msg_type_operatefriend));
            writer.WriteData(htonl(m_seq));
            outbuf = root.toStyledString();
            writer.WriteData(outbuf);

            targetSession->Send(writer);
            LOG_INFO << "Send to client: cmd=msg_type_operatefriend, data=" << outbuf << ", userid=" << friendid;
        }

        return;
    }

    //退群消息
    //给其他在线群成员推送群信息发生变化的消息
    std::list<User> friends;
    Singleton<UserManager>::instance().GetFriendInfoByUserId(friendid, friends);
    IMServer& imserver = Singleton<IMServer>::instance();
    for (const auto& iter : friends)
    {
        //先看目标用户是否在线
        ClientSessionPtr targetSession;
        targetSession = imserver.GetSessionByID(iter.userid);
        if (targetSession)
            targetSession->SendUserStatusChangeMsg(friendid, 3);
    }
}
void ClientSession::OnAddGroupResponse(const TcpConnectionPtr &conn, int32_t groupid)
{
    if (!Singleton<UserManager>::instance().MakeFriendRelationship(m_user->userid, groupid))
    {
        LOG_ERROR << "make relationship error, groupId: " << groupid << ", userid: " << m_user->userid << "client: " << conn->peerAddress().toIpPort();
        cout << __FILE__ << "(" << __LINE__ << ") OnAddGroupResponse" << endl;
        return;
    }
    
    User groupUser;
    if (!Singleton<UserManager>::instance().GetUserInfoByUserId(groupid, groupUser))
    {
        LOG_ERROR << "Get group info by id error, targetuserid: " << groupid << ", userid: " << m_user->userid << ", client: " << conn->peerAddress().toIpPort();
        cout << __FILE__ << "(" << __LINE__ << ") OnAddGroupResponse" << endl;
        return;
    }

    Json::Value root;
    root["userid"] = groupUser.userid;
    root["type"] = 3;
    root["username"] = groupUser.username;
    root["accept"] = 3;
    string outbufx;
    outbufx = root.toStyledString();
    BinaryWriter writer;
    writer.WriteData(htonl(msg_type_operatefriend));
    writer.WriteData(htonl(m_seq));
    writer.WriteData(outbufx);

    Send(writer);
    LOG_INFO << "Response to client: cmd=msg_type_addfriend, data=" << outbufx << ", userid=" << m_user->userid;
    cout << __FILE__ << "(" << __LINE__ << ") OnAddGroupResponse" << endl;

    //给其他在线群成员推送群信息发生变化的消息
    std::list<User> friends;
    Singleton<UserManager>::instance().GetFriendInfoByUserId(groupid, friends);
    IMServer& imserver = Singleton<IMServer>::instance();
    for (const auto& iter : friends)
    {
        //先看目标用户是否在线
        ClientSessionPtr targetSession;
        targetSession = imserver.GetSessionByID(iter.userid);
        if (targetSession)
            targetSession->SendUserStatusChangeMsg(groupid, 3);
    }
}
void ClientSession::SendUserStatusChangeMsg(int32_t userid, int type)
{
    Json::Value data;
    //用户上线
    if(type == 1)
    {
        data["type"] = 1;
        data["onlinestatus"] = 1;
    }
    else if (type == 2) //用户下线
    {
        data["type"] = 2;
        data["onlinestatus"] = 0;
    }
    else if (type == 3) //个人昵称、头像、签名等信息更改
    {
        data["type"] = 3;
    }

    string outbuf;
    BinaryWriter writer;
    outbuf = data.toStyledString();
    writer.WriteData(htonl(msg_type_userstatuschange));
    writer.WriteData(htonl(m_seq));
    writer.WriteData(outbuf);
    writer.WriteData(htonl(userid));

    Send(writer);

    LOG_INFO << "Send to client: cmd=msg_type_userstatuschange, data=" 
    << outbuf << ", userid=" << m_user->userid;
}
