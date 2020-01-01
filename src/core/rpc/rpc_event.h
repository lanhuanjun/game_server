#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.



#include "rpc_info.h"

#include <core/svc_info/svc_info.h>
#include <core/net_msg/net_msg_info.h>


enum RPCEventID
{
    RPCEventID_REQ = 1,
    RPCEventID_REP = 2,
    RPCEventID_PING = 3,
};

// RPC 请求的消息
struct CEventRPCRequest : public CMsgRPC
{
    svc_token_t from; // 来源的服务器
    svc_token_t target; // 目标服务器
    mng_t to_mng; // 请求的服务组件
    int32_t to_func; // 请求的接口
    uint64_t p_node;
    bool is_co; // 是否是协程,只有协程请求才会有返回值
    std::string args; // 参数
    CEventRPCRequest()
        : from(SVC_INVALID_TOKEN)
        , target(SVC_INVALID_TOKEN)
        , to_mng(-1)
        , to_func(-1)
        , p_node(0)
        , is_co(false)

    {

    }
    EVENT_DEFINE(RPCEventID_REQ)
    BINARY_SERIALIZE_DEF(from << target << to_mng << to_func << p_node << is_co << args)
    BINARY_DESERIALIZE_DEF(from >> target >> to_mng >> to_func >> p_node >> is_co >> args)
};
// RPC 响应的消息
struct CEventRPCResponse : public CMsgRPC
{
    svc_token_t from; // 来源的服务器
    svc_token_t target; // 目标服务器
    uint64_t p_node;
    std::string ret; // 返回的数据
    int32_t err;

    CEventRPCResponse()
        : from(SVC_INVALID_TOKEN)
        , target(SVC_INVALID_TOKEN)
        , p_node(0)
        , err(RPC_RET_OK)
    {

    }
    EVENT_DEFINE(RPCEventID_REP)
    BINARY_SERIALIZE_DEF(from << target << p_node << ret << err)
    BINARY_DESERIALIZE_DEF(from >> target >> p_node >> ret >> err)
};

struct CEventRPCPing : public CMsgRPC
{
    svc_token_t request; // 请求连接的服务器
    CEventRPCPing()
        : request(SVC_INVALID_TOKEN)
    {

    }
    EVENT_DEFINE(RPCEventID_PING)
    BINARY_SERIALIZE_DEF(request)
    BINARY_DESERIALIZE_DEF(request)
};