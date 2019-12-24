#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.



#include "net_msg_info.h"
#include "core/module/manager.h"
#include "core/net_io/net_io_interface.h"

class INetMsgProc;
class INetMsgManager : public IManager
{
public:
    /* 注册消息的类型的处理模块 */
    virtual void RegMsgProc(int32_t msg_type, mng_t mng_id) = 0;

    /* 在进程中循环调用该方法，该方法会逐条消息调用RegMsgProc让对应组件处理
     * 处理消息，该方法会阻塞，可设置阻塞时间，单位毫秒。
     * 处理了消息返回true，否则返回false
     */
    virtual bool ProcMsg(const uint32_t& time_out) = 0;

    /* 发送消息 */
    virtual bool SendMsg(const net_link& link, const IMsg& msg) = 0;

    /*
     * 连接到的服务器
     * 返回一个SOCKET
     */
    virtual net_link AddConnect(const std::string& ip, const uint16_t& port) = 0;
};

/*
 * 所有处理消息的组件都必须继承该接口
 */
class INetMsgProc
{
public:
    virtual ~INetMsgProc() = default;
    /*
     * 处理消息接口，各个服务模块实现消息处理函数
     */
    virtual void ProcMessage(const net_link& link, const int32_t& msg_id, const char* data, const size_t& len) = 0;
};

MNG_DEF(net_msg, INetMsgManager)