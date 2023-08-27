#include"FileSession.h"
#include<sstream>
#include<list>
#include"../net/TcpConnection.h"
#include"../BinaryReader.h"
#include"../base/Logging.h"
#include"../base/Singleton.h"
#include"FileManager.h"
#include"FileMsg.h"

using namespace edoyun;

FileSession::FileSession(TcpConnectionPtr conn)
:TcpSession(conn),m_id(0),m_seq(0),m_fp(NULL)
{
    // conn->setMessageCallback(std::bind(&FileSession::OnRead,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
}

void FileSession::OnRead(const TcpConnectionPtr &conn, Buffer *buf,Timestamp receivTime)
{
    while (true)
    {
        if(buf->readableBytes() < (size_t)sizeof(file_msg))
        {
            LOG_INFO << "buffer is not enough for a package header, pBuffer->readableBytes()=" << buf->readableBytes() << ", sizeof(msg)=" << sizeof(file_msg);
            // cout << "buffer is not enough for a package header, pBuffer->readableBytes()=" << pbuffer->readableBytes() << ", sizeof(msg)=" << sizeof(file_msg);
            return;
        }

        //不够一个整包大小
        file_msg header;
        memcpy(&header, buf->peek(), sizeof(file_msg));
        if (buf->readableBytes() < (size_t)header.packagesize + sizeof(file_msg))
        {
            return;
        }
        buf->retrieve(sizeof(file_msg));
        string inbuf;
        inbuf.assign(buf->peek() + 6, header.packagesize - 6);

        // inbuf.append(pbuffer->peek(), header.packagesize);
        buf->retrieve(header.packagesize);
        if (!Process(conn, inbuf))
        {
            LOG_WARN << "Process error, close TcpConnection";
            // cout <<__FILE__<<"("<<__LINE__<<")" <<"Process error, close TcpConnection";
            conn->forceClose();
        }
    }
    //TODO:业务代码
    // while (buf->readableBytes() >= sizeof(int64_t))
    // {
    //     int64_t packagesize = 0;
    //     // BinaryReader::dump(buf->peek(),buf->readableBytes());
    //     packagesize = *(int64_t*)buf->peek();
    //     if(buf->readableBytes() < (sizeof(int32_t) + packagesize))
    //     {
    //         return;
    //     }
    //     buf->retrieve(sizeof(int64_t));
    //     string msgbuff;

    //     // BinaryReader::dump(buf->peek(),buf->readableBytes());
    //     msgbuff.assign(buf->peek() + 6, packagesize - 6);
    //     // BinaryReader::dump(msgbuff);
    //     // buf->retrieve(6);
    //     // cout << " 00 00 03 EA = " << htonl(*(int32_t *)buf->peek());
    //     buf->retrieve(packagesize);
    //     //TODO:处理消息
    //     // cout << "收到消息 ： " << msgbuff << endl;
    //     if (Process(conn, msgbuff) != true)
    //     {
    //         cout << "process error,close connect!" << endl;
    //         conn->forceClose();
    //     }
    // }
}

void FileSession::SendUserStatusChangeMsg(int32_t userid, int type)
{
    
}

bool FileSession::Process(const TcpConnectionPtr& conn,const string buffer)
{
    cout << __FILE__ << "(" << __LINE__ << ") 进入Process buffer.size"<<buffer.size() << endl;
    // BinaryReader::dump(buffer);
    BinaryReader reader(buffer);
    int cmd;
    if(!reader.ReadData(cmd))
    {
        LOG_WARN << "read cmd error !!!";
        // cout <<__FILE__<<"("<<__LINE__<<")" << "read cmd error !!!";
        return false;
    }
    cout << __FILE__ << "(" << __LINE__ << ") cmd=" <<cmd<< endl;
    if (!reader.ReadData(m_seq))
    {
        LOG_WARN << "read seq error !!!";
        // cout <<__FILE__<<"("<<__LINE__<<")"<< "read seq error !!!";
        return false;
    }
    cout << __FILE__ << "(" << __LINE__ << ") m_seq=" <<m_seq<< endl;
    string filemd5;
    // size_t md5Len;
    if(!reader.ReadData(filemd5) || filemd5.size() == 0)
    {
        LOG_WARN << "read filemd5 error !!!";
        // cout <<__FILE__<<"("<<__LINE__<<")"<< "read filemd5 error !!!";
        return false;
    }

    int offset;
    if(!reader.ReadData(offset))
    {
        LOG_WARN << "read offset error !!!";
        // cout <<__FILE__<<"("<<__LINE__<<")"<< "read offset error !!!";
        return false;
    }
    cout << __FILE__ << "(" << __LINE__ << ") offset=" <<offset<< endl;
    int filesize;
    if(!reader.ReadData(filesize))
    {
        LOG_WARN << "read filesize error !!!";
        // cout <<__FILE__<<"("<<__LINE__<<")"<< "read filesize error !!!";
        return false;
    }
    cout << __FILE__ << "(" << __LINE__ << ") filesize=" <<filesize<< endl;
    string filedata;
    // size_t filedataLen;
    if(!reader.ReadData(filedata))
    {
        LOG_WARN << "read filedata error !!!";
        // cout <<__FILE__<<"("<<__LINE__<<")"<< "read filesize error !!!";
        return false;
    }

    LOG_INFO << "Recv from client: cmd=" << cmd << ", seq=" << m_seq << ", header.packagesize:" << buffer.length() << ", filemd5=" << filemd5 << ", md5length=" << filemd5.size();
    // cout <<__FILE__<<"("<<__LINE__<<")" << "Recv from client: cmd=" << cmd << ", seq=" << m_seq << ", header.packagesize:" << buffer.length() << ", filemd5=" << filemd5 << ", md5length=" << filemd5.size();

    switch (cmd)
    {
        //文件上传
        case msg_type_upload_req:
            OnUploadFileResponse(filemd5, offset, filesize, filedata, conn);
            break;
        //客户端上传的文件内容, 服务器端下载
        case msg_type_download_req:
        {           
            //对于下载，客户端不知道文件大小， 所以值是0
            if (filedata.size() != 0)
                return false;
            OnDownloadFileResponse(filemd5, offset, filesize,  conn);
        }
            break;

        default:
            LOG_WARN << "unsupport cmd, cmd:" << cmd << ", connection name:" << conn->peerAddress().toIpPort();
            // cout <<__FILE__<<"("<<__LINE__<<")"<< "unsupport cmd, cmd:" << cmd << ", connection name:" << conn->peerAddress().toIpPort();
            return false;
    }

    ++m_seq;

    return true;
}

