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
    void TestCall();

private:
    msec_t m_last_call;
};