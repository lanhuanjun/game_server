#include "binary_stream_test.h"
#include "core/stream/binary_stream.h"

struct DemoStruct : public ISerialize
{
    int32_t m_int;
    std::string m_str;
    DemoStruct()
        : m_int(0)
    {
        
    }
    BINARY_SERIALIZE_DEF(m_int<<m_str)
    BINARY_DESERIALIZE_DEF(m_int>>m_str)
};

void test_stream_1()
{
    CBinaryStream in;
    std::string hello = "hello world!";
    in << hello;

    CBinaryStream out(in.data(), in.size());
    std::string word;
    out >> word;

}

void binary_stream_test()
{
    test_stream_1();
    CBinaryStream in;

    std::map<int, int> m_data;
    m_data.insert({ 1, 1 });
    m_data.insert({ 2, 2 });
    m_data.insert({ 3, 3 });
    m_data.insert({ 4, 4 });
    in << m_data;
    DemoStruct m_struct;
    m_struct.m_int = 1001;
    m_struct.m_str = "hello world";
    in << m_struct;

    CBinaryStream out(in.data(), in.size());
    m_data.clear();
    m_struct.m_int = 0;
    m_struct.m_str = "";
    out >> m_data;
    out >> m_struct;
}