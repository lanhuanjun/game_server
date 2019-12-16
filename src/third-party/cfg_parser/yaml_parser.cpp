#include "yaml_parser.h"
#include <glog/logging.h>
IYamlParser::IYamlParser()
{
    
}
IYamlParser::~IYamlParser()
{
    
}
bool IYamlParser::Refresh()
{
    try {
        yaml_node root = YAML::LoadFile(m_cfg_path);
        return ReadContent(root);
    } catch (const std::exception& e) {
        LOG(ERROR) << "load yaml file fail! err:" << e.what() << " file:" << m_cfg_path;
        return false;
    }
}
