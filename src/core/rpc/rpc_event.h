#pragma once
//*****************************************************************************\
// Copyright (c) 2019 lanyeo
// All rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 
// Author   : lanyeo

#include "rpc_info.h"

#include <core/svc_info/svc_info.h>
#include <core/net_msg/net_msg_info.h>


enum RPCEventID
{
    RPCEventID_REQ = 1,
    RPCEventID_REP = 2,
    RPCEventID_PING = 3,
};

// RPC �������Ϣ
struct CEventRPCRequest : public CMsgRPC
{
    svc_token_t from; // ��Դ�ķ�����
    svc_token_t target; // Ŀ�������
    mng_t to_mng; // ����ķ������
    int32_t to_func; // ����Ľӿ�
    uint64_t p_node;
    bool is_co; // �Ƿ���Э��,ֻ��Э������Ż��з���ֵ
    std::string args; // ����
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
// RPC ��Ӧ����Ϣ
struct CEventRPCResponse : public CMsgRPC
{
    svc_token_t from; // ��Դ�ķ�����
    svc_token_t target; // Ŀ�������
    uint64_t p_node;
    std::string ret; // ���ص�����
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
    svc_token_t request; // �������ӵķ�����
    CEventRPCPing()
        : request(SVC_INVALID_TOKEN)
    {

    }
    EVENT_DEFINE(RPCEventID_PING)
    BINARY_SERIALIZE_DEF(request)
    BINARY_DESERIALIZE_DEF(request)
};