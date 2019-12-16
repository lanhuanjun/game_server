#pragma once
//*****************************************************************************\
// Copyright (c) 2019 lanyeo
// All rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 
// Author   : lanyeo

#include <cstdint>
#include <cstring>
#include <glog/logging.h>

#ifdef _WIN32

#include <Windows.h>
#include <bcrypt.h>

#pragma comment(lib, "Bcrypt")
#endif
namespace gs
{
    
    /* 生成随机数 [begin, end) begin <= end*/
    inline uint32_t rand(const uint32_t& begin, const uint32_t& end)
    {
        if (begin >= end) {
            return end;
        }
        uint32_t rand = 0;
#ifdef _WIN32

        BYTE buf[sizeof(uint32_t)];
        if (0 != BCryptGenRandom(nullptr, buf, sizeof(uint32_t), 0x00000002)) {
            LOG(FATAL) << "ran error!";
        }
        else {
            memcpy_s(&rand, sizeof(uint32_t), buf, sizeof(uint32_t));
        }
#elif

#endif

        return rand % (end - begin) + begin;
    }
    /* 生成随机数 [0,INT_MAX) */
    inline uint32_t rand()
    {
        return rand(0, INT_MAX);
    }
    /* 生成随机数 [0, end) */
    inline uint32_t rand(const uint32_t& end)
    {
        return rand(0, end);
    }
    
}

/* 生成连续的64位无符号整型 */
class CUInt64SeqGenerator final
{
public:
    CUInt64SeqGenerator(const int64_t& begin = 0)
        : m_num(begin)
    {
        
    }
    uint64_t next()
    {
        if (m_num == UINT64_MAX) {
            m_num = 0;
        }
        else {
            m_num++;
        }
        return m_num;
    }
private:
    uint64_t m_num;
};