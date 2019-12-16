#pragma once
/*****************************************************************************\
    *  @COPYRIGHT NOTICE
    *  @Copyright (c)2019 - 2030 lanyeo
    *  @file	 : net_io_interface.h
    *  @version  : ver 1.0
    
    *  @author   : lanyeo
    *  @date     : 2019年10月23日 12:00:00
    *  @brief    : 
\*****************************************************************************/

#include "net_io_info.h"

class INetIO
{
public:

    const static uint32_t MSG_BUF_LEN = NET_MSG_DEFAULT_BUFFER_SIZE;

    virtual ~INetIO() = default;
    /*
     * 设置服务器监听端口 默认端口为 DEFAULT_LISTEN_PORT, 且值不能小于MIN_TCP_IP_PORT
     */
    virtual int SetListenPort(const uint16_t& port) = 0;


    /**
     * 设置工作线程数量，最大不能超过NET_MAX_WORK_THREAD_NUM，
     * (num<2)表示使用PC逻辑处理器数量,
     * 默认NET_DEFAULT_WORK_THREAD_NUM。
     * 其中接收线程和发送线程比例大概是3：1
     * 0：成功 其他：错误
     */
    virtual int SetWorkThreadNum(const uint32_t& num) = 0;


    /**
     * 初始化套接字等工作
     * 0：成功 其他：失败
     */
    virtual int Initiate() = 0;


    /**
     * 运行网络环境
     * 0：成功 其他：失败
     */
    virtual int Run() = 0;

    /*
     * 连接到的服务器，该方法调用需要在Run之后
     * 返回一个net_link
     */
    virtual net_link AddConnect(const std::string& ip, const uint16_t& port) = 0;

    /**
     * 获取接收数据, 该线程会阻塞线程执行，可以设置超时时间，参考参数time_out
     * time_out 超时时间,单位毫秒, 小于0表示永不超时，一直阻塞进行执行
     * return true有数据，false没有数据
     */
    virtual bool RecvData(net_link& socket, net_io_buf& recv_buf, const int32_t& time_out = -1) = 0;

    /**
     * 发送数据到某个socket
     */
    virtual int SendData(const net_link& socket, const char* data, const size_t& data_size) = 0;
    /**
     * 清理网络环境
     */
    virtual void CleanUp() = 0;
};
