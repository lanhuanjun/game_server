#include "role_manager.h"
#include "role_rmi.h"
#include <third-party/coroutine/gs_co.h>
#include <core/tools/gs_random.h>

MNG_IMPL(role, IRoleManager, CRoleManager)

CRoleManager::CRoleManager()
    : m_last_call(0)
    , err_count(0)
    , call_count(0)
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
    //if (svc_self_token() == SVC_TOKEN_MAKE(1, 1)) {

    //}
    if (svc_run_msec() - m_last_call > 50) {
        m_last_call = svc_run_msec();
        START_TASK std::bind(&CRoleManager::TestCall, this);
    }
}
void CRoleManager::TestCall()
{
    Rmi<IRoleManager> svc_lobby(__ANY_LOBBY__);
    int32_t a = gs::rand(1, 10000);
    int32_t b = gs::rand(1, 10000);
    LOG(INFO) << "rmi--->call: "<< a << "+" << b << "=" << svc_lobby.RmiTest_Add(a, b);
    svc_lobby.RmiTest_Add(a, b);
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
    LOG(INFO) << "rmi--->exe: a:" << a << " b:" << b;
    return a + b;
}
void CRoleManager::RmiTest_Ref(int a, int b, std::string& ret)
{

}
