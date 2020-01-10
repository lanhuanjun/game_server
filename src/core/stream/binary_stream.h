#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

#include "stream.h"


#include <cstdint>
#include <vector>
#include <list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

#include <glog/logging.h>
#include <core/tools/gs_assert.h>
#include <core/safe/safe_function.h>

#define __BINARY_STREAM_BASE_TYPE__(TYPE)   \
CBinaryStream& operator <<(const TYPE& v)   \
{                                           \
    write((char*)(&v), sizeof(TYPE));       \
    return *this;                           \
}                                           \
CBinaryStream& operator <<(const TYPE* v)   \
{                                           \
    LOG_IF(FATAL, v == nullptr);            \
    write((char*)v, sizeof(TYPE));          \
    return *this;                           \
}                                           \
                                            \
CBinaryStream& operator >>(TYPE& v)         \
{                                           \
    read((char*)(&v), sizeof(TYPE));        \
    return *this;                           \
}                                           \
CBinaryStream& operator >>(TYPE* v)         \
{                                           \
    LOG_IF(FATAL, v == nullptr);            \
    write((char*)v, sizeof(TYPE));          \
    return *this;                           \
}                                           \

#define BINARY_SERIALIZE_DEF(DATA)                                  \
void __to__(IStream* stream) const override                         \
{                                                                   \
    CBinaryStream sr;                                               \
    sr << DATA;                                                     \
    stream->write((char*)(&sr.size()), sizeof(size_t));             \
    stream->write(sr.data(), sr.size());                            \
}                                                                   \

#define BINARY_DESERIALIZE_DEF(DATA)                                \
void __from__(IStream* stream) override                             \
{                                                                   \
    size_t len = 0;                                                 \
    stream->read((char*)(&len), sizeof(size_t));                    \
    if (len > 0) {                                                  \
        char* p_bin = (char*)calloc(len, sizeof(char));             \
        stream->read(p_bin, len);                                   \
        CBinaryStream sr(p_bin, len, true);                         \
        sr >> DATA;                                                 \
        free(p_bin);                                                \
    }                                                               \
}                                                                   \



class CBinaryStream : public IStream
{
public:
    /* 默认初始容量大小 */
    inline const static uint32_t __DEFAULT_CAPACITY = 128;
    /* 每次增加的容量大小 */
    inline const static uint32_t __INC_CAPACITY = 64;

    
    explicit CBinaryStream(const unsigned int& cap = __DEFAULT_CAPACITY)
    {
        m_buf = (char*)malloc(sizeof(char) * cap);
        memset(m_buf, 0, sizeof(char) * cap);
        m_cap = cap;
        m_size = 0;
        m_offset = 0;
        m_buf_is_delegate = false;
    }
    
    explicit CBinaryStream(const char* data, const size_t& len)
    {
        m_buf = (char*)malloc(sizeof(char) * len);
        safe_memcpy(m_buf, sizeof(char) * len, data, sizeof(char) * len);
        m_cap = len;
        m_offset = 0;
        m_size = len;
        m_buf_is_delegate = false;
    }
    /*
     * delegate_data:是否只是接管外部data,用于序列化
     */
    explicit CBinaryStream(char* data, const size_t& len, const bool& delegate_data)
    {
        if (delegate_data) {
            m_buf = data;
        } else {
            m_buf = (char*)malloc(sizeof(char) * len);
            safe_memcpy(m_buf, sizeof(char) * len, data, sizeof(char) * len);
        }
        m_cap = len;
        m_offset = 0;
        m_size = len;
        m_buf_is_delegate = delegate_data;
    }
    CBinaryStream(const CBinaryStream& cp) = delete;
    CBinaryStream operator = (const CBinaryStream& cp) = delete;
    CBinaryStream(CBinaryStream&& cp) = delete;
    virtual ~CBinaryStream()
    {
        if (!m_buf_is_delegate) {
            free(m_buf);
            m_buf = nullptr;
        }
    };
public:
    const size_t& size()
    {
        return m_size;
    }
    const size_t& capacity()
    {
        return m_cap;
    }
    void reserve(const size_t& cap)
    {
        if (cap < m_cap) {
            return ;
        }
        char* p_new = (char*)realloc(m_buf, sizeof(char) * cap);
        if (p_new == nullptr) {
            return;
        }
        m_cap = cap;
        m_buf = p_new;
    }
    void resize(const size_t& len)
    {
        if (len < m_cap) {
            m_buf = (char*)realloc(m_buf, sizeof(char) * len);
        }
        else {
            char* p_new = (char*)realloc(m_buf, sizeof(char) * len);
            if (p_new == nullptr) {
                return;
            }
            m_buf = p_new;
        }
        if (len < m_size) {
            m_size = len;
        }
        if (len < m_offset) {
            m_offset = len;
        }
        m_cap = len;
    }
    const char* data()
    {
        return m_buf;
    }

