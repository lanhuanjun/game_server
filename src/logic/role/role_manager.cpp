#include "role_manager.h"
#include "role_rmi.h"
#include <third-party/coroutine/gs_co.h>
#include <core/tools/gs_random.h>

MNG_IMPL(role, IRoleManager, CRoleManager)

CRoleManager::CRoleManager()
    : m_last_call(0)
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
    if (svc_run_msec() - m_last_call > 1000) {
        m_last_call = svc_run_msec();
        START_TASK(&CRoleManager::TestCall, this);
    }
}
void CRoleManager::TestCall()
{
    Rmi<IRoleManager> svc_lobby(__ANY_LOBBY__);
    int32_t a = gs::rand(1, 1000);
    int32_t b = gs::rand(1, 1000);
    int32_t res = svc_lobby.RmiTest_Add(a, b);
    LOG(INFO) << a << " + " << b << " = " << res;
    if (rmi_last_err() != RMI_CODE_OK) {
        rmi_clear_err();
    }
}
void CRoleManager::Destroy()
{
    
}

int CRoleManager::RmiTest_Add(int a, int b)
{
    LOG(INFO) << "call: a:" << a << " b:" << b;
    return a + b;
}
