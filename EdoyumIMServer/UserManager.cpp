#include "UserManager.h"
#include "base/Singleton.h"
#include "MySqlManager.h"
#include <sstream>

using namespace muduo;

bool UserManager::Init()
{
    //加载用户信息 (所有的用户)
    if(LoadUserFromDB() == false)
    {
        cout << "load users from database failed!" << endl;
        return false;
    }
    
    //加载用户关系
    for (auto& iter : m_cachedUsers)
    {
        if(!LoadRelationshipFromDB(iter.userid,iter.friends))
        {
            cout << "load friend failed!" << endl;
        }
        
    }
    return true;
}

bool UserManager::AddUser(User &user)
{
    stringstream sql;
    m_baseUserID++;
    sql << "INSERT INTO t_user (f_user_id,f_username,f_nickname,f_password,f_register_time)"
        << " VALUES(" << m_baseUserID << ",'" << user.username << "','" << user.nickname << "','" << user.password << "',NOW());";

    bool result = Singleton<MySqlManager>::instance().Excute(sql.str());
    if (result== false)
    {
        return false;
    }
    user.userid = m_baseUserID;
    user.facetype = 0;
    user.birthday = 20000101;
    user.gender = 0;
    user.ownerid = 0;
    {
        lock_guard<mutex> guard(m_mutex);
        m_cachedUsers.push_back(user);
    }
    return true;
}

bool UserManager::LoadUserFromDB()
{
    stringstream sql;
    sql << "SELECT f_user_id,f_username,f_nickname,f_password,f_facetype,f_customface,f_gender,f_birthday,f_signature,f_address,f_phonenumber,f_mail FROM t_user ORDER BY f_user_id DESC;";

    //先注册的用户，id小，后注册的用户id大
    //后注册的用户上线概率会更大，所以进行降序排序
    // cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    QueryResultPtr result = Singleton<MySqlManager>::instance().Query(sql.str());
    // cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    if (result== nullptr)
    {
        return false;
    }
    // cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    while (result != nullptr)
    {
        Field *pRow = result->Fetch();
        if(pRow == nullptr)
        {
            break;
        }
        User u;
        u.userid = pRow[0].toInt32();
        u.username = pRow[1].GetString();
        u.nickname = pRow[2].GetString();
        u.password = pRow[3].GetString();
        u.facetype = pRow[4].toInt32();
        u.custimface = pRow[5].GetString();
        u.gender = pRow[6].toInt32();
        u.birthday = pRow[7].toInt32();
        u.signature = pRow[8].GetString();
        u.address = pRow[9].GetString();
        u.phonenumber = pRow[10].GetString();
        u.mail = pRow[11].GetString();
        {
            lock_guard<mutex> guard(m_mutex);
            m_cachedUsers.push_back(u);
        }

        if(u.userid > m_baseUserID)
        {
            m_baseUserID = u.userid;
        }
        if(result->NextRow() == false)
        {
            break;
        }
    }
    result->EndQuery();
    return true;
}

bool UserManager::LoadRelationshipFromDB(int32_t userid, set<int32_t>& friends)
{
    stringstream sql;
    sql << "SELECT f_user_id1,f_user_id2 FROM t_user_relationship WHERE f_user_id1=" << userid << " OR f_user_id2=" << userid << ";";
    QueryResultPtr result = Singleton<MySqlManager>::instance().Query(sql.str());
    if(result == nullptr)
    {
        cout << "(LoadRelationshipFromDB) the sql is error" << endl;
        return false;
    }
    while (result != nullptr)
    {
        Field *pRow = result->Fetch();
        if(pRow == nullptr)
        {
            break;
        }
        int friendid1 = pRow[0].toInt32();
        int firendid2 = pRow[1].toInt32();
        cout << "friendid1=" << friendid1 << "   firendid2=" << firendid2 << endl;
        if (friendid1 == userid)
        {
            friends.insert(firendid2);
            LOG_INFO << "userid=" << userid << ", friendid=" << firendid2;
        }
        else
        {
            friends.insert(friendid1);
            LOG_INFO << "userid=" << userid << ", friendid=" << friendid1;
        }

        if (result->NextRow() == false)
        {
            break;
        }
    }
    result->EndQuery();

    return true;
}

// bool UserManager::GetUserInfoByUsername(const string &name, UserPtr &user)
// {
    
// }

bool UserManager::GetUserInfoByUsername(const string &name, User &user)
{
    lock_guard<mutex> guard(m_mutex);
    for(const auto& iter : m_cachedUsers)
    {
        if(iter.username == name)
        {
            user = iter;
            return true;
        }
    }
    return false;
}

bool UserManager::GetFriendInfoByUserId(int32_t userid, list<User> &friends)
{
    std::set<int32_t> friendsId;
    std::lock_guard<std::mutex> guard(m_mutex);
    for (const auto& iter : m_cachedUsers)
    {
        if (iter.userid == userid)
        {
            friendsId = iter.friends;
            break;
        }
    }
    // TODO: 这种算法效率太低
    for (const auto& iter : friendsId)
    {
        User u;
        for (const auto& iter2 : m_cachedUsers)
        {
            if (iter2.userid == iter)
            {
                u = iter2;
                friends.push_back(u);
                break;
            }
        }
    }

    return true;
}

