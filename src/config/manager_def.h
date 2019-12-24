#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

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