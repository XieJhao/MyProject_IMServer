#pragma once
#include<stdint.h>
#include<string>
#include<list>
#include<mutex>

using namespace std;

struct NotifyMsgCache
{
    int32_t     userid;
    string      notifymsg; 
};

struct ChatMsgCache
{
    int32_t userid;
    string chatmsg;
};

class MsgCacheManager final
{
public:
    MsgCacheManager(/* args */);
    ~MsgCacheManager();
    MsgCacheManager(const MsgCacheManager &rhs) = delete;
    MsgCacheManager &operator=(const MsgCacheManager &rhs) = delete;

    bool AddNotifyMsgCache(int32_t userid, const string &cache);
    void GetNotifyMsgCache(int32_t userid, list<NotifyMsgCache>& cache);

    bool AddChatMsgCache(int32_t userid, const string &cache);
    void GetChatMagCache(int32_t userid, list<ChatMsgCache> &cache);

private:
    list<NotifyMsgCache>    m_listNotifyMsgCache;//通知类消息缓存
    mutex                   m_mtNotifyMsgCache;
    list<ChatMsgCache>      m_listChatMsgCache;//聊天消息缓存
    mutex                   m_mtChatMsgCache;
};
//改进优化
// 1 数据库缓存（数据库表的设计和使用、消息缓存的初始化、群中的消息缓存）
// 2 缓存数据结构的优化