void FileSession::OnUploadFileResponse(const std::string& filemd5, int32_t offset, int32_t filesize, const std::string& filedata, const TcpConnectionPtr& conn)
{
    cout << __FILE__ << "(" << __LINE__ << ") 文件上传OnUploadFileResponse" << endl;
    cout << "filemd5=" << filemd5 << "  offset=" << offset << "   filesize=" << filesize << endl;
    if (filemd5.empty())
    {
        LOG_WARN << "Empty filemd5, connection name:" << conn->peerAddress().toIpPort();
        // cout <<__FILE__<<"("<<__LINE__<<")"<< "Empty filemd5, connection name:" << conn->peerAddress().toIpPort();
        return;
    }

    string outbuf;
    BinaryWriter writer;
    //补充长度
    // writer.writerFile();
    writer.WriteData(htonl(msg_type_upload_resp));
    writer.WriteData(htonl(m_seq));
    writer.WriteData(filemd5);

    if(Singleton<FileManager>::instance().IsFileExsit(filemd5.c_str()))
    {
        
        offset = filesize = -1;
        writer.WriteData(htonl(offset));
        writer.WriteData(htonl(filesize));
        string dummyfiledata;
        writer.WriteData(dummyfiledata);

        LOG_INFO << "Response to client: cmd=msg_type_upload_resp" << ", connection name:" << conn->peerAddress().toIpPort();
        // cout <<__FILE__<<"("<<__LINE__<<")"<< "Response to client: cmd=msg_type_upload_resp" << ", connection name:" << conn->peerAddress().toIpPort();
        cout << __FILE__ << "(" << __LINE__ << ") 1 Send(writer)OnUploadFileResponse"<<"  conn.str="<<string((char*)(void*)conn.get()) << endl;
        BinaryReader::dump(writer.toString());
        Send(writer);
        return;
    }

    
    if(offset == 0)
    {
        string filename = "filecache/";
        filename += filemd5;
        m_fp = fopen(filename.c_str(), "w");
        if(m_fp == NULL)
        {
            //  cout <<__FILE__<<"("<<__LINE__<<")"
            LOG_INFO<< "fopen file error, filemd5=" << filemd5 << ", connection name:" << conn->peerAddress().toIpPort();
            return;
        }
    }
    fseek(m_fp, offset, SEEK_SET);
    if (fwrite(filedata.c_str(), filedata.length(), 1, m_fp) != 1)
    {
        //  cout <<__FILE__<<"("<<__LINE__<<")"
        LOG_ERROR << "fwrite error, filemd5: " << filemd5
                  << ", errno: " << errno << ", errinfo: " << strerror(errno)
                  << ", filedata.length(): " << filedata.length()
                  << ", m_fp: " << m_fp
                  << ", buffer size is 512*1024"
                  << ", connection name:" << conn->peerAddress().toIpPort();
        return;
    }

    //文件上传成功
    if (offset + (int32_t)filedata.length() == filesize)
    {
        offset = filesize = -1;
        Singleton<FileManager>::instance().addFile(filemd5.c_str());
        ResetFile();
    }

    writer.WriteData(htonl(offset));
    writer.WriteData(htonl(filesize));
    string dummyfiledatax;
    writer.WriteData(dummyfiledatax);
    cout << __FILE__ << "(" << __LINE__ << ") 2 Send(writer)OnUploadFileResponse" << endl;
    BinaryReader::dump(writer.toString());
    Send(writer);

    //  cout <<__FILE__<<"("<<__LINE__<<")"
    LOG_INFO<< "2Response to client: cmd=msg_type_upload_resp" << ", connection name:" << conn->peerAddress().toIpPort();
    
    // fseek(m_fp, offset, SEEK_SET);

}

