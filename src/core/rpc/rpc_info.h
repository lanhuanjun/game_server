#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.



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
    safe_recycle_list<RMIRepData>::iterator self;
    RMIRepData()
        : is_back(false)
    {
        
    }
};