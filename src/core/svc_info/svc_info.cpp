#include "svc_info.h"
#include "svc_cfg.h"
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <core/tools/gs_assert.h>


// 进程的服务类型
DEFINE_int32(SERVICE_TYPE, 0, "server launch type");
// 进程的服务ID
DEFINE_int32(SERVICE_ID, 0, "server id");
// 进程的服务update时间间隔
DEFINE_int32(SERVICE_UPDATE_TICK, 50, "server update tick time");

svc_token_t __svc_self_token__ = SVC_INVALID_TOKEN;

CSvcCfgParser g_svc_cfg;

svc_cfg_info* __svc_self_cfg__ = nullptr;

mng_ptr_map* __svc_mng_ptr_map__ = nullptr;

mng_server_ptr_map* __mng_server_map__ = nullptr;

int __svc_info_init__(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);

    std::stringstream log_path;
    log_path << FLAGS_log_dir << FLAGS_SERVICE_TYPE << "_" << FLAGS_SERVICE_ID << ".";
    const std::string log_file_ext = log_path.str();
    google::SetLogDestination(google::GLOG_INFO, log_file_ext.c_str());
    google::SetLogDestination(google::GLOG_WARNING, log_file_ext.c_str());
    google::SetLogDestination(google::GLOG_ERROR, log_file_ext.c_str());
    google::SetLogDestination(google::GLOG_FATAL, log_file_ext.c_str());
    google::InitGoogleLogging(argv[0]);

    __svc_self_token__ = SVC_TOKEN_MAKE(FLAGS_SERVICE_TYPE, FLAGS_SERVICE_ID);

    LOG(INFO) << "self id:" << FLAGS_SERVICE_TYPE << " type:" << FLAGS_SERVICE_ID << " token:" << __svc_self_token__;
    g_svc_cfg.Load(CFG_FULL_PATH(cfg/svc_global_cfg.yaml));

    __svc_self_cfg__ = g_svc_cfg.FindSvcCfg(__svc_self_token__);

    LOG_IF(FATAL, __svc_self_cfg__ == NULL) << "can't find svc config!";

    LOG(INFO) << "svc init finish!";
    return 0;
}

void __set_svc_mng_ptr_map(mng_ptr_map* p)
{
    __svc_mng_ptr_map__ = p;
}

void __set_mng_server_map(mng_server_ptr_map* p)
{
    __mng_server_map__ = p;
}

const int& svc_self_raw_id()
{
    return FLAGS_SERVICE_ID;
}

const int& svc_self_type()
{
    return FLAGS_SERVICE_TYPE;
}

const int& svc_self_update_tick()
{
    return FLAGS_SERVICE_UPDATE_TICK;
}

const std::string& svc_self_name()
{
    return __svc_self_cfg__->name;
}

const uint16_t& svc_self_net_listen_port()
{
    return __svc_self_cfg__->port;
}

const uint32_t& svc_self_net_thread_num()
{
    return __svc_self_cfg__->net_thread;
}

svc_token_t svc_self_token()
{
    return __svc_self_token__;
}

const std::list<std::string>& svc_self_mng_names()
{
    return __svc_self_cfg__->mngs;
}

IManager* svc_find_manager(const int32_t& mng_id)
{
    return __svc_mng_ptr_map__->find(mng_id);
}

IRmiServer* svc_find_manager_server(const int32_t& mng_id)
{
    return __mng_server_map__->find(mng_id);
}

const svc_cfg_info* find_svc_cfg(const svc_token_t& svc_token)
{
    return g_svc_cfg.FindSvcCfg(svc_token);
}

const svc_cfg_info* svc_self_cfg()
{
    return __svc_self_cfg__;
}

void find_svc_cfg(const int32_t& svc_type, std::map<svc_token_t, svc_cfg_info*>& infos)
{
    return g_svc_cfg.FilterSvcCfgByType(svc_type, infos);
}

int32_t find_relay_svc(const int32_t& target)
{
    return g_svc_cfg.FindRelay(target);
}
