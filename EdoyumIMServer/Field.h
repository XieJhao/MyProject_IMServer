#pragma once

#include<iostream>
#include<unistd.h>
#include<mysql/mysql.h>
#include<stdio.h>
#include<sys/types.h>
#include<errno.h>
#include<string>
#include<memory>
#include "base/Logging.h"

using namespace std;

class Field
{   
public:
    typedef enum
    {
        ENY_TYPE_NONE = 0,
        ENY_TYPE_STRING,
        ENY_TYPE_INTEGER,
        ENY_TYPE_FLOAT,
        ENY_TYPE_BOOL,
        ENY_TYPE_NULL
    }
    DataTypes;

public:
    Field();
    ~Field();

    void SetType(DataTypes tp);
    DataTypes GetType() const;
    void SetName(const string &name);
    const string &GetName() const;
    void SetValue(const char *value, size_t nLen);
    const string &GetValue() const;
    bool isNull() const;
    // TODO:整数、小数、字符串
    bool toBool() const;
    int8_t toInt8() const;
    uint8_t toUInt8() const;
    int32_t toInt32() const;
    uint32_t toUInt32() const;
    int64_t toInt64() const;
    uint64_t toUInt64() const;
    double toFloat() const;
    const string &GetString() const { return m_value; }

private:
    string m_value;
    string m_name;
    DataTypes m_type;
    bool m_isNull;
};
