#pragma once


#include "role_interface.h"
#include <core/tools/gs_time.h>

class CRoleManager : public IRoleManager
{
public:
    explicit CRoleManager();
    ~CRoleManager();

    DISALLOW_COPY_AND_ASSIGN(CRoleManager)

    void Init() override;
    void Update() override;
    void Destroy() override;
    int RmiTest_Add(int a, int b) override;
    void RmiTest_Ref(int a, int b, std::string& ret) override;

    void TestCall();

private:
    msec_t m_last_call;

    uint32_t err_count;
    uint32_t call_count;
    uint32_t be_call;
    msec_t m_last_tick;
    int call_ser;
};