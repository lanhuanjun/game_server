#pragma once
//*****************************************************************************\
// Copyright (c) 2019 lanyeo
// All rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 
// Author   : lanyeo

#include "role_info.h"

#include <core/module/manager.h>
#include <core/module/player.h>
#include <core/rpc/rpc_interface.h>
#include <third-party/coroutine/gs_co.h>


class IRoleManager : public IManager
{
public:

    /*
     * abc这个中文
     */
    Annotation(@RMI)
    virtual int RmiTest_Add(int a, int b) = 0;

    Annotation(@RMI)
    virtual int RmiTest_Func2(int& a, int b) = 0;

    Annotation(@RMI)
    virtual void RmiTest_Func3(int* a, int b, int & c) = 0;

    Annotation(@RMI)
    virtual void RmiTest_Func4() = 0;
};
MNG_DEF(role, IRoleManager)

class IRolePlayer : public IPlayer
{
    
};
PLAYER_DEF(role, IRolePlayer, 1)


