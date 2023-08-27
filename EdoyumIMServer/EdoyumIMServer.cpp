// EdoyumIMServer.cpp: 定义应用程序的入口点。
//

#include "EdoyumIMServer.h"
#include<signal.h>
#include "base/Singleton.h"
#include "IMServer.h"
#include "MySqlManager.h"
#include "UserManager.h"

using namespace std;



void show_help(const char* cmd)
{
	cout << "found error argument!\r\n" << endl;
	cout << "Usege: " << endl;
	cout << cmd << "[-d]" << endl;
	cout << "\t -d run in daemon mode\r\n" << endl;
}

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

void daemon()
{
	signal(SIGCHLD, SIG_IGN);
	int pid = fork();
	if(pid < 0)
	{
			cout << "fork call error,code is " << pid << " error code is " << errno << endl;
			exit(-1);
	}
	if(pid > 0)//主进程结束
	{
			exit(0);
	}
	//子进程代码
	//可以避免父进程所在会话结束时，把子进程带走
	//避免前台影响后台
	setsid();

	//避免后台影响前台
	int fd = open("/dev/null", O_RDWR, 0);
	cout << "invoke success !" << endl;
	cout << "STDIN_FILENO is " << STDIN_FILENO << endl;
	cout << "STDOUT_FILENO is " << STDOUT_FILENO << endl;
	cout << "STDERR_FILENO is " << STDERR_FILENO << endl;
	cout << "fd is " << fd << endl;
	if(fd != -1)
	{
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
	}
	if (fd > STDERR_FILENO)
	{
		close(fd);
	}
}

void onConnection(const muduo::net::TcpConnectionPtr& conn)
{
	cout << conn->name() << endl;
}

void onMessage(const muduo::net::TcpConnectionPtr& conn,
				muduo::net::Buffer* buf,muduo::Timestamp time)
{
	conn->send(buf);
	// conn->shutdown();
	// if(!conn->connected())
	// 	conn->getLoop()->quit();
}

void test(muduo::net::EventLoop& loop)
{
	muduo::net::InetAddress addr(9527);
	muduo::net::TcpServer server(&loop, addr,"echo server");
	server.setConnectionCallback(onConnection);
	server.setMessageCallback(onMessage);
	server.start();
}

int main(int argc,char* argv[],char* env[])
{
	signal(SIGCHLD, SIG_DFL);
	signal(SIGPIPE, SIG_IGN);//网络当中，管道操作
	signal(SIGINT, signal_exit);//中断错误
	signal(SIGKILL, signal_exit);
	signal(SIGTERM, signal_exit);//ctrl +c
	signal(SIGILL, signal_exit);//非法指令错误
	signal(SIGSEGV, signal_exit);//段错误
	signal(SIGTRAP, signal_exit);//Ctrl+break
	signal(SIGABRT, signal_exit);//abort函数调用触发
	cout << "EdoyumIMServer is invoking.... " << endl;
	int ch = 0;
	bool is_daemon = false;
	while((ch = getopt(argc, argv, "ab:c::d")) != -1)
	{
		cout << "ch = " << ch << endl;
		cout << "current " << optind - 1 << " value: " << argv[optind - 1]<<endl;
		switch (ch)
		{
			
			case 'd':
				is_daemon = true;
				break;
			default:
				show_help(argv[0]);
				exit(-1);
				break;
		}
	}

	if(is_daemon)
	{
		//TODO:开启守护进程
		daemon();
	}
	//TODO:开启服务器逻辑
	muduo::net::EventLoop loop;
	// test(loop);
	if(Singleton<MySqlManager>::instance().Init("127.0.0.1","debian-sys-maint","0ZaRwExruKSQPWuV","myim") == false)
	{
		cout << "database init error in main" << endl;
		return -2;
	}
	cout << __FILE__ << "(" << __LINE__ << ")" << endl;
	if(Singleton<UserManager>::instance().Init() == false)
	{
		cout << "load user failed !" << endl;
		return -3;
	}
	cout << __FILE__ << "(" << __LINE__ << ")" << endl;
	//单例解决两个问题：全局访问、单个实例
	if(Singleton<IMServer>::instance().Init("0.0.0.0",9527,&loop) == false)
	{
		cout << "server init error\r\n";
	}
	cout << __FILE__ << "(" << __LINE__ << ")" << endl;
	loop.loop();

	return 0;
}