    std::string str()
    {
        return std::string(m_buf, m_size);
    }

    void write(const char* data, const size_t& len) override
    {
        if (m_cap - m_size < len) {
            // 空间不够先申请空间
            reserve(len + m_size + __INC_CAPACITY);
        }
        safe_memcpy(m_buf + m_offset, m_cap - m_offset, data, len);
        m_offset += len;
        m_size += len;
    }
    void read(char* data, const size_t& len) override
    {
        if (m_offset > m_size) {
            return;
        }
        safe_memcpy(data, len, m_buf + m_offset, len);
        m_offset += len;
    }
public:
    /* 自定义数据类型 */
    template <typename T>
    CBinaryStream& operator <<(const T& v)
    {
        CBinaryStream* me = this;
        v.__to__((IStream*)me);
        return *this;
    }
    template <typename T>
    CBinaryStream& operator <<(const T* v)
    {
        v->__to__((IStream*)this);
        return *this;
    }
    template <typename T>
    CBinaryStream& operator >>(T& v)
    {
        v.__from__((IStream*)this);
        return *this;
    }
    template <typename T>
    CBinaryStream& operator >>(T* v)
    {
        v->__from__((IStream*)this);
        return *this;
    }

    /* 基础数据类型 */
    __BINARY_STREAM_BASE_TYPE__(bool)
    __BINARY_STREAM_BASE_TYPE__(char)
    __BINARY_STREAM_BASE_TYPE__(float)
    __BINARY_STREAM_BASE_TYPE__(double)
    __BINARY_STREAM_BASE_TYPE__(int8_t)
    __BINARY_STREAM_BASE_TYPE__(int16_t)
    __BINARY_STREAM_BASE_TYPE__(int32_t)
    __BINARY_STREAM_BASE_TYPE__(int64_t)
    __BINARY_STREAM_BASE_TYPE__(uint8_t)
    __BINARY_STREAM_BASE_TYPE__(uint16_t)
    __BINARY_STREAM_BASE_TYPE__(uint32_t)
    __BINARY_STREAM_BASE_TYPE__(uint64_t)

    /* STL数据类型 */
    CBinaryStream& operator <<(const std::string& val)
    {
        *this << val.length();
        if (val.length() > 0) {
            write(val.data(), val.length());
        }
        return *this;
    }
    CBinaryStream& operator >>(std::string& val)
    {
        std::string::size_type len = 0;
        *this >> len;
        if (len > 0) {
            val.append(m_buf + m_offset, len);
        }
        m_offset += len;
        return *this;
    }
    template <typename T>
    CBinaryStream& operator <<(const std::vector<T>& val);
    template <typename T>
    CBinaryStream& operator >>(std::vector<T>& val);
    template <typename T>
    CBinaryStream& operator <<(const std::list<T>& val);
    template <typename T>
    CBinaryStream& operator >>(std::list<T>& val);
    template <typename T>
    CBinaryStream& operator <<(const std::set<T>& val);
    template <typename T>
    CBinaryStream& operator >>(std::set<T>& val);
    template <typename T>
    CBinaryStream& operator <<(const std::unordered_set<T>& val);
    template <typename T>
    CBinaryStream& operator >>(std::unordered_set<T>& val);

    template<typename Key, typename Val>
    CBinaryStream& operator <<(const std::map<Key, Val>& data);
    template<typename Key, typename Val>
    CBinaryStream& operator >>(std::map<Key, Val>& data);
    template<typename Key, typename Val>
    CBinaryStream& operator <<(const std::unordered_map<Key, Val>& data);
    template<typename Key, typename Val>
    CBinaryStream& operator >>(const std::unordered_map<Key, Val>& data);

