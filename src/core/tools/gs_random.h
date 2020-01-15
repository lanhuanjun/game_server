#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

#include <cstdint>
#include <climits>
#include <cstring>
#include <random>
#include <random>
#include <glog/logging.h>
#include <core/platform/os_macro.h>

#ifdef OS_WIN

#include <Windows.h>
#include <bcrypt.h>

#pragma comment(lib, "Bcrypt")
#endif
namespace gs
{
#ifdef OS_LINUX
    inline std::random_device g_linux_rand_dv;
#endif
    /* 生成随机数 [begin, end) begin <= end*/
    inline uint32_t rand(const uint32_t& begin, const uint32_t& end)
    {
        if (begin >= end) {
            return end;
        }
        uint32_t rand = 0;
#ifdef OS_WIN

        BYTE buf[sizeof(uint32_t)];
        if (0 != BCryptGenRandom(nullptr, buf, sizeof(uint32_t), 0x00000002)) {
            LOG(FATAL) << "ran error!";
        }
        else {
            memcpy_s(&rand, sizeof(uint32_t), buf, sizeof(uint32_t));
        }
#endif

#ifdef OS_LINUX
        rand = g_linux_rand_dv();
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