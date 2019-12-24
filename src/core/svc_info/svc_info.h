#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

#include <cstdint>
#include <list>
#include <string>
#include <set>
#include <map>
#include <core/tools/make_word.h>
#include <core/module/manager.h>
#include <core/module/rmi.h>
int __svc_info_init__(int argc, char* argv[]);
void __set_svc_mng_ptr_map(mng_ptr_map* p);
void __set_mng_server_map(mng_server_ptr_map* p);

enum SvcType
{
    SVC_INVALID = 0,
    SVC_LOBBY = 1,
    SVC_DB = 2,
    SVC_TRANSMIT = 3,
};


struct svc_cfg_info
{
    int32_t type;
    int32_t id;
    uint16_t port;
    uint32_t net_thread;
    std::string name;
    std::string ip;
    /* ���ɸ����ͽ��̵�manager */
    std::list<std::string> mngs;
    /* �÷����������������ķ������� */
    std::set<int32_t> connect;
    svc_cfg_info() : type(0), id(0), port(0), net_thread(0) { }
};

/* ���������ID */
typedef int64_t svc_token_t;

#define SVC_TOKEN_MAKE(TYPE, ID) WORD_MAKE(TYPE, ID)
#define SVC_TOKEN_TYPE(TOKEN) WORD_HIGH(TOKEN)
#define SVC_TOKEN_ID(TOKEN) WORD_LOW(TOKEN)
#define SVC_INVALID_TOKEN 0
#define SVC_STR(TOKEN) "[" << WORD_HIGH(TOKEN) << "," << WORD_LOW(TOKEN) <<"]"

#define __ANY_SVC_ 0
#define __ANY_LOBBY__ WORD_MAKE(SVC_LOBBY, __ANY_SVC_)
#define __ANY_DB__ WORD_MAKE(SVC_DB, __ANY_SVC_)
#define __ANY_TRANSMIT__ WORD_MAKE(SVC_TRANSMIT, __ANY_SVC_)

/* �����������̵�ԭʼID */
const int& svc_self_raw_id();
/* �����������̵����� */
const int& svc_self_type();
/* �����������̵�updateʱ���� */
const int& svc_self_update_tick();
/* �����������̵����� */
const std::string& svc_self_name();
/* �����������̵�netģ������˿� */
const uint16_t& svc_self_net_listen_port();
/* �����������̵�netģ���߳����� */
const uint32_t& svc_self_net_thread_num();
/* �����������̵�ȫ�ֵ�ID */
svc_token_t svc_self_token();
/* �����������̵�������� */
const std::list<std::string>& svc_self_mng_names();

/* ���������������� */
const svc_cfg_info* svc_self_cfg();

/* �������ʵ�� */
class IManager;
IManager* svc_find_manager(const int32_t& mng_id);

class IRmiServer;
IRmiServer* svc_find_manager_server(const int32_t& mng_id);
/* ���ҷ������������� */
const svc_cfg_info* find_svc_cfg(const svc_token_t& svc_token);
/* ���ݷ��������ͣ��������и����ͷ��������� */
void find_svc_cfg(const int32_t& svc_type, std::map<svc_token_t, svc_cfg_info*>& infos);

/* ���ݷ��������ͣ���ȡ���͵������ͷ���������ת���ķ����� */
int32_t find_relay_svc(const int32_t& target);
