#pragma once
#include<unistd.h>
#include<mysql/mysql.h>
#include<mysql/errmsg.h>
#include<stdio.h>
#include<sys/types.h>
#include<errno.h>
#include<string>
#include "base/Logging.h"
#include "QueryResult.h"

using namespace std;

class MySQLTool
{
public:
    MySQLTool();
    ~MySQLTool();

    bool connect(const string &host, const string &user, const string &passwd, const string &db,unsigned port = 3306);

    QueryResultPtr Query(const string &sql);
    bool Execute(const string &sql);
    bool Execute(const string &sql, uint32_t &nAffectedCount, int &nErrno);
    const string GetDBName() const;

private:
    MYSQL *m_mysql;
    string m_host;
    string m_user;
    string m_passwd;
    string m_dbname;
    unsigned m_port;
};
