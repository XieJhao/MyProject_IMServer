#include<iostream>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include"../base/Logging.h"
#include"../base/Singleton.h"
#include"../net/EventLoop.h"
#include"../net/EventLoopThreadPool.h"
#include"FileManager.h"
#include"FileServer.h"

using namespace std;



void signal_exit(int signum)
{
	cout<<"signal " << signum << " found ,exit... \r\n";
	//TODO:退出清除
	switch (signum)
	{
		case SIGINT:
		case SIGKILL:
		case SIGTERM:
		case SIGILL:
		case SIGSEGV:
		case SIGTRAP:
		case SIGABRT:
		//TODO:
			break;
		default:
		//TODO:
			break;
	}
	exit(signum);
}


// void prog_exit(int signo)
// {
//     cout << "program recv signal [" << signo << "] to exit" << endl;

//     g_mainLoop.quit();
// }

void daemon_run()
{
    int pid;
    signal(SIGCHLD, SIG_IGN);
    //1）在父进程中，fork返回新创建子进程的进程ID；
    //2）在子进程中，fork返回0；
    //3）如果出现错误，fork返回一个负值；
    pid = fork();
    if(pid < 0)
    {
        cout << " fork error" << endl;
        exit(-1);
    }
    else if (pid > 0)//父进程退出，子进程独立运行
    {
        exit(0);
    }
    //之前parent和child运行在同一个session里,parent是会话（session）的领头进程,
    //parent进程作为会话的领头进程，如果exit结束执行的话，那么子进程会成为孤儿进程，并被init收养。
    //执行setsid()之后,child将重新获得一个新的会话(session)id。
    //这时parent退出之后,将不会影响到child了。
    setsid();
    int fd;
    fd = open("/dev/null", O_RDWR, 0);
    if(fd != -1)
    {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
    }
    if(fd > 2)
    {
        close(fd);
    }
}

int main(int argc,  char  *argv[])
{
    cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    //设置信号处理
    signal(SIGCHLD, SIG_DFL);
	signal(SIGPIPE, SIG_IGN);//网络当中，管道操作
	signal(SIGINT, signal_exit);//中断错误
	signal(SIGKILL, signal_exit);
	signal(SIGTERM, signal_exit);//ctrl +c
	signal(SIGILL, signal_exit);//非法指令错误
	signal(SIGSEGV, signal_exit);//段错误
	signal(SIGTRAP, signal_exit);//Ctrl+break
	signal(SIGABRT, signal_exit);//abort函数调用触发

    cout << __FILE__ << "(" << __LINE__ << ")" << endl;

    // short port = 0;
    int ch;
    bool bdaemon = false;
    while ((ch = getopt(argc,argv,"d")) != -1)
    {
        switch (ch)
        {
        case 'd':
            bdaemon = true;
            break;

        default:
            break;
        }
    }
    cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    if(bdaemon)
    {
        daemon_run();
    }
    cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    // if(port == 0)
    // {
    //     port = 123456;
    // }
    cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    Logger::setLogLevel(Logger::DEBUG);

    EventLoop g_mainLoop;

    if(!Singleton<FileManager>::instance().Init("./filecache"))
    {
        cout << __FILE__ << "(" << __LINE__ << ") FileManger::Init() is error" << endl;
        return -1;
    }
    EventLoopThreadPool threadPool(&g_mainLoop, "fileserver");
    threadPool.setThreadNum(6);
    threadPool.start();

    if(!Singleton<FileServer>::instance().Init("0.0.0.0", 8001 ,&g_mainLoop))
    {
        cout << __FILE__ << "(" << __LINE__ << ") FileServer::Init() is error" << endl;
        return -1;
    }

    g_mainLoop.loop();

    return 0;
}

