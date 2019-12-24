#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

#include <cstdint>
#include <core/safe/safe_function.h>

template <typename Type>
struct __TLV
{
    Type _TYPE;
    uint32_t _LEN;
    __TLV()
        : _TYPE(0)
        , _LEN(0)
    {
        
    }
};

typedef char* tlv;

template <typename Type>
tlv __make_tlv__(const Type& type, const char* data, const uint32_t& len)
{
    tlv p = (tlv)malloc(sizeof(__TLV<Type>) + sizeof(char) * len);

    __TLV<Type>* p_tlv = (__TLV<Type>*)p;
    p_tlv->_TYPE = type;
    p_tlv->_LEN = len;
    char* p_val = p + sizeof(__TLV<Type>);
    if (data == nullptr) {
        memset(p_val, 0, len);
    } else {
        safe_memcpy(p_val, p_tlv->_LEN, data, len);
    }
    return p;
}

inline void __free_tlv__(tlv p_tlv)
{
    char* p = (char*)(p_tlv);
    free(p);
}

template <typename Type>
constexpr size_t tlv_min_len()
{
    return sizeof(__TLV<Type>);
}

template <typename Type>
tlv tlv_mk(const Type& type, const uint32_t& len)
{
    return __make_tlv__(type, nullptr, len);
}

template <typename Type>
tlv tlv_mk(const Type& type, const char* data, const uint32_t& len)
{
    return __make_tlv__(type, data, len);
}

inline void tlv_free(tlv p)
{
    __free_tlv__(p);
}

template <typename Type>
constexpr Type& tlv_type(tlv p)
{
    return ((__TLV<Type>*)(p))->_TYPE;
}

/* TLV数据长度 */
template <typename Type>
constexpr size_t tlv_len(tlv p)
{
    return ((__TLV<Type>*)(p))->_LEN;
}

/* TLV总长度，包括TLV头 */
template <typename Type>
constexpr size_t tlv_total_len(tlv p)
{
    return ((__TLV<Type>*)(p))->_LEN + sizeof(__TLV<Type>);
}

template <typename Type>
constexpr char* tlv_val(tlv p)
{
    return p + sizeof(__TLV<Type>);
}

