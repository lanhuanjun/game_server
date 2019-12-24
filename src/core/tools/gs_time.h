#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.


#include <cstdint>
#include <core/platform/os_macro.h>
#ifdef OS_WIN
#include "Windows.h"
#endif

/* ���� */
typedef uint64_t msec_t;

/* �� */
typedef uint64_t sec_t;


//window tickʱ��Ϊ100����
#define WINDOW_TICK 10000000
//the windows epoch starts 1601 - 01 - 01T00:00 : 00Z.It's 11644473600 seconds before the UNIX/Linux epoch (1970-01-01T00:00:00Z). 
#define WINDOW_SEC_TO_UNIX_EPOCH 0x019DB1DED53E8000

#define ONE_MINUTE 60 //һ���ӵ�����
#define ONE_HOUR (60*ONE_MINUTE) //һСʱ������
#define ONE_DAY (24*ONE_HOUR) //һ�������
#define ONE_WEEK (7*ONE_DAY) //һ�ܵ�����
#define ONE_YEAR (365*ONE_DAY) //һ�������

#ifdef OS_WIN
inline sec_t file_time_to_unix_epoch(LPFILETIME file_time)
{
    LARGE_INTEGER i;
    i.HighPart = file_time->dwHighDateTime;
    i.LowPart = file_time->dwLowDateTime;
    return (i.QuadPart - WINDOW_SEC_TO_UNIX_EPOCH) / WINDOW_TICK;
}
#endif

/* ��ȡ��1970-01-01����ǰʱ�������  */
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

/* ��ȡ�����ĵ���ʱ�ĺ����� */
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