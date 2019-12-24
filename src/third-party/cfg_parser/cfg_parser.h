#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.



#include <string>
#include <core/tools/gs_time.h>

#define CFG_FULL_PATH(BASE) "../../"#BASE

class ICfgParser
{
public:
    ICfgParser();
    virtual ~ICfgParser();
public:
    /* 首次加载设置配置文件路径 */
    bool Load(const char* path);
    /* 配置文件更新检查 */
    bool Update();
    /* 当配置有更新，会调用此接口 */
    virtual bool Refresh() = 0;
    /* 配置文件从运行到此时的变更次数 */
    const uint32_t& Version();
protected:
    /* 检查文件是否有更新,有更新则重新触发配置读取逻辑*/
    bool Reload();
protected:
    std::string m_cfg_path;
    msec_t m_cfg_last_check;
    sec_t m_cfg_last_modify;
    /* 配置文件从运行到此时的变更次数 */
    uint32_t m_version;
};