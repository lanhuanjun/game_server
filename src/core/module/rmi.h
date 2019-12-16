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