
#pragma once

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include "base/Logging.h"
#include "base/Singleton.h"
#include <set>
#include <list>
#include<mutex>
#include<map>
#include<memory>

using namespace std;

#define GROUPID_BOUBDARY   0x0FFFFFFF 

class User
{
public:
    int32_t          userid;//用户id
    string           username;//用户名
    string           password;//密码
    string           nickname;//昵称
    int32_t          facetype;//默认头像
    string           custimface;//自定义头像
    string           customfacefmt;//自定义头像的格式
    int32_t          gender;//性别
    int32_t          birthday;//生日
    string           signature;//签名
    string           address;//位置
    string           phonenumber;//手机号
    string           mail;//邮箱
    int32_t          ownerid;//用于标记群主信息
    
    int32_t          clienttype;//客户端类型,pc=1, android=2, ios=3
    int32_t          status;         //在线状态 0离线 1在线 2忙碌 3离开 4隐身
    set<int32_t>     friends; // 好友id

    User():userid(-1),facetype(-1),gender(-1),
           birthday(-1),ownerid(-1){}

    ~User() = default;
};


typedef shared_ptr<User> UserPtr;

class UserManager final
{
public:
    UserManager() = default;
    ~UserManager() = default;

    bool Init();

    UserManager(const UserManager &) = delete;
    UserManager &operator=(const UserManager &) = delete;

    //添加用户
    bool AddUser(User &user);
    //从数据库加载用户信息
    bool LoadUserFromDB();
    bool LoadRelationshipFromDB(int32_t userid, set<int32_t>& friends);
    bool GetUserInfoByUsername(const string &name, User &user);
    bool GetFriendInfoByUserId(int32_t userid, list<User> &friends);
    UserPtr GetUserByID(int32_t userid);
    bool GetUserInfoByUserId(int32_t userid, User& u);
    bool AddFriendToUser(int32_t userid, int32_t friendid);
    bool MakeFriendRelationship(int32_t smallid, int32_t greatid);
    bool ReleaseFriendRelationship(int32_t smallid, int32_t greatid);
    bool UpdateUserInfo(int32_t userid, const User &newuserinfo);
    bool ModifyUserPassword(int32_t userid, const string &newpassword);
    bool AddGroup(const char *groupname, int32_t ownerid, int32_t &groupid);
    bool SaveChatMsgToDb(int32_t senderid, int32_t targetid, const string &chatmsg);
    bool DeleteFriendToUser(int32_t userid, int32_t friendid);

private:
    mutex m_mutex;
    list<User> m_cachedUsers;
    map<int32_t, UserPtr> m_mapUsers;
    int32_t m_baseUserID{0};
    int32_t m_baseGrupID{0xFFFFFFF};

    string              m_strDbServer;
    string              m_strDbUserName;
    string              m_strDbPassword;
    string              m_strDbName;
};

