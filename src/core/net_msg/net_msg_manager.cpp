#include "net_msg_manager.h"
#include <core/net_io/lib_net_io.h>
#include <core/tools/gs_assert.h>
#include <core/tools/gs_tlv.h>
#include <core/stream/binary_stream.h>
#include <core/svc_info/svc_info.h>
#include <core/module/rmi.h>

#define NET_MSG_LOG(SEVERITY)\
    LOG(SEVERITY) << "[net_msg][" <<__FUNCTION__<<"] "

struct MsgKey
{
    int32_t type;
    int32_t id;
};

MNG_IMPL(net_msg, INetMsgManager, CNetMsgManager)
CNetMsgManager::CNetMsgManager()
    : m_net_io(nullptr)
    , m_msg_buf(INetIO::MSG_BUF_LEN)
{
}

CNetMsgManager::~CNetMsgManager()
{
    delete m_net_io;
}

void CNetMsgManager::Init()
{
    m_net_io = lib_export();
    m_net_io->SetListenPort(svc_self_net_listen_port());
    m_net_io->SetWorkThreadNum(svc_self_net_thread_num());
    m_net_io->Initiate();
    AlwaysAssert(m_net_io->Run() == NET_OK);
}
void CNetMsgManager::Update()
{
    
}
void CNetMsgManager::Destroy()
{
    m_net_io->CleanUp();
}
void CNetMsgManager::RegMsgProc(int32_t msg_type, mng_t mng_id)
{
    auto p_mng = FindManager(mng_id);
    auto p_msg_proc = dynamic_cast<INetMsgProc*>(p_mng);
    AlwaysAssert(p_msg_proc != nullptr)
    AlwaysAssert(m_msg_proc.insert({msg_type, p_msg_proc}).second);
}
bool CNetMsgManager::ProcMsg(const uint32_t& time_out)
{
    m_msg_buf.reset();
    net_link link = INVALID_NET_LINK;
    if (!m_net_io->RecvData(link, m_msg_buf, time_out)) {
        return true;
    }
    if (m_msg_buf.used < tlv_min_len<MsgKey>()) {
        NET_MSG_LOG(WARNING) << "incomplete msg!";
        return false;
    }

    const int32_t& msg_type = tlv_type<MsgKey>(m_msg_buf.data).type;

    auto msg_proc = m_msg_proc.find(msg_type);
    if (m_msg_proc.end() == msg_proc) {
        NET_MSG_LOG(ERROR) << "unknown msg type! " << msg_type;
        return false;
    }

    msg_proc->second->ProcMessage(link, tlv_type<MsgKey>(m_msg_buf.data).id, tlv_val<MsgKey>(m_msg_buf.data), 
        tlv_len<MsgKey>(m_msg_buf.data));
    return true;
}
bool CNetMsgManager::SendMsg(const net_link& link, const IMsg& msg)
{
    CBinaryStream stream;
    stream << msg;

    tlv data = tlv_mk(MsgKey{ msg.__msg_type__(), msg.__msg_id__() }, stream.data(), stream.size());

    if (NET_OK != m_net_io->SendData(link, data, tlv_total_len<MsgKey>(data))) {
        NET_MSG_LOG(ERROR) << "send msg fail!";
        tlv_free(data);
        return false;
    }
    tlv_free(data);
    return true;
}
net_link CNetMsgManager::AddConnect(const std::string& ip, const uint16_t& port)
{
    return m_net_io->AddConnect(ip, port);
}

