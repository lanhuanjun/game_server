#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.


#include <cstdint>
#include <string>
#include <vector>
#include <core/tools/buffer.h>
#include <core/safe/safe_recycle_list.h>
#include <core/safe/safe_list.h>
#include <core/safe/safe_map.h>
#include <core/platform/os_macro.h>

#ifdef OS_WIN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <MSWSock.h>
#include <Windows.h>

typedef SOCKET net_link;

#define INVALID_NET_LINK INVALID_SOCKET


#else

#include <sys/socket.h>

typedef int net_link;
#define INVALID_NET_LINK -1

#endif

/* Ĭ�Ϸ����������˿� */
#define DEFAULT_LISTEN_PORT 18110
/* ��С��ʹ�ö˿ڣ���Ҫʹ�ô��ڴ˶˿��� */
#define MIN_TCP_IP_PORT 5000
/* ��ϢĬ�ϻ����С */
#define NET_MSG_DEFAULT_BUFFER_SIZE 10240
/* Ĭ�Ϲ����߳���������1���߳����ڷ������ݣ������߳����ڽ������Ӻͽ������� */
#define NET_DEFAULT_WORK_THREAD_NUM 2
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
    NET_CREATE_EPOLL_FAIL = 10,
    NET_LINK_BROKEN = 11,
    NET_MSG_TOO_LARGE = 12,
};

enum NetIOOperation
{
    NET_IO_OPERATION_NONE = -1,
    NET_IO_OPERATION_ACCEPT = 0,
    NET_IO_OPERATION_READ = 1,
    NET_IO_OPERATION_WRITE = 2,
};

typedef fixed_buf<NET_MSG_DEFAULT_BUFFER_SIZE> net_io_buf;

inline const static uint32_t NET_MSG_MAX_LEN = NET_MSG_DEFAULT_BUFFER_SIZE * 10;
typedef fixed_buf<NET_MSG_MAX_LEN> net_msg_buf;
struct NetMsgBufList
{
    /* ����д���¼dataʵ��д����*/
    size_t count;
    const static int32_t MAX_MSG_NUM = 50;
    std::vector<net_msg_buf> data;
    NetMsgBufList()
        : count(0)
    {
        
    }
};
