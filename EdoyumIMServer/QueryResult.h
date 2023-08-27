#pragma once

#include<iostream>
#include<unistd.h>
#include<mysql/mysql.h>
#include<stdio.h>
#include<sys/types.h>
#include<errno.h>
#include<string>
#include<memory>
#include <vector>
#include "base/Logging.h"
#include "Field.h"

using namespace std;

class QueryResult
{
public:
    QueryResult(MYSQL_RES* result,uint32_t rowcount,uint32_t cloumncout);
    ~QueryResult();
    //true表示还有下一行，并且切换成功；false表示没有了
    bool NextRow();
    Field* Fetch(){
        return m_CurrentRow.data();
    }

    const Field& operator[](int index)const
    {
        return this->m_CurrentRow[index];
    }

    const Field& operator[](const string &name) const;

    uint32_t GetFieldCount() const;
    uint32_t GetRowCount() const;
    vector<string> const &GetFieldName() const;
    void EndQuery();
    Field::DataTypes toEDYType(enum_field_types mysqltype) const;

protected:
    int GetFieldIndexByName(const string &name) const;

private:
    vector<Field> m_CurrentRow;
    vector<string> m_vecFieldName;
    MYSQL_RES *m_result;
    uint32_t m_rowcount;
    uint32_t m_cloumncout;
};

typedef shared_ptr<QueryResult> QueryResultPtr;
