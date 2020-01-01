#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.





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


