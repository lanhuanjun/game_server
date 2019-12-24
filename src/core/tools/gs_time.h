#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.


#include <cstdint>
#include <core/platform/os_macro.h>
#ifdef OS_WIN
#include "Windows.h"
#endif

/* 毫秒 */
typedef uint64_t msec_t;

/* 秒 */
typedef uint64_t sec_t;


//window tick时间为100纳秒
#define WINDOW_TICK 10000000
//the windows epoch starts 1601 - 01 - 01T00:00 : 00Z.It's 11644473600 seconds before the UNIX/Linux epoch (1970-01-01T00:00:00Z). 
#define WINDOW_SEC_TO_UNIX_EPOCH 0x019DB1DED53E8000

#define ONE_MINUTE 60 //一分钟的秒数
#define ONE_HOUR (60*ONE_MINUTE) //一小时的秒数
#define ONE_DAY (24*ONE_HOUR) //一天的秒数
#define ONE_WEEK (7*ONE_DAY) //一周的秒数
#define ONE_YEAR (365*ONE_DAY) //一年的秒数

#ifdef OS_WIN
inline sec_t file_time_to_unix_epoch(LPFILETIME file_time)
{
    LARGE_INTEGER i;
    i.HighPart = file_time->dwHighDateTime;
    i.LowPart = file_time->dwLowDateTime;
    return (i.QuadPart - WINDOW_SEC_TO_UNIX_EPOCH) / WINDOW_TICK;
}
#endif

/* 获取从1970-01-01到当前时间的秒数  */
inline sec_t now_time()
#ifdef OS_WIN
{
    FILETIME time;
    GetSystemTimeAsFileTime(&time);
    return file_time_to_unix_epoch(&time);
}
#else
{
    return time(NULL);
}
#endif

/* 获取开机的到此时的毫秒数 */
#ifdef OS_WIN
inline msec_t svc_run_msec()
{
    return GetTickCount64();
}
#endif
#ifdef OS_LINUX
inline struct timespec g_svc_run_time_temp;
inline msec_t svc_run_msec()
{
    clock_gettime(CLOCK_MONOTONIC_COARSE, &g_svc_run_time_temp);
    return (g_svc_run_time_temp.tv_sec * 1000 + g_svc_run_time_temp.tv_nsec / 1000000);
}
#endif