#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.
#include <core/platform/os_macro.h>

#ifdef OS_WIN

#include "net_io_interface.h"

struct PerIOCtx;
struct PerSocketCtx;
class CNetIOWin32 : public INetIO
{
    using socket_ctx_iter = safe_recycle_list<PerSocketCtx>::iterator;
    using io_ctx_iter = safe_recycle_list<PerIOCtx>::iterator;
public:
    CNetIOWin32();
    virtual ~CNetIOWin32();
public:
    int SetListenPort(const uint16_t& port) override;
    int SetWorkThreadNum(const uint32_t& num) override;
    int Initiate() override;
    int Run() override;
    net_link AddConnect(const std::string& ip, const uint16_t& port) override;
    bool RecvData(net_link& socket, net_io_buf& recv_buf, const int32_t& time_out) override;
    int SendData(const net_link& socket, const char* data, const size_t& data_size) override;
    void CleanUp() override;
    static DWORD WINAPI RecvDataThread(LPVOID p_net_cs);
    static DWORD WINAPI SendDataThread(LPVOID p_net_cs);
private:
    // 创建工作线程
    int CreateThread();

    //  创建一个socket
    net_link CreateOneNetLink();
    // 创建服务端socket
    int CreateListenNetLink();
    // 创建一个等待客户端连接的socket
    int CreateClientNetLink();
    // 处理连接事件
    void DoAccept(PerIOCtx* p_io_ctx, DWORD io_size);
    // 处理发送事件
    void DoSend(PerIOCtx* p_io_ctx, DWORD io_size);
    // 处理读取事件
    void DoRead(PerIOCtx* p_io_ctx, DWORD io_size);

    /* 发送一个读取事件 */
    bool PostRead(socket_ctx_iter& socket_ctx);

    /* 发送一个写入事件
     * offset:相对于发送数据的基址偏移
     */
    bool PostWrite(io_ctx_iter& io_ctx, const size_t& offset = 0);

    /* 关闭一个client连接 */
    void CloseLink(net_link link);
private:
    // 完成端口
    HANDLE m_iocp;

    // 监听端口
    uint16_t m_port;

    // 工作线程数量
    uint32_t m_work_thread_num;

    // 接收线程数量
    uint32_t m_recv_work_thread_num;

    // 工作线程
    HANDLE m_work_threads[NET_MAX_WORK_THREAD_NUM];

    // 临界区
    CRITICAL_SECTION m_critical_section;

    // 监听socket
    net_link m_listen;

    // 所有连接的socket
    safe_recycle_list<PerSocketCtx> m_socket_ctx;

    // 所有的IO
    safe_recycle_list<PerIOCtx> m_io_ctx;

    // 等待处理的用户数据
    safe_list<io_ctx_iter> m_wait_detail;

    // 等待发送的用户数据
    safe_list<io_ctx_iter> m_wait_send;

    // 服务结束
    bool m_finish;

    // 客户端连接
    safe_map<net_link, socket_ctx_iter, false> m_active_link;

    // 读写控制
    WSAEVENT m_read_event;
    WSAEVENT m_write_event;
};

#endif
