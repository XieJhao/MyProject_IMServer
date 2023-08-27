#pragma once

#include"../net/Buffer.h"
#include"TcpSession.h"

class FileSession : public TcpSession
{
public:
    
    FileSession(TcpConnectionPtr conn);
    ~FileSession();

    FileSession(const FileSession &rhs) = delete;

    FileSession &operator=(const FileSession &rhs) = delete;

    //有数据可读, 会被多个工作loop调用
    void OnRead(const TcpConnectionPtr &conn, Buffer *pbuffer,Timestamp receivTime);

    void SendUserStatusChangeMsg(int32_t userid, int type);

private:
    bool Process(const TcpConnectionPtr& conn, const string buffer);
    
    void OnUploadFileResponse(const std::string& filemd5, int32_t offset, int32_t filesize, const std::string& filedata, const TcpConnectionPtr& conn);
    void OnDownloadFileResponse(const std::string& filemd5, int32_t offset, int32_t filesize, const TcpConnectionPtr& conn);

    void ResetFile();
private:
    int32_t           m_id;         //session id
    int               m_seq;        //当前Session数据包序列号

    //当前文件信息
    FILE*             m_fp{};
    int32_t           m_offset{};
    int32_t           m_filesize{};
};

typedef shared_ptr<FileSession> FileSessionPtr;