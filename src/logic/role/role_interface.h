#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.


#include "role_info.h"

#include <core/module/manager.h>
#include <core/module/player.h>
#include <core/rpc/rpc_interface.h>
#include <third-party/coroutine/gs_co.h>


class IRoleManager : public IManager
{
public:

    /*
     * rmi test
     */
    Annotation(@RMI)
    virtual int RmiTest_Add(int a, int b) = 0;
};
MNG_DEF(role, IRoleManager)


class IRolePlayer : public IPlayer
{
    
};
PLAYER_DEF(role, IRolePlayer, 1)


