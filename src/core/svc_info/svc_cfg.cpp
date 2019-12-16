#include "svc_cfg.h"
#include <glog/logging.h>

bool CSvcCfgParser::ReadContent(const yaml_node& root)
{
    yaml_node svc = root["servers"];
    for (auto iter = svc.begin(); iter != svc.end(); ++iter) {
        if (!ReadOneSvc(iter->second)) {
            LOG(FATAL) << "global svc err! svc_name:" << iter->first.as<std::string>();
            return false;
        }
    }
    yaml_node relay = root["relay"];
    ReadSvcRelay(relay);
    return true;
}

svc_cfg_info* CSvcCfgParser::FindSvcCfg(const svc_token_t& token)
{
    auto f_svc = m_svc.find(token);
    if (f_svc == m_svc.end()) {
        return nullptr;
    }
    return &f_svc->second;
}

void CSvcCfgParser::FilterSvcCfgByType(const int32_t& type, std::map<svc_token_t, svc_cfg_info*>& filter_svc)
{
    for (auto&& svc : m_svc) {
        if (SVC_TOKEN_TYPE(svc.first) != type) {
            continue;
        }
        filter_svc.insert({ svc.first, &svc.second });
    }
}

int32_t CSvcCfgParser::FindRelay(const int32_t& relay)
{
    auto f_res = m_relay.find(relay);
    if (m_relay.end() == f_res) {
        return SVC_INVALID;
    }
    return f_res->second;
}

bool CSvcCfgParser::ReadOneSvc(const yaml_node& node)
{
    svc_cfg_info svc_cfg;
    svc_cfg.id = node["id"].as<int32_t>();
    svc_cfg.ip = node["ip"].as<std::string>();
    svc_cfg.port = node["port"].as<uint16_t>();
    svc_cfg.net_thread = node["net_thread"].as<int32_t>();
    svc_cfg.type = node["type"]["type"].as<int32_t>();
    svc_cfg.name = node["type"]["name"].as<std::string>();
    auto mngs = node["type"]["mngs"];
    for (size_t i = 0; i < mngs.size(); ++i) {
        svc_cfg.mngs.emplace_back(mngs[i].as<std::string>());
    }
    auto connect = node["type"]["connect"];
    for (size_t i = 0; i < connect.size(); ++i) {
        svc_cfg.connect.insert(connect[i].as<int32_t>());
    }
    auto res = m_svc.insert({ SVC_TOKEN_MAKE(svc_cfg.type, svc_cfg.id), svc_cfg });
    return res.second;
}

bool CSvcCfgParser::ReadSvcRelay(const yaml_node& node)
{
    for (size_t i = 0; i < node.size(); ++i) {
        m_relay.insert({ node[i][0].as<int32_t>(), node[i][1].as<int32_t>() });
    }
    return true;
}