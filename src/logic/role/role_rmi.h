#pragma once
//*****************************************************************************// Copyright (c) 2019 lanyeo
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

// 本文件由工具自动生成, 请勿修改

#include "role_interface.h"
#include <core/module/rmi.h>
enum
{
    __Role_RMI_FUNC_RmiTest_Add__,
};
template <>
class Rmi<IRoleManager>
{
public:
    int RmiTest_Add(int a, int b)
    {
        CBinaryStream stream;
        stream << a << b;
        std::string retData;
        mp_rpc_mng->RmiCall(m_svc_token, ManagerShareInfo<IRoleManager>::MNG_ID, __Role_RMI_FUNC_RmiTest_Add__, stream.str(), retData);
        int __return_val__ = { 0 };
        CBinaryStream retStream(retData.data(), retData.length());
        retStream >> __return_val__;
        return __return_val__;
    }


    Rmi(const svc_token_t& token)
        : m_svc_token(token)
    {
        mp_rpc_mng = dynamic_cast<IRPCManager*>(svc_find_manager(ManagerShareInfo<IRPCManager>::MNG_ID));
        AlwaysAssert(mp_rpc_mng);
    }
private:
    svc_token_t m_svc_token;
    IRPCManager* mp_rpc_mng;
};
    
template <>
class RmiServerImpl<IRoleManager> : public IRmiServer
{
public:
    RmiServerImpl()
        : mp_mng(nullptr)
    { 
    }
    virtual int RmiExec(const int32_t& func_id, const std::string& args, std::string& res) override
    {
        switch (func_id) {
                case __Role_RMI_FUNC_RmiTest_Add__:{
            RmiTest_Add(args, res);
            break;
        }

        default:
            LOG(ERROR) << "[rmi] unknown func id:" << func_id;
            break;
        }
        return 0;
    }
    void Init() override
    {
        mp_mng = dynamic_cast<IRoleManager*>(svc_find_manager(ManagerShareInfo<IRoleManager>::MNG_ID));
        AlwaysAssert(mp_mng);
    }
private:
    void RmiTest_Add(const std::string& args, std::string& res)
    {
        CBinaryStream unpack_args(args.data(), args.size());
        int a = { 0 };
        int b = { 0 };
        unpack_args >> a >> b;
        CBinaryStream pack_res;
        pack_res << mp_mng->RmiTest_Add(a,b);
        res.append(pack_res.data(), pack_res.size());
    }


private:
    IRoleManager* mp_mng;
};
