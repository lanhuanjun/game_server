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