#include "rpc_manager.h"
#include "rpc_event.h"
#include <glog/logging.h>
#include <core/tools/gs_random.h>
#include <third-party/coroutine/gs_co.h>


#define RPC_LOG(SEVERITY)\
    LOG(SEVERITY) << "[rpc][" <<__FUNCTION__<<"] "


/* 服务器连接检查时间 */
const int32_t CONNECT_CHECK_TIME = 1000;
/* 远程调用返回检查的最长时间 */
const int32_t RPC_RETURN_CHECK_TIME = 1000 * 5;


MNG_IMPL(rpc, IRPCManager, CRPCManager)
CRPCManager::CRPCManager()
    : m_net_msg_mng(nullptr)
    , m_last_check_con(0)
{
}

CRPCManager::~CRPCManager()
{
}

void CRPCManager::Init()
{
    CMngPtr<INetMsgManager> p_net_msg_mng(this);
    m_net_msg_mng = p_net_msg_mng.ptr();
    TryConnectPeer();
}
void CRPCManager::Update()
{
    START_TASK(&CRPCManager::UpdateConnectPeer, this);
}
void CRPCManager::Destroy()
{
}

void CRPCManager::ProcMessage(const net_link& link, const int32_t& msg_id, const char* data, const size_t& len)
{
    switch (msg_id) {
    case CEventRPCRequest::__MSG_ID:
        ProcRmiReq(link, data, len);
        break;
    case CEventRPCResponse::__MSG_ID:
        ProcRmiRep(data, len);
        break;
    case CEventRPCPing::__MSG_ID:
        ProcPing(link, data, len);
        break;
    default:
        RPC_LOG(ERROR) << "unknown msg id:" << msg_id;
        break;
    }
}

void CRPCManager::RmiCall(const svc_token_t& to_svc, const mng_t& to_mng, const int32_t& to_func, 
                          const std::string& args, std::string& ret)
{
    CEventRPCRequest request;
    request.target = to_svc;
    request.to_mng = to_mng;
    request.to_func = to_func;
    request.args = args;
    request.from = svc_self_token();
    request.is_co = is_in_co();

    RMIRepData* p_rep = nullptr;
    if (request.is_co) {
        // 用于匹配返回值
        auto iter = m_rmi_rep.get_free_item();
        iter->self = iter;
        p_rep = &(*iter);
        request.p_node = (uint64_t)(p_rep);
    }

    auto send_link = GetToPeerLink(request.target, request.from);
    if (send_link == nullptr) {
        RPC_LOG(ERROR) << "target svc unreachable! svc:" << SVC_STR(request.target);
        __rmi_set_last_err__(RMI_CODE_UNREACHABLE);
        return ;
    }
    if (!m_net_msg_mng->SendMsg(send_link->link, request)) {
        RPC_LOG(ERROR) << "rmi fail. svc:" << SVC_STR(request.target);
        __rmi_set_last_err__(RMI_CODE_UNREACHABLE);
        DisconnectPeer(send_link->token);
        return ;
    }
    if (!request.is_co) {
        // 如果不在协程中，不需要检查返回值
        return ;
    }
    msec_t ret_check = svc_run_msec();
    while (svc_run_msec() - ret_check < RPC_RETURN_CHECK_TIME) {
        co_sleep(1);
        if (p_rep->is_back) {
            ret.assign(p_rep->data);
            p_rep->is_back = false;
            m_rmi_rep.recycle(p_rep->self);
            return;
        }
    }
    __rmi_set_last_err__(RMI_CODE_TIMEOUT);
    LOG(WARNING) << "rmi timeout. mng:"<< to_mng << " func:" << to_func << " svc:" << SVC_STR(request.target);
}

void CRPCManager::TryConnectPeer()
{
    std::map<svc_token_t, svc_cfg_info*> all_peers;
    for (auto && svc_type : svc_self_cfg()->connect) {
        find_svc_cfg(svc_type, all_peers);
        for (auto && peer : all_peers) {
            RPCConnectPeerInfo connect;
            connect.token = peer.first;
            connect.ip = peer.second->ip;
            connect.port = peer.second->port;
            m_connect_peer.insert({ peer.first, connect });
        }
    }
    UpdateConnectPeer();
}

