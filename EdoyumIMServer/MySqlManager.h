#pragma once

#include"mysqltool.h"
#include<map>

typedef struct fieldinfo
    {
        fieldinfo() = default;
        fieldinfo(const string &name, const string &tp, const string &desc)
        {
            sName = name;
            sType = tp;
            sDesc = desc;
        }
        string sName;
        string sType;
        string sDesc;
    } sFieldInfo;

    typedef struct 
    {
        string sName;
        map<string,sFieldInfo> mapField;
        string sKey;
    } sTableInfo;

class MySqlManager
{
public:
    

public:
    MySqlManager();
    ~MySqlManager();
    bool Init(const string &host, const string user, const string passwd, const string dbname, unsigned port = 3306);
    QueryResultPtr Query(const string& sql);
    bool Excute(const string &sql);
    // TODO:业务实现时候追加

private:
    bool CheckDatabase();
    bool CheckTable(const sTableInfo& info);
    bool CreateDatabase();
    bool CreateTable(const sTableInfo& info);
    bool UpdateTable(const sTableInfo& info);

private:
    map<string, sTableInfo> m_mapTable;
    shared_ptr<MySQLTool> m_mysql;
};
typedef pair<string, sTableInfo> TablePair;
typedef map<string, sTableInfo>::iterator TableIter;
typedef map<string, sFieldInfo>::iterator FieldIter;
typedef map<string, sFieldInfo>::const_iterator FieldConstIter;
typedef pair<string, sFieldInfo> FieldPair;
