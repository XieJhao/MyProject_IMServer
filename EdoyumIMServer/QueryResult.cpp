#include"QueryResult.h"
QueryResult::QueryResult(MYSQL_RES* result,uint32_t rowcount,uint32_t cloumncout)
    :m_result(result),m_rowcount(rowcount),m_cloumncout(cloumncout)
{
    m_CurrentRow.resize(m_cloumncout);
    m_vecFieldName.resize(m_cloumncout);
    //对于指定列，返回MYSQL_FIELD结构
    //对于结果集，返回所有MYSQL_FIELD结构的数组。
    //每个结构提供了结果集中1列的字段定义。
    MYSQL_FIELD *fields = mysql_fetch_fields(m_result);
    //检索结果集的下一行。
    MYSQL_ROW row = mysql_fetch_row(m_result);

    if (row == nullptr)
    {
        //TODO:结束查询
        EndQuery();
        return;
    }
    //返回结果集内当前行的列的长度。
    unsigned long *pFieldLength = mysql_fetch_lengths(m_result);
    for (uint32_t i = 0; i < m_cloumncout; i++)
    {
        m_vecFieldName[i] = fields[i].name;
        // TODO:设置type
        m_CurrentRow[i].SetType(toEDYType(fields[i].type));
        
        //TODO:设置列名
        m_CurrentRow[i].SetName(m_vecFieldName[i]);
        if (row[i] == nullptr)
        {
            //TODO:设置field的值为空
            m_CurrentRow[i].SetValue(nullptr, 0);
        }
        else
        {
            //TODO:设置field的值
            m_CurrentRow[i].SetValue(row[i], pFieldLength[i]);
            
        }
    }
}

QueryResult::~QueryResult()
{
    
}

bool QueryResult::NextRow()
{
    if(m_result == nullptr)
    {
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(m_result);
    if(row == nullptr)
    {
        EndQuery();
        return false;
    }

    unsigned long *pFieldLength = mysql_fetch_lengths(m_result);
    for (uint32_t i = 0; i < m_cloumncout; i++)
    {
        if(row[i] == nullptr)
        {
            //TODO:设置field的值为空
            m_CurrentRow[i].SetValue(nullptr, 0);
        }
        else
        {
            //TODO:设置field的值
            m_CurrentRow[i].SetValue(row[i], pFieldLength[i]);
            
        }

    }
    return true;
}

uint32_t QueryResult::GetFieldCount() const
{
    return m_cloumncout;
}

uint32_t QueryResult::GetRowCount() const
{
    return m_rowcount;
}

vector<string> const& QueryResult::GetFieldName() const
{
    return m_vecFieldName;
}

const Field& QueryResult::operator[](const string &name) const
{
    return m_CurrentRow[GetFieldIndexByName(name)];
}

void QueryResult::EndQuery()
{
    m_CurrentRow.clear();
    m_vecFieldName.clear();
    if (m_result)
    {
        mysql_free_result(m_result);
        m_result = nullptr;
    }
    m_cloumncout = 0;
    m_rowcount = 0;
}

Field::DataTypes QueryResult::toEDYType(enum_field_types mysqltype) const
{
    switch (mysqltype)
    {
        case FIELD_TYPE_TIMESTAMP:
        case FIELD_TYPE_DATE:
        case FIELD_TYPE_TIME:
        case FIELD_TYPE_DATETIME:
        case FIELD_TYPE_YEAR:
        case FIELD_TYPE_STRING:
        case FIELD_TYPE_VAR_STRING:
        case FIELD_TYPE_BLOB:
        case FIELD_TYPE_SET:
            return Field::ENY_TYPE_STRING;
        case FIELD_TYPE_NULL:
            return Field::ENY_TYPE_NULL;
        case FIELD_TYPE_TINY:
        case FIELD_TYPE_SHORT:
        case FIELD_TYPE_LONG:
        case FIELD_TYPE_LONGLONG:
        case FIELD_TYPE_INT24:
        case FIELD_TYPE_ENUM:
            return Field::ENY_TYPE_INTEGER;
        case FIELD_TYPE_DECIMAL:
        case FIELD_TYPE_FLOAT:
        case FIELD_TYPE_DOUBLE:
            return Field::ENY_TYPE_FLOAT;
        default:
            return Field::ENY_TYPE_NONE;

    }
    return Field::ENY_TYPE_NONE;
}

int QueryResult::GetFieldIndexByName(const string &name) const
{
    for (uint32_t i = 0; i < m_vecFieldName.size();i++)
        {
            if(m_vecFieldName[i] == name)
            {
                return i;
            }
        }
        return -1;
}
