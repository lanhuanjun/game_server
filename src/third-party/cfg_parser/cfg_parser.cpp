#include "cfg_parser.h"
#include "core/tools/gs_file.h"
#include <glog/logging.h>

const uint32_t CFG_CHECK_TICK = 1000 * 60 * 3;//3∑÷÷”ºÏ≤È

ICfgParser::ICfgParser()
    : m_cfg_last_check(0)
    , m_cfg_last_modify(0)
    , m_version(0)
{
    
}

ICfgParser::~ICfgParser() {}

bool ICfgParser::Load(const char* path)
{
    m_cfg_path = path;
    return Reload();
}
bool ICfgParser::Update()
{
    return Reload();
}

const uint32_t& ICfgParser::Version()
{
    return m_version;
}

bool ICfgParser::Reload()
{
    if (svc_run_msec() - m_cfg_last_check < CFG_CHECK_TICK) {
        return true;
    }
    m_cfg_last_check = svc_run_msec();
    if (file_last_modify(m_cfg_path.c_str()) != m_cfg_last_modify) {
        if (gs_file_last_error() != GS_FILE_NO_ERROR) {
            LOG(ERROR) << "cfg file not found! file:" << m_cfg_path;
            gs_file_clear_last_error();
            return false;
        }
        m_cfg_last_modify = file_last_modify(m_cfg_path.c_str());
        m_version++;
        return Refresh();
    }
    return true;
}
