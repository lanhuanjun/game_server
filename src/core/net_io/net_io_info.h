#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.


#include <cstdint>
#include <string>
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

/* 默认服务器监听端口 */
#define DEFAULT_LISTEN_PORT 18110
/* 最小的使用端口，需要使用大于此端口数 */
#define MIN_TCP_IP_PORT 5000
/* 消息默认缓冲大小 */
#define NET_MSG_DEFAULT_BUFFER_SIZE 8192
/* 默认工作线程数，其中1个线程用于发送数据，其他线程用于建立连接和接收数据 */
#define NET_DEFAULT_WORK_THREAD_NUM 2
/* 最大工作线程数 */
#define NET_MAX_WORK_THREAD_NUM 32
/* 接收数据线程与发送线程的比例 X:1 */
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
};

enum NetIOOperation
{
    NET_IO_OPERATION_NONE = -1,
    NET_IO_OPERATION_ACCEPT = 0,
    NET_IO_OPERATION_READ = 1,
    NET_IO_OPERATION_WRITE = 2,
};

typedef fixed_buf net_io_buf;