    template <typename ...Args>
    void from(const Args& ...args)
    {
        (((*this) << args), ...);
    }
    template <typename ...Args>
    void to(Args& ...args)
    {
        (((*this) >> args), ...);
    }
private:
    char* m_buf;
    size_t m_size;//数据大小
    size_t m_cap;// 容量大小
    size_t m_offset;//读取到的位置
    bool m_buf_is_delegate;// buf是否是外部
};

template <typename T>
CBinaryStream& CBinaryStream::operator<<(const std::vector<T>& val)
{
    *this << val.size();
    if (val.empty()) {
        return *this;
    }
    for (const T& i : val) {
        *this << i;
    }
    return *this;
}
template <typename T>
CBinaryStream& CBinaryStream::operator>>(std::vector<T>& val)
{
    typename std::vector<T>::size_type size = 0;
    *this >> size;
    if (0 != size) {
        T tmp;
        for (unsigned int i = 0; i < size; ++i) {
            *this >> tmp;
            val.emplace_back(tmp);
        }
    }
    return *this;
}
template <typename T>
CBinaryStream& CBinaryStream::operator<<(const std::list<T>& val)
{
    *this << val.size();
    if (val.empty()) {
        return *this;
    }
    for (const T& i : val) {
        *this << i;
    }
    return *this;
}

template <typename T>
CBinaryStream& CBinaryStream::operator>>(std::list<T>& val)
{
    typename std::list<T>::size_type size = 0;
    *this >> size;
    if (0 != size) {
        T tmp;
        for (unsigned int i = 0; i < size; ++i) {
            *this >> tmp;
            val.emplace_back(tmp);
        }
    }
    return *this;
}
template <typename T>
CBinaryStream& CBinaryStream::operator<<(const std::set<T>& val)
{
    *this << val.size();
    if (val.empty()) {
        return *this;
    }
    for (const T& i : val) {
        *this << i;
    }
    return *this;
}

template <typename T>
CBinaryStream& CBinaryStream::operator>>(std::set<T>& val)
{
    typename std::set<T>::size_type size = 0;
    *this >> size;
    if (0 != size) {
        T tmp;
        for (unsigned int i = 0; i < size; ++i) {
            *this >> tmp;
            val.emplace(tmp);
        }
    }
    return *this;
}
template <typename T>
CBinaryStream& CBinaryStream::operator<<(const std::unordered_set<T>& val)
{
    *this << val.size();
    if (val.empty()) {
        return *this;
    }
    for (const T& i : val) {
        *this << i;
    }
    return *this;
}

template <typename T>
CBinaryStream& CBinaryStream::operator>>(std::unordered_set<T>& val)
{
    typename std::unordered_set<T>::size_type size = 0;
    *this >> size;
    if (0 != size) {
        T tmp;
        for (unsigned int i = 0; i < size; ++i) {
            *this >> tmp;
            val.emplace(tmp);
        }
    }
    return *this;
}

template <typename Key, typename Val>
CBinaryStream& CBinaryStream::operator<<(const std::map<Key, Val>& data)
{
    *this << data.size();
    if (data.empty()) {
        return *this;
    }
    for (std::pair<Key, Val>&& i : data) {
        *this << i.first << i.second;
    }
    return *this;
}

template <typename Key, typename Val>
CBinaryStream& CBinaryStream::operator>>(std::map<Key, Val>& data)
{
    typename std::map<Key, Val>::size_type size = 0;
    *this >> size;
    if (0 == size) {
        return *this;
    }
    std::pair<Key, Val> tmp;
    for (unsigned int i = 0; i < size; ++i) {
        *this >> tmp.first >> tmp.second;
        data.emplace(tmp);
    }
    return *this;
}

template <typename Key, typename Val>
CBinaryStream& CBinaryStream::operator<<(const std::unordered_map<Key, Val>& data)
{
    *this << data.size();
    if (data.empty()) {
        return *this;
    }
    for (std::pair<Key, Val>&& i : data) {
        *this << i.first << i.second;
    }
    return *this;
}

template <typename Key, typename Val>
CBinaryStream& CBinaryStream::operator>>(const std::unordered_map<Key, Val>& data)
{
    typename std::unordered_map<Key, Val>::size_type size = 0;
    *this >> size;
    if (0 == size) {
        return *this;
    }
    std::pair<Key, Val> tmp;
    for (unsigned int i = 0; i < size; ++i) {
        *this >> tmp.first >> tmp.second;
        data.emplace(tmp);
    }
    return *this;
}