#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

#include <unordered_map>

namespace gs
{
    template <typename Key, typename PtrType>
    class ptr_map final
    {
    public:
        ptr_map()
        {

        }
        ~ptr_map()
        {
            for (auto iter = m_ptr_map.begin(); iter != m_ptr_map.end(); ++iter) {
                PtrType* ptr = iter->second;
                delete ptr;
                iter->second = nullptr;
            }
            m_ptr_map.clear();
        }
        /*
        * 插入成功返回true, 重复则返回false
        */
        bool insert(const Key& unique_id, PtrType* ptr) {
            return m_ptr_map.insert({ unique_id, ptr }).second;
        }

        /*
         * 根据ID查找，对应指针实例。如果找不到则返回false
         */
        PtrType* find(const Key& id) {
            auto f = m_ptr_map.find(id);
            if (m_ptr_map.end() == f) {
                return nullptr;
            }
            return f->second;
        }

        template <typename Impl>
        Impl* find(const Key& id) {
            auto f = m_ptr_map.find(id);
            if (m_ptr_map.end() == f) {
                return nullptr;
            }
            return dynamic_cast<Impl*>(f->second);
        }

        /*
         * 传入函数对每个指针执行该操作
         */
        template <typename Func>
        void foreach(Func func) {
            for (auto&& ptr : m_ptr_map) {
                func(ptr.second);
            }
        }
    private:
        std::unordered_map<Key, PtrType*> m_ptr_map;
    };
}