bool UserManager::GetUserInfoByUserId(int32_t userid, User& u)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    for (const auto& iter : m_cachedUsers)
    {
        if (iter.userid == userid)
        {
            u = iter;
            return true;
        }
    }

    return false;
}

bool UserManager::AddFriendToUser(int32_t userid, int32_t friendid)
{
    bool bFound1 = false;
    bool bFound2 = false;
    std::lock_guard<std::mutex> guard(m_mutex);
    for (auto& iter : m_cachedUsers)
    {
        if (iter.userid == userid)
        {
            iter.friends.insert(friendid);
            bFound1 = true;
        }

        if (iter.userid == friendid)
        {
            iter.friends.insert(userid);
            bFound2 = true;
        }

        if (bFound1 && bFound2)
            return true;
    }

    return false;
}

UserPtr UserManager::GetUserByID(int32_t userid)
{
    lock_guard<mutex> guard(m_mutex);
    auto iter = m_mapUsers.find(userid);
    if(iter == m_mapUsers.end())
    {
        return UserPtr();
    }
    return iter->second;

}

bool UserManager::MakeFriendRelationship(int32_t smallid, int32_t greatid)
{
    if (smallid >= greatid)
        return false;
    // auto iter = m_mapUsers.find(smallid);
    // std::unique_ptr<MySQLTool> pConn;
    // pConn.reset(new MySQLTool());
    // if (!pConn->connect(m_strDbServer, m_strDbUserName, m_strDbPassword, m_strDbName))
    // {
    //     LOG_FATAL << "UserManager::LoadUsersFromDb failed, please check params";
    //     return false;
    // }
    stringstream sql;
    sql << "INSERT INTO t_user_relationship(f_user_id1, f_user_id2) VALUES(" << smallid << "," << greatid << ") ;";

    // char sql[256] = { 0 };
    // snprintf(sql, 256, "INSERT INTO t_user_relationship(f_user_id1, f_user_id2) VALUES(%d, %d)", smallid, greatid);
    if(!Singleton<MySqlManager>::instance().Excute(sql.str()))
    {
        LOG_WARN << "make relationship error, sql=" << sql.str() << ", smallUserid = " << smallid << ", greaterUserid = " << greatid;;
        return false;
    }
    
    if (!AddFriendToUser(smallid, greatid))
    {
        LOG_WARN << "make relationship error, smallUserid=" << smallid << ", greaterUserid=" << greatid;
        return false;
    }

    return true;
}

bool UserManager::ReleaseFriendRelationship(int32_t smallid, int32_t greatid)
{
    if(smallid >= greatid)
        return false;

    // std::unique_ptr<MySQLTool> pConn;
    // pConn.reset(new MySQLTool());
    // if (!pConn->connect(m_strDbServer, m_strDbUserName, m_strDbPassword, m_strDbName))
    // {
    //     LOG_FATAL << "UserManager::LoadUsersFromDb failed, please check params";
    //     return false;
    // }

    char sql[256] = { 0 };
    snprintf(sql, 256, "DELETE FROM t_user_relationship WHERE f_user_id1 = %d AND f_user_id2 = %d ;", smallid, greatid);
    if (!Singleton<MySqlManager>::instance().Excute(sql))
    {
        LOG_WARN << "release relationship error, sql=" << sql << ", smallUserid = " << smallid << ", greaterUserid = " << greatid;;
        return false;
    }

    if (!DeleteFriendToUser(smallid, greatid))
    {
        LOG_WARN << "delete relationship error, smallUserid=" << smallid << ", greaterUserid=" << greatid;
        return false;
    }

    return true;
}

bool UserManager::UpdateUserInfo(int32_t userid, const User &newuserinfo)
{
    // std::unique_ptr<MySQLTool> pConn;
    // pConn.reset(new MySQLTool());
    // if (!pConn->connect(m_strDbServer, m_strDbUserName, m_strDbPassword, m_strDbName))
    // {
    //     LOG_ERROR << "UserManager::Initialize db failed, please check params";
    //     return false;
    // }

    std::ostringstream osSql;
    osSql << "UPDATE t_user SET f_nickname='"
          << newuserinfo.nickname << "', f_facetype="
          << newuserinfo.facetype << ", f_customface='"
          << newuserinfo.custimface << "', f_gender="
          << newuserinfo.gender << ", f_birthday="
          << newuserinfo.birthday << ", f_signature='"
          << newuserinfo.signature << "', f_address='"
          << newuserinfo.address << "', f_phonenumber='"
          << newuserinfo.phonenumber << "', f_mail='"
          << newuserinfo.mail << "' WHERE f_user_id="
          << userid << ";";
    if (!Singleton<MySqlManager>::instance().Excute(osSql.str()))
    {
        LOG_ERROR << "UpdateUserInfo error, sql=" << osSql.str();
        return false;
    }

    LOG_INFO << "update userinfo successfully, userid: " << userid << ", sql: " << osSql.str();

    std::lock_guard<std::mutex> guard(m_mutex);
    for (auto& iter : m_cachedUsers)
    {
        if (iter.userid == userid)
        {
            iter.nickname = newuserinfo.nickname;
            iter.facetype = newuserinfo.facetype;
            iter.custimface = newuserinfo.custimface;
            iter.gender = newuserinfo.gender;
            iter.birthday = newuserinfo.birthday;
            iter.signature = newuserinfo.signature;
            iter.address = newuserinfo.address;
            iter.phonenumber = newuserinfo.phonenumber;
            iter.mail = newuserinfo.mail;
            return true;
        }
    }

    LOG_ERROR << "update userinfo to db successfully, find exsit user in memory error, m_allCachedUsers.size(): " << m_cachedUsers.size() << ", userid: " << userid << ", sql : " << osSql.str();

    return false;
}

