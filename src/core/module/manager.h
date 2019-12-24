#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.


#include "module.h"
#include <core/tools/ptr_map.h>
#include <config/manager_def.h>
#include <core/platform/lib_export.h>


class IManager;
using mng_ptr_map = gs::ptr_map<mng_t, IManager>;

template <typename MngImpl>
struct ManagerShareInfo
{
    const static mng_t MNG_ID = ManagerUniqueID_none;
};

class IManager : public IModule
{
public:
    IManager()
        : mp_ptr_map(nullptr)
    {
        
    }
    mng_ptr_map* __ptr_map__() {
        return mp_ptr_map;
    }
    void __set_ptr_map__(mng_ptr_map* ptr_map) {
        mp_ptr_map = ptr_map;
    }

    IManager* FindManager(const mng_t& id)
    {
        return mp_ptr_map->find(id);
    }
private:
    mng_ptr_map* mp_ptr_map;
};


/* 可使用此类将一个Manager类转化为另一个类 */
template <typename MngImpl>
class CMngPtr final
{
public:
    CMngPtr(IManager* p_mng) {
        auto ptr_map = p_mng->__ptr_map__();
        if (ptr_map != nullptr) {
            mp_mng_impl = dynamic_cast<MngImpl*>(ptr_map->find(ManagerShareInfo<MngImpl>::MNG_ID));
        }
    }
    MngImpl* operator -> () {
        return mp_mng_impl;
    }
    MngImpl& operator * () {
        return (*mp_mng_impl);
    }
    MngImpl* ptr() {
        return mp_mng_impl;
    }
private:
    MngImpl* mp_mng_impl;
};


#define MNG_DEF(NAME, INTERFACE)\
template<>\
struct ManagerShareInfo<INTERFACE>\
{\
    inline const static mng_t MNG_ID = ManagerUniqueID_##NAME;\
};\
extern "C" {\
LIB_EXPORT IManager* lib_export_##NAME();\
LIB_EXPORT IRmiServer* lib_export_server_##NAME();\
}\

#define MNG_IMPL(NAME, INTERFACE, MNG_IMPL)\
IManager* lib_export_##NAME()\
{\
    return new MNG_IMPL();\
}\
IRmiServer* lib_export_server_##NAME()\
{\
    return new RmiServerImpl<INTERFACE>();\
}

#define Annotation(TYPES)
