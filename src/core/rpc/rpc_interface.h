#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.



#include "rpc_info.h"
#include "rpc_helper.h"
#include <core/net_msg/net_msg_interface.h>

class IRPCManager : public INetMsgProc
{
public:
    virtual void RmiCall(const svc_token_t& to_svc, const mng_t& to_mng, const int32_t& to_func, 
        const std::string& args, std::string& ret) = 0;
};
MNG_DEF(rpc, IRPCManager)