#include"MsgCacheManager.h"
#include "base/Logging.h"

MsgCacheManager::MsgCacheManager(/* args */)
{
    
}

bool MsgCacheManager::AddNotifyMsgCache(int32_t userid, const string &cache)
{
    std::lock_guard<std::mutex> guard(m_mtNotifyMsgCache);
    NotifyMsgCache nc;
    nc.userid = userid;
    nc.notifymsg.append(cache.c_str(), cache.length());;
    m_listNotifyMsgCache.push_back(nc);
    LOG_INFO << "append notify msg to cache, userid: " << userid << ", m_mapNotifyMsgCache.size() : " << m_listNotifyMsgCache.size() << ", cache length : " << cache.length();

    //TODO:存入或写入数据以防程序崩溃丢失
    return true;
}

void MsgCacheManager::GetNotifyMsgCache(int32_t userid, list<NotifyMsgCache>& cache)
{
    lock_guard<mutex> guard(m_mtNotifyMsgCache);
    for (auto iter = m_listNotifyMsgCache.begin(); iter != m_listNotifyMsgCache.end();)
    {
        if(iter->userid == userid)
        {
            cache.push_back(*iter);
            iter = m_listNotifyMsgCache.erase(iter);
        }
        else
        {
            iter++;
        }
    }
    LOG_INFO << "get notify msg  cache,  userid: " << userid << ", m_mapNotifyMsgCache.size(): " << m_listNotifyMsgCache.size() << ", cached size: " << cache.size();
    
}

bool MsgCacheManager::AddChatMsgCache(int32_t userid, const string &cache)
{
    std::lock_guard<std::mutex> guard(m_mtChatMsgCache);
    ChatMsgCache c;
    c.userid = userid;
    c.chatmsg.append(cache.c_str(), cache.length());
    m_listChatMsgCache.push_back(c);
    LOG_INFO << "append chat msg to cache, userid: " << userid << ", m_listChatMsgCache.size() : " << m_listChatMsgCache.size() << ", cache length : " << cache.length();

    return true;
}

void MsgCacheManager::GetChatMagCache(int32_t userid, list<ChatMsgCache> &cache)
{
    std::lock_guard<mutex> guard(m_mtChatMsgCache);
    for (auto iter = m_listChatMsgCache.begin(); iter != m_listChatMsgCache.end();)
    {
        if(iter->userid == userid)
        {
            // ChatMsgCache chat;
            // chat.userid = iter->userid;
            // chat.chatmsg = iter->chatmsg;
            cache.push_back(*iter);
            iter = m_listChatMsgCache.erase(iter);
        }
        else
        {
            iter++;
        }
    }
    LOG_INFO << "get chat msg cache, no cache,  userid: " << userid << ",m_listChatMsgCache.size(): " << m_listChatMsgCache.size() << ", cached size: " << cache.size();
    
}

MsgCacheManager::~MsgCacheManager()
{
    
}
