#pragma once
/*****************************************************************************\
    *  @COPYRIGHT NOTICE
    *  @Copyright (c)2019 - 2030 lanyeo
    *  @file	 : net_io_info.h
    *  @version  : ver 1.0
    
    *  @author   : lanyeo
    *  @date     : 2019��10��23�� 12:00:00
    *  @brief    : 
\*****************************************************************************/
#include <cstdint>
#include <string>
#include "core/tools/buffer.h"
#include "core/safe/safe_recycle_list.h"
#include "core/safe/safe_list.h"
#include "core/safe/safe_map.h"
#include "core/platform/os_macro.h"

#ifdef OS_WIN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <MSWSock.h>
#include <Windows.h>

typedef SOCKET net_link;

#define INVALID_NET_LINK INVALID_SOCKET


#endif

/* Ĭ�Ϸ����������˿� */
#define DEFAULT_LISTEN_PORT 18110
/* ��С��ʹ�ö˿ڣ���Ҫʹ�ô��ڴ˶˿��� */
#define MIN_TCP_IP_PORT 5000
/* ��ϢĬ�ϻ����С */
#define NET_MSG_DEFAULT_BUFFER_SIZE 8192
/* Ĭ�Ϲ����߳���������1���߳����ڷ������ݣ������߳����ڽ������Ӻͽ������� */
#define NET_DEFAULT_WORK_THREAD_NUM 2
/* ������߳��� */
#define NET_MAX_WORK_THREAD_NUM 32
/* ���������߳��뷢���̵߳ı��� X:1 */
#define RECV_WORK_THREAD_RATIO 3

enum NetIOCode
{
    NET_FAIL = -1,
    NET_OK = 0,
    NET_INIT_FAIL = 1,
    NET_CREATE_IOCP_FAIL = 2,
    NET_CREATE_THREAD_FAIL = 3,
    NET_VAL_INCORRECT = 4,
    NET_STATUS_ERROR = 5,
    NET_LISTEN_PORT_FAIL = 6,
    NET_CREATE_SOCKET_FAIL = 7,
    NET_UPDATE_IOCP_FAIL = 8,
    NET_CLIENT_CLOSED = 9,
};

enum NetIOOperation
{
    NET_IO_OPERATION_NONE = -1,
    NET_IO_OPERATION_ACCEPT = 0,
    NET_IO_OPERATION_READ = 1,
    NET_IO_OPERATION_WRITE = 2,
};

typedef fixed_buf net_io_buf;