void FileSession::OnDownloadFileResponse(const std::string& filemd5, int32_t offset, int32_t filesize, const TcpConnectionPtr& conn)
{
    cout << __FILE__ << "(" << __LINE__ << ") 进入OnDownloadFileResponse" << endl;
    if (filemd5.empty())
    {
        //  cout <<__FILE__<<"("<<__LINE__<<")"
        LOG_WARN<< "Empty filemd5, connection name:" << conn->peerAddress().toIpPort();
        cout << __FILE__ << "(" << __LINE__ << ") OnDownloadFileResponse  filemd5.empty()" << endl;
        return;
    }

    //TODO: 客户端下载不存在的文件，不应答客户端？
    if (!Singleton<FileManager>::instance().IsFileExsit(filemd5.c_str()))
    {
        //  cout <<__FILE__<<"("<<__LINE__<<")"
        LOG_WARN<< "filemd5 not exsit, filemd5: " << filemd5 << ", connection name:" << conn->peerAddress().toIpPort();
        cout << __FILE__ << "(" << __LINE__ << ") OnDownloadFileResponse  IsFileExsit文件不存在" << endl;
        return;
    }

    if(m_fp == NULL)
    {
        string filename = "filecache/";
        filename += filemd5;
        m_fp = fopen(filename.c_str(), "r+");
        if(m_fp == NULL)
        {
            //  cout <<__FILE__<<"("<<__LINE__<<")"
            LOG_ERROR<< "fopen file error, filemd5: " << filemd5 << ", connection name:" << conn->peerAddress().toIpPort();
            cout << __FILE__ << "(" << __LINE__ << ") OnDownloadFileResponse  m_fp文件不存在或打开错误" << endl;
            return;
        }
        fseek(m_fp, 0, SEEK_END);
        m_filesize = ftell(m_fp);
        if(m_filesize <= 0)
        {
            //  cout <<__FILE__<<"("<<__LINE__<<")"
            LOG_ERROR<< "m_filesize: " << m_filesize << ", errno: " << errno << ", filemd5: " << filemd5 << ", connection name : " << conn->peerAddress().toIpPort();
            cout << __FILE__ << "(" << __LINE__ << ") OnDownloadFileResponse  m_filesize <= 0 文件读取错误" << endl;
			return;
        }
        fseek(m_fp, 0, SEEK_SET);

    }

    // string outbuf;
    BinaryWriter writer;
    //补充长度
    // writer.writerFile();
    writer.WriteData(htonl(msg_type_download_resp));
    writer.WriteData(htonl(m_seq));
    writer.WriteData(filemd5);

    string filedata;

    int32_t currentSendSize = 512 * 1024;
    char buffer[512 * 1024] = {0};
    if(m_filesize <= m_offset + currentSendSize)
    {
        currentSendSize = m_filesize - m_offset;
    }

    //  cout <<__FILE__<<"("<<__LINE__<<")"
    LOG_INFO << "currentSendSize: " << currentSendSize 
			 << ", m_filesize: " << m_filesize 
			 << ", m_offset: " << m_offset 
			 << ", filemd5: " << filemd5
			 << ", connection name:" << conn->peerAddress().toIpPort();
    
    if (currentSendSize <= 0 || fread(buffer, currentSendSize, 1, m_fp) != 1)
	{
		//  cout <<__FILE__<<"("<<__LINE__<<")"
        LOG_ERROR   << "fread error, filemd5: " << filemd5
					<< ", errno: " << errno << ", errinfo: " << strerror(errno)
					<< ", currentSendSize: " << currentSendSize
					<< ", m_fp: " << m_fp
					<< ", buffer size is 512*1024"
					<< ", connection name:" << conn->peerAddress().toIpPort();
	}

    writer.WriteData(htonl(m_offset));
    m_offset += currentSendSize;
    filedata.append(buffer, currentSendSize);
    writer.WriteData(htonl(m_filesize));
    writer.WriteData(filedata);

    //  cout <<__FILE__<<"("<<__LINE__<<")"
    LOG_INFO<< "Response to client: cmd = msg_type_download_resp, filemd5: " << filemd5 << ", connection name:" << conn->peerAddress().toIpPort();
    cout << __FILE__ << "(" << __LINE__ << ") Send(writer)OnDownloadFileResponse" << endl;
    // BinaryReader::dump(writer.toString());
    Send(writer);

    //文件已经下载完成
    if (m_offset == m_filesize)
    {
        ResetFile();
    }
}

void FileSession::ResetFile()
{
    if (m_fp)
    {
        fclose(m_fp);
        m_offset = 0;
        m_filesize = 0;
		m_fp = NULL;
    }
}

FileSession::~FileSession()
{
    
}
