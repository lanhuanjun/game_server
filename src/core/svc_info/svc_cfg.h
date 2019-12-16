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