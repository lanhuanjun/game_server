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
