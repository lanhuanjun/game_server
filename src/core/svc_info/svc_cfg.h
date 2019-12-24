#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

#include "svc_info.h"
#include <cstdint>
#include <unordered_map>
#include <third-party/cfg_parser/yaml_parser.h>


class CSvcCfgParser : public IYamlParser
{
public:
    bool ReadContent(const yaml_node& root) override;

    svc_cfg_info* FindSvcCfg(const svc_token_t& token);

    void FilterSvcCfgByType(const int32_t& type, std::map<svc_token_t, svc_cfg_info*>& filter_svc);

    int32_t FindRelay(const int32_t& relay);
private:
    bool ReadOneSvc(const yaml_node& node);
    bool ReadSvcRelay(const yaml_node& node);
private:
    std::unordered_map<svc_token_t, svc_cfg_info> m_svc;
    std::unordered_map<int32_t, int32_t> m_relay;
};