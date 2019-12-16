#pragma once
/*****************************************************************************\
    *  @COPYRIGHT NOTICE
    *  @Copyright (c)2019 - 2030 lanyeo
    *  @file	 : ptr_map.h
    *  @version  : ver 1.0
    
    *  @author   : lanyeo
    *  @date     : 2019��10��22�� 12:00:00
    *  @brief    : ���� Key�� ָ��������һ��ӳ�����
    *              ����Key�����int����
\*****************************************************************************/

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
        * ����ɹ�����true, �ظ��򷵻�false
        */
        bool insert(const Key& unique_id, PtrType* ptr) {
            return m_ptr_map.insert({ unique_id, ptr }).second;
        }

        /*
         * ����ID���ң���Ӧָ��ʵ��������Ҳ����򷵻�false
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
         * ���뺯����ÿ��ָ��ִ�иò���
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


