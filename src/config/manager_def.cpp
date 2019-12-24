#include "manager_def.h"
#include <string>

//-----------------manager_interface------------------
#include <core/net_msg/net_msg_interface.h>
#include <core/rpc/rpc_interface.h>
#include <logic/role/role_interface.h>



/* none manager */
IManager* lib_export_none() { return nullptr; }
IRmiServer* lib_export_server_none() { return nullptr; }


#define MNG(NAME) {ManagerUniqueID_##NAME, #NAME, lib_export_##NAME, lib_export_server_##NAME},
GlobalManagerInfo g_mng_info[] = {
    MANAGER
};
#undef MNG


const GlobalManagerInfo* find_manager_info_by_name(const char* name)
{
    for (size_t i = 0; i < sizeof(g_mng_info); ++i) {
        if (0 == strcmp(name, g_mng_info[i].name.c_str())) {
            return &g_mng_info[i];
        }
    }
    return nullptr;
}