void CRPCManager::UpdateConnectPeer()
{
    if (svc_run_msec() - m_last_check_con < CONNECT_CHECK_TIME) {
        return;
    }

    m_last_check_con = svc_run_msec();

    for (auto && peer : m_connect_peer) {
        if (peer.second.link != INVALID_NET_LINK || peer.second.ip.empty()) {
            continue;
        }
        // 如果没有连接则进行连接
        peer.second.link = m_net_msg_mng->AddConnect(peer.second.ip, peer.second.port);
        if (peer.second.link == INVALID_NET_LINK) {
            RPC_LOG(WARNING) << "connect fail! ip:" << peer.second.ip << " port:" << peer.second.port;
        }
        else {
            RPC_LOG(INFO) << "connect ok! svc_type:" << peer.second.ip << " port:" << peer.second.port
                << " sd:" << peer.second.link;
        }
    }
    StartPing();
}

void CRPCManager::DisconnectPeer(const svc_token_t& token)
{
    auto f_peer = m_connect_peer.find(token);
    if (m_connect_peer.end() == f_peer) {
        return;
    }
    f_peer->second.link = INVALID_NET_LINK;
    RPC_LOG(WARNING) << "broken down peer. ip:" << f_peer->second.ip << " port:" << f_peer->second.port;
}

void CRPCManager::ProcRmiReq(const net_link& link, const char* data, const size_t& len)
{
    CEventRPCRequest request;
    CBinaryStream stream(data, len);
    stream >> request;
    if (request.target != svc_self_token()) {
        // 不是目标服务器直接转发
        auto send_link = GetToPeerLink(request.target, request.from);
        if (send_link == nullptr) {
            RPC_LOG(ERROR) << "not found relay svc. token[type:" << SVC_TOKEN_TYPE(request.target) << " id:"
                << SVC_TOKEN_ID(request.target);
            ProcRequestErr(link, request.from, request.p_node, RPC_RET_NO_FOUND_TARGET);
            return;
        }
        if (SVC_TOKEN_ID(request.target) == __ANY_SVC_) {
            // 不是具体的服务器需要改为具体的
            request.target = send_link->token;
        }
        if (!m_net_msg_mng->SendMsg(send_link->link, request)) {
            RPC_LOG(ERROR) << "send msg fail. token[type:" << SVC_TOKEN_TYPE(request.target) << " id:"
                << SVC_TOKEN_ID(request.target);
            ProcRequestErr(link, request.from, request.p_node, RPC_RET_NO_FOUND_TARGET);
        }
        return;
    }
    
    if (request.is_co) {
        START_TASK(&CRPCManager::RmiImpl, this, request.to_mng, request.from, request.p_node, 
            request.to_func, request.args, true);
    } else {
        RmiImpl(request.to_mng, request.from, request.p_node, request.to_func, request.args, false);
    }
}

void CRPCManager::ProcRequestErr(const net_link& link, const svc_token_t& from, const uint64_t& res_id, int err)
{
    // 出错了往回反
    CEventRPCResponse response;
    response.target = from;
    response.from = svc_self_token();
    response.p_node = res_id;
    response.err = err;
    if (!m_net_msg_mng->SendMsg(link, response)) {
        RPC_LOG(ERROR) << "send msg fail. svc:" << SVC_STR(from);
    }
}

void CRPCManager::ProcRmiRep(const char* data, const size_t& len)
{
    CEventRPCResponse response;
    CBinaryStream stream(data, len);
    stream >> response;
    if (svc_self_token() == response.target) {
        auto p = (RMIRepData*)(response.p_node);
        if (p != nullptr) {
            p->data.assign(response.ret);
            p->is_back = true;
        }
        if (response.err != RPC_RET_OK) {
            __rmi_set_last_err__(response.err);
        }
        return;
    }
    // 不是目标服务器直接转发
    auto send_link = GetToPeerLink(response.target, response.from);
    if (send_link == nullptr) {
        return;
    }
    if (!m_net_msg_mng->SendMsg(send_link->link, response)) {
        RPC_LOG(ERROR) << "send msg fail. token[type:" << SVC_TOKEN_TYPE(response.target) << " id:"
            << SVC_TOKEN_ID(response.target);
        DisconnectPeer(send_link->token);
    }
}


