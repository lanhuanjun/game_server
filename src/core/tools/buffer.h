#pragma once
/*****************************************************************************\
    *  @COPYRIGHT NOTICE
    *  @Copyright (c)2019 - 2030 lanyeo
    *  @file	 : buffer.h
    *  @version  : ver 1.0
    
    *  @author   : lanyeo
    *  @date     : 2019年10月24日 12:00:00
    *  @brief    : 
\*****************************************************************************/

#include <cstring>


/* 固定大小的缓冲区 */
struct fixed_buf final
{
    const size_t LEN;
    char* data;
    size_t used; // 数据量的大小
    size_t offset; // 定义一个偏移量
    fixed_buf(const size_t& init_size = 1024)
        : LEN(init_size)
        , data(nullptr)
        , used(0)
        , offset(0)
    {
        data = new char[LEN]();
    }
    fixed_buf(const fixed_buf& _cp)
        : LEN(_cp.LEN)
        , used(0)
    {
        data = new char[LEN]();
        memcpy_s(data, LEN * sizeof(char), _cp.data, _cp.LEN * sizeof(char));
        used = _cp.used;
        offset = _cp.offset;
    }
    fixed_buf& operator =(const fixed_buf& _cp)
    {
        data = new char[LEN]();
        memcpy_s(data, LEN * sizeof(char), _cp.data, _cp.LEN * sizeof(char));
        used = _cp.used;
        offset = _cp.offset;
        return *this;
    }
    fixed_buf(const fixed_buf&& _mv) noexcept
        : LEN(_mv.LEN)
    {
        this->data = _mv.data;
        this->used = _mv.used;
        this->offset = _mv.offset;
    }
    ~ fixed_buf()
    {
        delete data;
    }
    void reset()
    {
        memset(data, 0, LEN * sizeof(char));
        used = 0;
        offset = 0;
    }
};