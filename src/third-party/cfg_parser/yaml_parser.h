#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.


#include <yaml-cpp/yaml.h>

#include "cfg_parser.h"

typedef YAML::Node yaml_node;

class IYamlParser : public ICfgParser
{
public:
    IYamlParser();
    virtual ~IYamlParser();

    /* 实现此接口读取配置 */
    virtual bool ReadContent(const yaml_node& root) = 0;

private:
    /* 当配置有更新，会调用此接口 */
    virtual bool Refresh() override;
};
