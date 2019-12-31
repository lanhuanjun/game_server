#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.
#include <core/platform/os_macro.h>

#ifdef OS_LINUX

#include "net_io_interface.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <sys/epoll.h>
#include <core/safe/safe_list.h>
#include <core/safe/safe_recycle_list.h>
#include <core/safe/safe_map.h>

class PerIOCtx;
class PerSocketCtx;
class CNetIOLinux : public INetIO
{
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
    bool RecvData(net_link& socket, net_io_buf& recv_buf, const int32_t& time_out = -1) override;

    /**
     * 发送数据到某个socket
     */
    int SendData(const net_link& socket, const char* data, const size_t& data_size) override;
    /**
     * 清理网络环境
     */
    void CleanUp() override;

    static void RecvDataThread(CNetIOLinux* pThis);
    static void SendDataThread(CNetIOLinux* pThis);
private:
    bool CreateEpollHandle();
    bool CreateListenSocket();
    bool SetSocketNonblocking(net_link sd);
    bool AddSocketToEpoll(net_link sd);
    void DoError(PerSocketCtx* pSd);
    void DoRead(PerSocketCtx* pSd);
    void DoWrite(PerIOCtx* pIO);
    void DoAccept();
private:
    uint16_t m_port;
    int m_epoll;
    net_link m_listen;
    std::thread m_recv_data;
    std::thread m_send_data;
    safe_list<PerIOCtx> m_recv_io;
    safe_list<PerIOCtx> m_send_io;
    safe_recycle_list<PerIOCtx> m_io_ctx;
    safe_recycle_list<PerSocketCtx> m_socket_ctx;
    safe_map<net_link, PerSocketCtx*, true> m_link_map;

    std::mutex m_send_mx;
    std::condition_variable m_send_cv;
    std::mutex m_recv_mx;
    std::condition_variable m_recv_cv;
};

#endif