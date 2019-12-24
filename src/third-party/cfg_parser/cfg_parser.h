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