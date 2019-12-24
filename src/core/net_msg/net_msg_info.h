#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

#include <core/net_io/net_io_info.h>
#include <core/stream/binary_stream.h>

#define EVENT_DEFINE(ID) \
const static int32_t __MSG_ID = ID;\
int32_t __msg_id__() const override\
{\
    return __MSG_ID;\
}\

enum NetMsgType
{
    NET_MSG_INVALID = 0,
    NET_MSG_RPC     = 1, // 远程调用的消息
    NET_MSG_CLIENT  = 2, // 客户端的消息
};

struct IMsg : public ISerialize
{
    net_link __event_link__;
    IMsg()
        : __event_link__(INVALID_NET_LINK)
    {
        
    }
    virtual int32_t __msg_type__() const = 0;
    virtual int32_t __msg_id__() const = 0;
};

struct CMsgRPC : public IMsg
{
    int32_t __msg_type__() const override
    {
        return NET_MSG_RPC;
    }
};

struct CMsgClient : public IMsg
{
    int32_t __msg_type__() const override
    {
        return NET_MSG_CLIENT;
    }
};