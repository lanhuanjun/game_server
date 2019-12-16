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



#include "net_msg_interface.h"
#include "core/tools/buffer.h"

typedef std::unordered_map<int32_t, INetMsgProc*> MsgProcMap;

class INetIO;

class CNetMsgManager : public INetMsgManager
{
public:
    explicit CNetMsgManager();
    ~CNetMsgManager();

    DISALLOW_COPY_AND_ASSIGN(CNetMsgManager)

public:

    void Init() override;
    void Update() override;
    void Destroy() override;
    void RegMsgProc(int32_t msg_type, mng_t mng_id) override;
    bool ProcMsg(const uint32_t& time_out) override;
    bool SendMsg(const net_link& link, const IMsg& msg) override;
    net_link AddConnect(const std::string& ip, const uint16_t& port) override;

private:
    MsgProcMap m_msg_proc;
    INetIO* m_net_io;
    fixed_buf m_msg_buf;

};


