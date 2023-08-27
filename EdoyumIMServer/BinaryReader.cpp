#include "BinaryReader.h"

namespace edoyun
{
    template<>
    bool BinaryReader::ReadData(string& data)
    {
        const char *pcur = m_buffer.c_str() + m_index;
        int lenght = 0;
        size_t i = 0;
        for (; i < m_buffer.size() - m_index; i++)
        {
            lenght <<= 7;
            lenght |= pcur[i] & 0x7F;
            if ((pcur[i] & 0x80) == 0)
            {
                break;
            }
        }
        m_index += i + 1;
        // cout << "m_index=" << m_index << endl;
        data.assign(m_buffer.c_str() + m_index, lenght);

        // memcpy((void *)data.c_str(), m_buffer.c_str() + m_index, data.size());
        m_index += lenght;
        // cout << "m_index=" << m_index <<"  data="<<data<< endl;
        return true;
    }


    template<>
    bool BinaryWriter::WriteData(const string& data)
    {

        string out;
        Compress(data.size(), out);
        m_buffer.append(out.c_str(),out.size());
        m_index += out.size();
        // cout << "计算包大小" << endl;
        // BinaryReader::dump(m_buffer);
        // cout << "WriteData<string> m_buffer = " << m_buffer << "  m_index = " << m_index << endl;
        if (data.size() > 0)
        {
            m_buffer.append(data.c_str(),data.size());
            m_index += data.size();
        }
        // cout << "添加全部" << endl;
        // BinaryReader::dump(m_buffer);
        // cout << "WriteData<string>2 m_buffer = " << m_buffer << "  m_index = " << m_index << endl;
        return true;
    }
}