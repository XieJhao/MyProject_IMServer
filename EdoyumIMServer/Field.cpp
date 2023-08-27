#include "Field.h"

Field::Field()
    :m_type(ENY_TYPE_NULL),m_isNull(true)
{
    
}

Field::~Field()
{
    
}
void Field::SetType(DataTypes tp)
{
    m_type = tp;
}

Field::DataTypes Field::GetType() const
{
    return m_type;
}

void Field::SetName(const string &name)
{
    m_name = name;
}

const string& Field::GetName() const
{
    return m_name;
}

void Field::SetValue(const char *value, size_t nLen)
{
    if(value == nullptr || nLen == 0)
    {
        m_isNull = true;
        m_value.clear();
        return;
    }
    // m_value.resize(nLen);
    // memcpy((void*)m_value.c_str(), value, nLen);
    //替换，用指定内容替换原有内容
    m_value.assign(value, nLen);
    m_isNull = false;
}

const string& Field::GetValue() const
{
    return m_value;
}

bool Field::isNull() const
{
    return m_isNull;
}

bool Field::toBool() const
{
    return atoi(m_value.c_str()) != 0;
}

int8_t Field::toInt8() const
{
    return static_cast<int8_t>(atoi(m_value.c_str()));
}

uint8_t Field::toUInt8() const
{
    return (uint8_t)static_cast<int8_t>(atoi(m_value.c_str()));
}

int32_t Field::toInt32() const
{
    return atoi(m_value.c_str());
}

uint32_t Field::toUInt32() const
{
    return (uint32_t)atoi(m_value.c_str());
}

int64_t Field::toInt64() const
{
    return atoll(m_value.c_str());
}

uint64_t Field::toUInt64() const
{
    return (uint64_t)atoll(m_value.c_str());
}

double Field::toFloat() const
{
    return atof(m_value.c_str());
}
