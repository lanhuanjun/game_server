#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

// ʵ��һ��list cache
// ���е�������Ҫ�Լ�ʵ��ʵ�����Լ����ͷŵĺ���
// example:
// struct TestNode
// {
//   static TestNode* Instance() //�ýӿڱ���ʵ��
//   {
//      return new TestNode();
//   }
//   static void Release(TestNode* p) //�ýӿڱ���ʵ��
//   {
//      delete p;
//   }
// }
// ���û�������������ֱ��ʹ�ú�:
// SAFE_RECYCLE_LIST_NODE_DECLARE(NODE_NAME)
//

#include "list_node.h"
#include <list>
#include <mutex>


#define SAFE_RECYCLE_LIST_NODE_DECLARE(NODE_NAME)\
public:\
static NODE_NAME* Instance()\
{\
   return new NODE_NAME();\
}\
static void Release(NODE_NAME* p)\
{\
   delete p;\
}\

template <typename T>
class safe_recycle_list
{
public:
    using iterator = list_node<T>;
    using EndNode = list_node<T>;

    safe_recycle_list()
        : m_head(nullptr)
        , m_tail(nullptr)
        , m_len(0)
    {
        
    }
    safe_recycle_list(const safe_recycle_list<T>& _cp) = delete;
    safe_recycle_list<T>& operator = (const safe_recycle_list<T>& _cp) = delete;
    safe_recycle_list(safe_recycle_list<T>&& _cp) = delete;
    ~safe_recycle_list<T>()
    {
        clear();
    }
    /* ��ȡһ������ʹ�õ��� */
    iterator get_free_item()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_free.empty()) {
            push_front(T::Instance());
            m_used.push_front(m_head);
        } else {
            m_used.push_front(m_free.back());
            m_free.pop_back();
        }
        return (*m_used.front());
    }
    /* ����һ��item���Ҫ��֤���iterator���ڸ��� */
    void recycle(iterator iter)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_used.remove(iter._Origin);
        m_free.push_back(iter._Origin);
    }

    /* �����ܵ���С�ռ� */
    void shrink()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto & iter : m_free) {
            erase(*iter);
        }
        m_free.clear();
    }

    /* free��use���ܴ�С */
    size_t capacity()
    {
        return m_len;
    }
    /* free�Ĵ�С */
    size_t free_size()
    {
        return m_free.size();
    }
    /* use�Ĵ�С */
    size_t used_size()
    {
        return m_used.size();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_len == 0) {
            return;
        }
        m_free.clear();
        m_used.clear();
        auto iter = m_head;
        do {
            m_head = m_head->_Backward;
            T::Release(iter->_Data);
            delete iter;
            iter = m_head;
        } while (iter != nullptr);
        m_tail = nullptr;
        m_len = 0;
    }

    iterator end()
    {
        return list_node<T>();
    }
private:
    /* ȷ��valΪnew������ */
    void push_front(T* val)
    {
        if (m_head == nullptr) {
            m_head = new list_node<T>();
            m_head->_Data = val;
            m_tail = m_head;
        }
        else {
            auto new_node = new list_node<T>();
            new_node->_Data = val;
            new_node->_Backward = m_head;
            m_head->_Forward = new_node;
            m_head = new_node;
        }
        m_len++;
    }
    void erase(iterator iter)
    {
        if (m_len == 1) {
            T::Release(iter._Data);
            delete iter->_Origin;
            m_head = m_tail = nullptr;
        }
        else {
            auto forward = iter->_Forward;
            auto backward = iter->_Backward;
            if (forward == nullptr) {
                // �ͷŵ�Ϊͷ�ڵ�
                m_head = backward;
                m_head->_Forward = nullptr;
            }
            else if (backward == nullptr) {
                // �ͷŵ�Ϊβ�ڵ�
                m_tail = forward;
                m_tail->_Backward = nullptr;
            }
            else {
                forward->_Backward = backward;
                backward->_Forward = forward;
            }
        }
        T::Release(iter._Data);
        delete iter->_Origin;
        m_len--;
    }
private:
    std::mutex m_mutex;
    std::list<list_node<T>*> m_free;
    std::list<list_node<T>*> m_used;

    list_node<T>* m_head;
    list_node<T>* m_tail;
    size_t m_len;
};