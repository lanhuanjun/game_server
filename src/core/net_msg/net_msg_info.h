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

#include "core/net_io/net_io_info.h"
#include "core/stream/binary_stream.h"

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