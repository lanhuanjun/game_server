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
    /* 构成该类型进程的manager */
    std::list<std::string> mngs;
    /* 该服务类型连接其他的服务类型 */
    std::set<int32_t> connect;
    svc_cfg_info() : type(0), id(0), port(0), net_thread(0) { }
};

/* 定义服务器ID */
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

/* 本服务器进程的原始ID */
const int& svc_self_raw_id();
/* 本服务器进程的类型 */
const int& svc_self_type();
/* 本服务器进程的update时间间隔 */
const int& svc_self_update_tick();
/* 本服务器进程的名称 */
const std::string& svc_self_name();
/* 本服务器进程的net模块监听端口 */
const uint16_t& svc_self_net_listen_port();
/* 本服务器进程的net模块线程数量 */
const uint32_t& svc_self_net_thread_num();
/* 本服务器进程的全局的ID */
svc_token_t svc_self_token();
/* 本服务器进程的组件名称 */
const std::list<std::string>& svc_self_mng_names();

/* 本服务器进程配置 */
const svc_cfg_info* svc_self_cfg();

/* 查找组件实例 */
class IManager;
IManager* svc_find_manager(const int32_t& mng_id);

class IRmiServer;
IRmiServer* svc_find_manager_server(const int32_t& mng_id);
/* 查找服务器进程配置 */
const svc_cfg_info* find_svc_cfg(const svc_token_t& svc_token);
/* 根据服务器类型，查找所有该类型服务器配置 */
void find_svc_cfg(const int32_t& svc_type, std::map<svc_token_t, svc_cfg_info*>& infos);

/* 根据服务器类型，获取发送到该类型服务器所需转发的服务器 */
int32_t find_relay_svc(const int32_t& target);
