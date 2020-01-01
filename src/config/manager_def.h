#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

// ���ļ�����������Ŀ��ΨһID

#include <cstdint>
#include <string>
/* �Ƿ�manager ID */
const static int32_t INVALID_MNG_ID = 0;

/* �������ģ���ID */
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

/* �ڴ˶��幤�� */
#define MANAGER\
    MNG(none)\
    MNG(net_msg)\
    MNG(rpc)\
    MNG(role)

#define MNG(NAME) ManagerUniqueID_##NAME,
enum ProjectIDDef { MANAGER };
#undef MNG

/* ����ģ��ID�ҵ�ģ����Ϣ */
const GlobalManagerInfo* find_manager_info_by_id(const mng_t& id);

/* ����ģ�����ҵ�ģ����Ϣ */
const GlobalManagerInfo* find_manager_info_by_name(const char* name);