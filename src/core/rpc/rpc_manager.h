#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.





#include "rpc_interface.h"
#include <core/tools/gs_time.h>
#include <core/svc_info/svc_info.h>

class CUInt64SeqGenerator;
class CRPCManager: public IRPCManager, public IManager
{
public:
    explicit CRPCManager();
    ~CRPCManager();

    DISALLOW_COPY_AND_ASSIGN(CRPCManager)

public:
    void Init() override;
    void Update() override;
    void Destroy() override;

    void ProcMessage(const net_link& link, const int32_t& msg_id, const char* data, const size_t& len) override;

    void RmiCall(const svc_token_t& to_svc, const mng_t& to_mng, const int32_t& to_func, const std::string& args, 
        std::string& ret) override;
private:
    void TryConnectPeer();
    void UpdateConnectPeer();
    void DisconnectPeer(const svc_token_t& token);

    void ProcRmiReq(const net_link& link, const char* data, const size_t& len);
    void ProcRequestErr(const net_link& link, const svc_token_t& from, const uint64_t& res_id, int err);
    void ProcRmiRep(const char* data, const size_t& len);
    void ProcPing(const net_link& link, const char* data, const size_t& len);
    void StartPing();
    const RPCConnectPeerInfo* GetToPeerLink(const svc_token_t& target, const svc_token_t& exclude);
    const RPCConnectPeerInfo* GetRandLink(const int32_t& svc_type, const svc_token_t& exclude);
    void RmiImpl(mng_t mng_id, svc_token_t req_svc, uint64_t req_id, int32_t func_id, std::string args, bool is_co);
private:
    INetMsgManager* m_net_msg_mng;
    std::unordered_map<svc_token_t, RPCConnectPeerInfo> m_connect_peer;
    msec_t m_last_check_con;// 检查服务器连接心跳时间
    safe_recycle_list<RMIRepData> m_rmi_rep;
    std::vector<RPCConnectPeerInfo*> m_can_relay_svc_tmp;
};


