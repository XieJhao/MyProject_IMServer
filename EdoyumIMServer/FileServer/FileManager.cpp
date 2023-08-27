#include"FileManager.h"
#include<sys/types.h>
#include<dirent.h>
#include<string.h>
#include"../base/Logging.h"
#include<sys/stat.h>

FileManager::FileManager()
{
    
}

FileManager::~FileManager()
{
    
}

bool FileManager::Init(const char* basepath)
{
    DIR *dp = opendir(basepath);
    if (dp == NULL)
    {
        //  cout <<__FILE__<<"("<<__LINE__<<")"
        LOG_INFO<< "open base dir error, errno: " << errno << ", " << strerror(errno);

        if (mkdir(basepath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
            return true;

        //  cout <<__FILE__<<"("<<__LINE__<<")"
         LOG_ERROR<< "create base dir error, " << basepath << ", errno: " << errno << ", " << strerror(errno);

        return false;
    }

    struct dirent *dirp;
    while ((dirp = readdir(dp)) != nullptr)
    {
        // cout << __FILE__ << "(" << __LINE__ << ") dirp->d_name=" << dirp->d_name << endl;
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
        {
            continue;
        }

        m_listFiles.emplace_back(dirp->d_name);
        //   cout <<__FILE__<<"("<<__LINE__<<")"
        LOG_INFO<< "filename: " << dirp->d_name;
    }
    cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    closedir(dp);

    return true;
}

bool FileManager::IsFileExsit(const char* filename)
{
    std::lock_guard<std::mutex> guard(m_mtFile);
    //先看缓存
    for (const auto &iter : m_listFiles)
    {
        if(iter == filename)
        {
            return true;
        }
    }

    //再查看文件系统
    FILE *fp = fopen(filename, "r");
    if(fp != NULL)
    {
        fclose(fp);
        m_listFiles.emplace_back(filename);
        return true;
    }

    return false;
}

void FileManager::addFile(const char* filename)
{
    std::lock_guard<mutex> guard(m_mtFile);
    m_listFiles.emplace_back(filename);
}
