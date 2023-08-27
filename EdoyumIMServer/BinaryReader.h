#pragma once
#include<iostream>
#include<string>
#include <string.h>

using namespace std;
namespace edoyun
{
    enum {
		TEXT_PACKLEN_LEN	= 4,
		TEXT_PACKAGE_MAXLEN	= 0xffff,
		BINARY_PACKLEN_LEN	= 2,
		BINARY_PACKAGE_MAXLEN	= 0xffff,

		TEXT_PACKLEN_LEN_2	= 6,
		TEXT_PACKAGE_MAXLEN_2	= 0xffffff,

		BINARY_PACKLEN_LEN_2 	= 4,//4字节头长度
		BINARY_PACKAGE_MAXLEN_2 = 0x10000000,//包最大长度是256M,足够了

		CHECKSUM_LEN		= 2,
	};

    class BinaryReader
    {
    public:
        static void dump(const string& buf)
        {
            dump(buf.c_str(), buf.size());
        }
        static void dump(const char* buf,size_t size)
        {
            for (size_t i = 0; i < size; i++)
            {
                if (i != 0 && ((i%60) == 0))
                {
                    cout << endl;
                }
                printf("%02X ", (unsigned)(buf[i]) & 0xFF);
            }
            cout << endl;
        }  
    public:
        BinaryReader()
        {
            m_index = 0;
        }
        BinaryReader(const BinaryReader& reader)
        {
            m_buffer = reader.m_buffer;
            m_index = reader.m_index;
        }
        BinaryReader(const string &buffer)
        {
            m_buffer = buffer;
            m_index = 0;
        }
        ~BinaryReader() = default;

        BinaryReader& operator=(const BinaryReader& reader)
        {
            if(this != &reader)
            {
                m_buffer = reader.m_buffer;
                m_index = reader.m_index;
            }
            return *this;
        }

        void UpdateBuffer(const string& buffer)
        {
            m_buffer = buffer;
            m_index = 0;
        }

        void Reset()
        {
            m_index = 0;
        }
        size_t Size()
        {
            return m_buffer.size();
        }

        bool ReadInt32(int32_t& data)
        {
            if((m_index + sizeof(int32_t)) >= m_buffer.size())
            {
                return false;
            }
            memcpy(&data, m_buffer.c_str() + m_index, sizeof(int32_t));
            m_index += sizeof(int32_t);
            return true;
        }

        template<class T>
        bool ReadData(T& data)
        {
            // cout << " 解析前 m_index=" << m_index << endl;
            if ((m_index + sizeof(T)) > m_buffer.size())
            {
                return false;
            }
            char *pData = (char *)&data;
            // uint32_t index = m_index;
            for (size_t i = 0; i < sizeof(T); i++)
            {
                pData[i] = *(m_buffer.c_str() + m_index + sizeof(T) - (i + 1));
            }

            // memcpy(&data, m_buffer.c_str() + m_index, sizeof(T));
            m_index += sizeof(T);
            // cout << "后m_index=" << m_index<<"  data="<<data << endl;
            return true;
        }
        

    private:
        string m_buffer;
        uint32_t m_index;
    };

    template <>
    bool BinaryReader::ReadData(string &data);

    class BinaryWriter
    {
    public:
        BinaryWriter():m_index(0){}
        ~BinaryWriter(){}

        BinaryWriter& operator=(const string& BW)
        {
            this->Clear();
            this->m_buffer = BW;
            // this->WriteData(BW);
            return *this;
        }

        template<class T>
        bool WriteData(const T& data)
        {
            // m_buffer.resize(m_buffer.size()+sizeof(T));
            m_buffer.resize(m_index + sizeof(T));
            memcpy((char *)m_buffer.c_str() + m_index,&data,sizeof(data));
            m_index += sizeof(T);
            return true;
        }

        uint32_t Size()
        {
            return m_index;
        }

        string toString() const
        {
            return m_buffer;
        }

        void Clear()
        {
            m_index = 0;
            m_buffer.clear();
        }

        void writerFile()
        {
            m_buffer.clear();
            char str[BINARY_PACKLEN_LEN_2 + CHECKSUM_LEN];
		    m_buffer.append(str,sizeof(str));
            m_index = sizeof(str);
        }

    protected:
        //压缩消息算法
        void Compress(size_t len,string& out)
        {
            char c = 0;
            if(len < 128)
            {
                c = (char)len & 0x7F;
                out += c;
                return;
            }
            //假定长度在百兆以内
            //输出时候不会超过5个字节,32个bit能够表达
            for (int i = 4; i >= 0; i--)
            {
                c = (len >> (7 * i)) & 0x7F;
                if(c == 0 && out.size() == 0)//目前还没有发现你有效数据都是0
                {
                    continue;
                }
                if(i > 0)//说明你是最后7位
                {
                    c |= 0x80;
                }
                out += c;
            }
        }

    private:
        string m_buffer;
        uint32_t m_index;
    };
    template <>
    bool BinaryWriter::WriteData(const string &data);
}