void CRPCManager::ProcPing(const net_link& link, const char* data, const size_t& len)
{
    CEventRPCPing ping;
    CBinaryStream stream(data, len);
    stream >> ping;

    m_connect_peer[ping.request].link = link;
    m_connect_peer[ping.request].token = ping.request;
    // RPC_LOG(INFO) << "ping from. svc:" << SVC_STR(ping.request);
}

void CRPCManager::StartPing()
{
    CEventRPCPing ping;
    ping.request = svc_self_token();
    for (auto&& rpc_link : m_connect_peer) {
        if (rpc_link.second.link == INVALID_NET_LINK) {
            continue;
        }
        if (!m_net_msg_mng->SendMsg(rpc_link.second.link, ping)) {
            RPC_LOG(ERROR) << "ping fail! link:" << rpc_link.second.link;
            rpc_link.second.link = INVALID_NET_LINK;
        }
        // RPC_LOG(INFO) << "ping ok! ip:" << rpc_link.second.ip << " port:" << rpc_link.second.port;
    }
}

const RPCConnectPeerInfo* CRPCManager::GetToPeerLink(const svc_token_t& target, const svc_token_t& exclude)
{
    // 1. 如果是发给自己则返回错误
    if (target == svc_self_token()) {
        RPC_LOG(ERROR) << "target service is self. type:" << SVC_TOKEN_TYPE(target) << " id:" << SVC_TOKEN_ID(target);
        return nullptr;
    }
    
    // 2. 是否是请求属于该服务类型的任意节点
    if (__ANY_SVC_ != SVC_TOKEN_ID(target)) {
        // 3. 查找直连的服务器直接转发
        auto f_target_svc = m_connect_peer.find(target);
        if (m_connect_peer.end() != f_target_svc) {
            if (f_target_svc->second.link != INVALID_NET_LINK) {
                return &f_target_svc->second;
            }
        }
    } else {
        auto rand_link = GetRandLink(SVC_TOKEN_TYPE(target), exclude);
        if (rand_link != nullptr) {
            return rand_link;
        }
    }
    // 4. 往转发服务器上转发
    const int32_t relay_svc_type = find_relay_svc(SVC_TOKEN_TYPE(target));
    if (relay_svc_type == SVC_INVALID) {
        RPC_LOG(ERROR) << "svc unreachable! type:" << SVC_TOKEN_TYPE(target) << " id:" << SVC_TOKEN_ID(target);
        return nullptr;
    }
    return GetRandLink(relay_svc_type, exclude);
}

const RPCConnectPeerInfo* CRPCManager::GetRandLink(const int32_t& svc_type, const svc_token_t& exclude)
{
    m_can_relay_svc_tmp.clear();
    // 往任意转发服务器上转发
    for (auto&& rpc_link : m_connect_peer) {
        if (svc_type == SVC_TOKEN_TYPE(rpc_link.first) && rpc_link.second.link != INVALID_NET_LINK 
            && rpc_link.first != exclude) {
            m_can_relay_svc_tmp.emplace_back(&rpc_link.second);
        }
    }
    if (m_can_relay_svc_tmp.empty()) {
        return nullptr;
    }
    return m_can_relay_svc_tmp[gs::rand(m_can_relay_svc_tmp.size())];
}

void CRPCManager::RmiImpl(mng_t mng_id, svc_token_t req_svc, uint64_t req_id, int32_t func_id, std::string args, 
    bool is_co)
{
    auto p_rmi_com = svc_find_manager_server(mng_id);

    if (p_rmi_com == nullptr) {
        LOG(ERROR) << "can't find mng. id:" << mng_id;
    }

    CEventRPCResponse response;
    response.target = req_svc;
    response.from = svc_self_token();
    response.p_node = req_id;
    p_rmi_com->RmiExec(func_id, args, response.ret);

    if (!is_co) {
        // 不在协程中，不需要返回值
        return;
    }

    auto send_link = GetToPeerLink(req_svc, response.from);
    if (send_link == nullptr) {
        return;
    }
    if (!m_net_msg_mng->SendMsg(send_link->link, response)) {
        RPC_LOG(ERROR) << "send msg fail. svc:" << SVC_STR(req_svc);
        DisconnectPeer(send_link->token);
    }
}
