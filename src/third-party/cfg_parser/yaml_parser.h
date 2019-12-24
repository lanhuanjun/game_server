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

    /* ʵ�ִ˽ӿڶ�ȡ���� */
    virtual bool ReadContent(const yaml_node& root) = 0;

private:
    /* �������и��£�����ô˽ӿ� */
    virtual bool Refresh() override;
};
