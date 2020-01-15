#include "svc.h"
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <core/tools/gs_time.h>
#include <core/svc_info/svc_info.h>
#include <core/net_msg/net_msg_interface.h>
#include <core/rpc/rpc_interface.h>
#include <core/platform/core_dump.h>


/*core manager*/
std::string g_core_mng[] = {
    "net_msg",
    "rpc"
};

/* 所有模块的指针 */
std::list<IManager*> g_all_mng_ptr;
mng_ptr_map g_mng_instance_map;

/* 所有模块的服务指针 */
std::list<IRmiServer*> g_all_mng_server_ptr;
mng_server_ptr_map g_mng_server_map;

/* 服务器开关 */
bool g_is_need_stop = false;

/* 全局初始化 */
int global_init(int argc, char* argv[])
{
    __svc_info_init__(argc, argv);
    return 0;
}

/* 全局释放 */
int global_release()
{
    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();
    return 0;
}

void destroy_manager()
{
    for (auto iter = g_all_mng_ptr.rbegin(); iter != g_all_mng_ptr.rend(); ++iter) {
        (*iter)->Destroy();
    }
}


/* 加载各个模块 */
void load_manager()
{
    for (auto& core_mng : g_core_mng) {
        auto mng_info = find_manager_info_by_name(core_mng.c_str());
        AlwaysAssert(mng_info != nullptr);
        auto p_mng = mng_info->instance();
        AlwaysAssert(p_mng != nullptr);
        g_all_mng_ptr.emplace_back(p_mng);
        g_mng_instance_map.insert(mng_info->id, p_mng);

        auto p_mng_server = mng_info->svc_instance();
        AlwaysAssert(p_mng_server != nullptr);
        g_all_mng_server_ptr.emplace_back(p_mng_server);
        g_mng_server_map.insert(mng_info->id, p_mng_server);
    }
    for (auto& cfg_mng : svc_self_mng_names()) {
        auto mng_info = find_manager_info_by_name(cfg_mng.c_str());
        AlwaysAssert(mng_info != nullptr);
        auto p_mng = mng_info->instance();
        AlwaysAssert(p_mng != nullptr);
        g_all_mng_ptr.emplace_back(p_mng);
        g_mng_instance_map.insert(mng_info->id, p_mng);

        auto p_mng_server = mng_info->svc_instance();
        AlwaysAssert(p_mng_server != nullptr);
        g_all_mng_server_ptr.emplace_back(p_mng_server);
        g_mng_server_map.insert(mng_info->id, p_mng_server);
    }
    __set_mng_server_map(&g_mng_server_map);
    __set_svc_mng_ptr_map(&g_mng_instance_map);
}

int __svc_run__(int argc, char* argv[])
{
#ifdef OS_WIN
    init_mini_dump();
#endif

    global_init(argc, argv);

    load_manager();

    LOG(INFO) << "start svc begin -------------->" << svc_self_name();

    /* 初始化各个组件 */
    for (auto && server : g_all_mng_server_ptr) {
        server->Init();
    }

    for (auto&& p_mng : g_all_mng_ptr) {
        p_mng->__set_ptr_map__(&g_mng_instance_map);
        p_mng->Init();
    }

    /* 调用消息处理模块 */
    auto p_net_msg_mng = dynamic_cast<INetMsgManager*>(svc_find_manager(ManagerShareInfo<INetMsgManager>::MNG_ID));

    LOG_IF(ERROR, p_net_msg_mng == nullptr) << " net messgae is null!";

    p_net_msg_mng->RegMsgProc(NET_MSG_RPC, ManagerShareInfo<IRPCManager>::MNG_ID);

    if (p_net_msg_mng != nullptr) {
        msec_t last_update = 0;
        while (!g_is_need_stop) {
            if (svc_run_msec() - last_update > svc_self_update_tick()) {
                for (auto&& p_mng : g_all_mng_ptr) {
                    p_mng->Update();
                }
            }
            try {
                p_net_msg_mng->ProcMsg(svc_self_update_tick());
            } catch (const std::exception& ex) {
                LOG(FATAL) << "err:" << ex.what();
            }
        }
    } else {
        LOG(ERROR) << "not fund net_msg manager. please check your cfg.";
    }
    
    destroy_manager();
    return global_release();
}

void __svc_stop__()
{
    g_is_need_stop = true;
}