bool UserManager::ModifyUserPassword(int32_t userid, const string &newpassword)
{
    // std::unique_ptr<MySQLTool> pConn;
    // pConn.reset(new MySQLTool());
    // if (!pConn->connect(m_strDbServer, m_strDbUserName, m_strDbPassword, m_strDbName))
    // {
    //     LOG_ERROR << "UserManager::Initialize db failed, please check params";
    //     return false;
    // }

    std::ostringstream osSql;
    osSql << "UPDATE t_user SET f_password='"
          << newpassword << "' WHERE f_user_id="
          << userid << ";";
    if (!Singleton<MySqlManager>::instance().Excute(osSql.str()))
    {
        LOG_ERROR << "UpdateUserInfo error, sql=" << osSql.str();
        return false;
    }

    LOG_INFO << "update user password successfully, userid: " << userid << ", sql : " << osSql.str();

    std::lock_guard<std::mutex> guard(m_mutex);
    for (auto& iter : m_cachedUsers)
    {
        if (iter.userid == userid)
        {
            iter.password = newpassword;         
            return true;
        }
    }

    LOG_ERROR << "update user password to db successfully, find exsit user in memory error, m_allCachedUsers.size(): " << m_cachedUsers.size() << ", userid: " << userid << ", sql : " << osSql.str();

    return false;
}

bool UserManager::AddGroup(const char *groupname, int32_t ownerid, int32_t &groupid)
{
    // std::unique_ptr<MySQLTool> pConn;
    // pConn.reset(new MySQLTool());
    // if (!pConn->connect(m_strDbServer, m_strDbUserName, m_strDbPassword, m_strDbName))
    // {
    //     LOG_FATAL << "UserManager::AddUser failed, please check params";
    //     return false;
    // }

    ++m_baseGrupID;
    char sql[256] = { 0 };
    snprintf(sql, 256, "INSERT INTO t_user(f_user_id, f_username, f_nickname, f_password, f_owner_id, f_register_time) VALUES(%d, '%d', '%s', '', %d,  NOW());", m_baseGrupID, m_baseGrupID, groupname, ownerid);
    if (!Singleton<MySqlManager>::instance().Excute(string(sql)))
    {
        LOG_WARN << "insert group error, sql=" << sql;
        return false;
    }
    
    groupid = m_baseGrupID;

    User u;
    u.userid = groupid;
    char szUserName[12] = { 0 };
    snprintf(szUserName, 12, "%d", groupid);
    u.username = szUserName;
    u.nickname = groupname;
    u.ownerid = ownerid;
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_cachedUsers.push_back(u);
    }

    return true;
}

bool UserManager::SaveChatMsgToDb(int32_t senderid, int32_t targetid, const string &chatmsg)
{
    // std::unique_ptr<MySQLTool> pConn;
    // pConn.reset(new MySQLTool());
    // if (!pConn->connect(m_strDbServer, m_strDbUserName, m_strDbPassword, m_strDbName))
    // {
    //     LOG_FATAL << "UserManager::SaveChatMsgToDb failed, please check params";
    //     return false;
    // }

    ostringstream sql;
    sql << "INSERT INTO t_chatmsg(f_senderid, f_targetid, f_msgcontent) VALUES(" << senderid << ", " << targetid << ", '" << chatmsg << "');";
    if (!Singleton<MySqlManager>::instance().Excute(sql.str()))
    {
        LOG_WARN << "UserManager::SaveChatMsgToDb, sql=" << sql.str() << ", senderid = " << senderid << ", targetid = " << targetid << ", chatmsg:" << chatmsg;
        return false;
    }

    return true;
}

bool UserManager::DeleteFriendToUser(int32_t userid, int32_t friendid)
{
    bool bFound1 = false;
    bool bFound2 = false;
    std::lock_guard<std::mutex> guard(m_mutex);
    for (auto& iter : m_cachedUsers)
    {
        if (iter.userid == userid)
        {
            iter.friends.erase(friendid);
            bFound1 = true;
        }

        if (iter.userid == friendid)
        {
            iter.friends.erase(userid);
            bFound2 = true;
        }

        if (bFound1 && bFound2)
            return true;
    }

    return false;
}
