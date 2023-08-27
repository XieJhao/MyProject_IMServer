#pragma once

#include<stdint.h>

enum file_msg_type
{
    file_msg_type_unknown,
    msg_type_upload_req,
    msg_type_upload_resp,
    msg_type_download_req,
    msg_type_download_resp,
};

struct file_msg
{
    int64_t packagesize;
};

#pragma pack(pop)

/** 
 *  文件上传
 */
/**
    客户端：cmd = msg_type_upload_req, seq = 0, filemd5, offset ,filesize, filedata
    服务器应答： cmd = msg_type_upload_resp, seq = 0, filemd5, offset, filesize//offset与filesize=-1时上传完成
 **/

/** 
 *  文件下载
 */
/** 
    客户端：cmd = msg_type_download_req, seq = 0, filemd5, offset, filesize, filedata
    服务器: cmd = msg_type_download_resp, seq = 0, filemd5, filesize, offset, filedata

 **/

