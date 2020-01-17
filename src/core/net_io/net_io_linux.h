#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.
#include <core/platform/os_macro.h>

#ifdef OS_LINUX

#include "net_io_interface.h"

#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>

#include <core/safe/safe_list.h>
#include <core/safe/safe_recycle_list.h>
#include <core/safe/safe_map.h>

typedef int32_t epoll_t;

class PerIOCtx;
class PerSocketCtx;
class CNetIOLinux : public INetIO
{
    using PerIOCtxIter = safe_recycle_list<PerIOCtx>::iterator;
    using PerSocketCtxIter = safe_recycle_list<PerSocketCtx>::iterator;
public:
    CNetIOLinux();
    ~CNetIOLinux();
    /*
     * 设置服务器监听端口 默认端口为 DEFAULT_LISTEN_PORT, 且值不能小于MIN_TCP_IP_PORT
     */
    int SetListenPort(const uint16_t& port) override;


    /**
     * 设置工作线程数量，最大不能超过NET_MAX_WORK_THREAD_NUM，
     * (num<2)表示使用PC逻辑处理器数量,
     * 默认NET_DEFAULT_WORK_THREAD_NUM。
     * 其中接收线程和发送线程比例大概是3：1
     * 0：成功 其他：错误
     */
    int SetWorkThreadNum(const uint32_t& num) override;


    /**
     * 初始化套接字等工作
     * 0：成功 其他：失败
     */
    int Initiate() override;


    /**
     * 运行网络环境
     * 0：成功 其他：失败
     */
    int Run() override;

    /*
     * 连接到的服务器，该方法调用需要在Run之后
     * 返回一个net_link
     */
    net_link AddConnect(const std::string& ip, const uint16_t& port) override;

    /**
     * 获取接收数据, 该线程会阻塞线程执行，可以设置超时时间，参考参数time_out
     * time_out 超时时间,单位毫秒, 小于0表示永不超时，一直阻塞进行执行
     * return true有数据，false没有数据
     */
    bool RecvData(net_link& socket, NetMsgBufList& recv_buf, const int32_t& time_out = -1) override;

    /**
     * 发送数据到某个socket
     */
    int SendData(const net_link& socket, const char* data, const uint32_t& data_size) override;
    /**
     * 清理网络环境
     */
    void CleanUp() override;

    static void AcceptThread(CNetIOLinux* pThis);
    static void RecvDataThread(CNetIOLinux* pThis, epoll_t epoll);
    static void SendDataThread(CNetIOLinux* pThis);
private:
    bool CreateAcceptEpoll();
    bool CreateRecvEpoll();
    bool CreateListenSocket();
    bool CreateThread();
    bool SetSocketNonblocking(net_link sd);
    bool AddSocketToEpoll(net_link sd, epoll_t epoll);
    void DoError(PerSocketCtxIter sd_ctx);
    void DoRead(PerSocketCtxIter sd_ctx);
    void PostRead(PerSocketCtxIter sd_ctx);
    void DoWrite(PerIOCtxIter io_ctx);
    void DoAccept();
private:
    /* 监听的本地端口 */
    uint16_t m_port;

    net_link m_listen;

    /* accept m_epoll */
    epoll_t m_epoll;
    std::thread m_accept;

    /* 接收数据的accept */
    std::vector<epoll_t> m_recv_epoll;

    /* 发送和接收数据的线程总数 */
    uint32_t m_thread_num;

    /* 接收数据的数量 */
    uint32_t m_recv_thread_num;


    std::vector<std::thread> m_recv_data;
    std::vector<std::thread> m_send_data;

    safe_recycle_list<PerIOCtx> m_io_ctx;
    safe_recycle_list<PerSocketCtx> m_socket_ctx;

    safe_map<net_link, PerSocketCtxIter, true> m_link_map;

    /* 待处理数据的socket */
    std::list<PerSocketCtxIter> m_recv_sd;
    std::mutex m_recv_mx;
    std::condition_variable m_recv_cv;

    /* 需要发送的数据 */
    std::list<PerIOCtxIter> m_send_io;
    std::mutex m_send_mx;
    std::condition_variable m_send_cv;
    
};

#endif