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

#include <cstdint>
#include <string>
#include <core/net_io/net_io_info.h>
#include <core/svc_info/svc_info.h>

enum RPC_ERROR_CODE
{
    RPC_RET_OK = 0,
    RPC_RET_NO_FOUND_TARGET = 1,
};

struct RPCConnectPeerInfo
{
    svc_token_t token;
    net_link link;
    std::string ip;
    uint16_t port;

    RPCConnectPeerInfo()
        : token(SVC_INVALID_TOKEN)
        , link(INVALID_NET_LINK)
        , port(0)
    {
        
    }
};


struct RMIRepData
{
    bool is_back;
    std::string data;
    RMIRepData()
        : is_back(false)
    {
        
    }
    SAFE_RECYCLE_LIST_NODE_DECLARE(RMIRepData)
};