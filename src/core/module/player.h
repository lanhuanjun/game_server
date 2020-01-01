#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.



#include <core/tools/ptr_map.h>
#include "module.h"



typedef uint32_t player_t;

class IPlayer;
using player_ptr_map = gs::ptr_map<player_t, IPlayer>;

class IPlayer : public IModule
{
public:
    IPlayer()
        : mp_ptr_map(nullptr)
    {

    }
    player_ptr_map* __ptr_map__() {
        return mp_ptr_map;
    }
    void __set_ptr_map__(player_ptr_map* ptr_map) {
        mp_ptr_map = ptr_map;
    }

    IPlayer* FindPlayer(const player_t& id)
    {
        return mp_ptr_map->find(id);
    }
private:
    player_ptr_map* mp_ptr_map;
};

template <typename MngImpl>
struct PlayerShareInfo
{
    const static player_t PLAYER_ID = 0;
};



/* 可使用此类将一个Player类转化为另一个类 */
template <typename PlayerImpl>
class CPlayerPtr final
{
public:
    CPlayerPtr(IPlayer* p_player) {
        auto ptr_map = p_player->__ptr_map__();
        if (ptr_map != nullptr) {
            mp_player_impl = dynamic_cast<PlayerImpl*>(ptr_map->find(PlayerShareInfo<PlayerImpl>::PLAYER_ID));
        }
    }
    PlayerImpl* operator -> () {
        return mp_player_impl;
    }
    PlayerImpl& operator * () {
        return (*mp_player_impl);
    }
    PlayerImpl* ptr() {
        return mp_player_impl;
    }
private:
    PlayerImpl* mp_player_impl;
};


#define PLAYER_DEF(NAME, INTERFACE, OFFSET)\
template<>\
struct ManagerShareInfo<INTERFACE>\
{\
    const static mng_t MNG_ID = ManagerUniqueID_##NAME + OFFSET;\
};
