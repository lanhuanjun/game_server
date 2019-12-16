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

// 本文件用于申请项目的唯一ID

#include <cstdint>
#include <string>
/* 非法manager ID */
const static int32_t INVALID_MNG_ID = 0;

/* 定义管理模块的ID */
typedef int32_t mng_t;


#define MANAGER_ID_DEF(NAME, ID) ManagerUniqueID_##NAME = ID,

class IManager;
typedef IManager* (*__manager_instance__)();
class IRmiServer;
typedef IRmiServer* (*__manager_server_instance__)();
struct GlobalManagerInfo
{
    mng_t id;
    std::string name;
    __manager_instance__ instance;
    __manager_server_instance__ svc_instance;
};

/* 在此定义工程 */
#define MANAGER\
    MNG(none)\
    MNG(net_msg)\
    MNG(rpc)\
    MNG(role)

#define MNG(NAME) ManagerUniqueID_##NAME,
enum ProjectIDDef { MANAGER };
#undef MNG

/* 根据模块ID找到模块信息 */
const GlobalManagerInfo* find_manager_info_by_id(const mng_t& id);

/* 根据模块名找到模块信息 */
const GlobalManagerInfo* find_manager_info_by_name(const char* name);