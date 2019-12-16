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
    /* �״μ������������ļ�·�� */
    bool Load(const char* path);
    /* �����ļ����¼�� */
    bool Update();
    /* �������и��£�����ô˽ӿ� */
    virtual bool Refresh() = 0;
    /* �����ļ������е���ʱ�ı������ */
    const uint32_t& Version();
protected:
    /* ����ļ��Ƿ��и���,�и��������´������ö�ȡ�߼�*/
    bool Reload();
protected:
    std::string m_cfg_path;
    msec_t m_cfg_last_check;
    sec_t m_cfg_last_modify;
    /* �����ļ������е���ʱ�ı������ */
    uint32_t m_version;
};