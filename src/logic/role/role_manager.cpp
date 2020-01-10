#include "role_manager.h"
#include "role_rmi.h"
#include <third-party/coroutine/gs_co.h>
#include <core/tools/gs_random.h>

MNG_IMPL(role, IRoleManager, CRoleManager)

CRoleManager::CRoleManager()
    : m_last_call(0)
    , err_count(0)
    , call_count(0)
    , be_call(0)
    , m_last_tick(0)
    , call_ser(0)
{
    
}
CRoleManager::~CRoleManager()
{
    
}
void CRoleManager::Init()
{
    
}
void CRoleManager::Update()
{
    if (svc_run_msec() - m_last_tick > 1000) {
        m_last_tick = svc_run_msec();
        LOG(INFO) << "cnt:" << call_count << " err:" << err_count << " be_call:" << be_call;
        call_count = 0;
        err_count = 0;
        be_call = 0;
        
    }
    
    if (svc_self_token() != SVC_TOKEN_MAKE(1, 1) && svc_run_msec() - m_last_call > 1000) {
        m_last_call = svc_run_msec();
        START_TASK(&CRoleManager::TestCall, this);
    }
    
    
}
void CRoleManager::TestCall()
{
    Rmi<IRoleManager> svc_lobby(__ANY_LOBBY__);
    int32_t a = 1;
    int32_t b = 2;
    const msec_t begin = svc_run_msec();
    svc_lobby.RmiTest_Add(a, b);
    // LOG(INFO) << "call time:" << svc_run_msec() - begin;
    call_count++;
    if (rmi_last_err() != RMI_CODE_OK) {
        err_count++;
        rmi_clear_err();
    }
}
void CRoleManager::Destroy()
{
    
}

int CRoleManager::RmiTest_Add(int a, int b)
{
    // LOG(INFO) << "rmi--->exe: a:" << a << " b:" << b;
    be_call++;
    return a + b;
}
void CRoleManager::RmiTest_Ref(int a, int b, std::string& ret)
{

}
