#include "mysqltool.h"

extern void  mysql_close(MYSQL *sock);

MySQLTool::MySQLTool()
{
    m_mysql = nullptr;
    m_port = 3306;
}

MySQLTool::~MySQLTool()
{
    if(m_mysql != nullptr)
    {
        MYSQL *tmp = m_mysql;
        m_mysql = nullptr;
        mysql_close(tmp);
    }
}

bool MySQLTool::connect(const string &host, const string &user, const string &passwd, const string &db,unsigned port)
{
    if(m_mysql != nullptr)
    {
        MYSQL *tmp = m_mysql;
        m_mysql = nullptr;
        mysql_close(tmp);
    }

    m_mysql = mysql_init(m_mysql);
    m_mysql = mysql_real_connect(m_mysql, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, nullptr, 0);
    cout << host << "\t"<< user << "\t"<< passwd << "\t"<< db << "\t"<<endl;

    if(m_mysql != nullptr)
    {
        m_host = host;
        m_user = user;
        m_passwd = passwd;
        m_dbname = db;
        mysql_query(m_mysql, "set name utf8;");
        cout << "connect mysql success!" << endl;
        return true;
    }

    cout << "connect mysql failed!\r\n";
    cout << "the error is " << mysql_errno(m_mysql) << endl;

    return false;
}

QueryResultPtr MySQLTool::Query(const string &sql)
{
    if(m_mysql == nullptr)
    {
        if(connect(m_host, m_user, m_passwd, m_dbname))
        {
            return nullptr;
        }
    }
    cout<<"(Query)  sql : " << sql << endl;
    // blob real_query 遇到\0不会认为是字符串结束
    int ret = mysql_real_query(m_mysql, sql.c_str(), sql.size());
    if(ret)
    {
        uint32_t nErrno = mysql_errno(m_mysql);
        cout << "(Query)mysql_real_query call failed! code is :" << nErrno << endl;

        if(CR_SERVER_GONE_ERROR == nErrno)//客户端无法向服务器发送请求
        {
            if(connect(m_host,m_user,m_passwd,m_dbname) == false)
            {
                return QueryResultPtr();
            }
            ret = mysql_real_query(m_mysql, sql.c_str(), sql.size());
            if(ret)
            {
                nErrno = mysql_errno(m_mysql);
                cout << "(Query)mysql_real_query call failed again ! code is:" << nErrno << endl;
                cout << "sql : " << sql << endl;
                return QueryResultPtr();
            }
        }
        else
        {
            return QueryResultPtr();
        }
    }

    //查询结果集
    MYSQL_RES* result = mysql_store_result(m_mysql);
    //返回上次数据变动行数
    uint32_t rowcount = mysql_affected_rows(m_mysql);
    //返回作用在连接上的最近查询的列数。
    uint32_t cloumcount = mysql_field_count(m_mysql);
    return QueryResultPtr(new QueryResult(result, rowcount, cloumcount));
}

bool MySQLTool::Execute(const string &sql)
{
    uint32_t nAffectedCount;
    int nErrno;

    return Execute(sql,nAffectedCount,nErrno);
}

bool MySQLTool::Execute(const string &sql, uint32_t &nAffectedCount, int &nErrno)
{
    if(m_mysql == nullptr)
    {
        if(connect(m_host, m_user, m_passwd, m_dbname))
        {
            return false;
        }
    }
    cout << "(Execute)sql : " << sql << endl;
    int ret = mysql_query(m_mysql, sql.c_str());

    if(ret)
    {
        uint32_t nErrno = mysql_errno(m_mysql);
        cout << "(Execute)mysql_query call failed! code is :" << nErrno << endl;
        
        if (CR_SERVER_GONE_ERROR == nErrno)
        {
            if(connect(m_host,m_user,m_passwd,m_dbname) == false)
            {
                return false;
            }
            ret = mysql_query(m_mysql, sql.c_str());
            if(ret)
            {
                nErrno = mysql_errno(m_mysql);
                cout << "(Execute)mysql_query call failed again ! code is:" << nErrno << endl;
                cout << "sql : " << sql << endl;
                return false;
            }
        }
        else
        {
            return false;
        }

        nErrno = 0;
        nAffectedCount = mysql_affected_rows(m_mysql);

    }
    return true;
}

const string MySQLTool::GetDBName() const
{
    return m_dbname;
}
