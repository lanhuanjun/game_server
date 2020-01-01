#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.


#include <string>
#include <core/tools/gs_assert.h>
#include <core/tools/ptr_map.h>

class IRmiServer;
using mng_server_ptr_map = gs::ptr_map<mng_t, IRmiServer>;

class IRmiServer
{
public:
    virtual void Init() = 0;
    virtual int RmiExec(const int32_t& func_id, const std::string& args, std::string& res) = 0;
};

template <typename Impl>
class RmiServerImpl : public IRmiServer
{
public:
    virtual int RmiExec(const int32_t& func_id, const std::string& args, std::string& res) override
    {
        AlwaysAssert(0);
    }
    virtual void Init() override
    {
        
    }
};

template <typename Impl>
class Rmi
{
    
};