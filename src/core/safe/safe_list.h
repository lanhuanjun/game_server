#pragma once
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
// 
// 实现一个简单的线程安全的list.

#include "list_node.h"
#include <mutex>

template <typename T>
class safe_list
{
public:
    using iterator = list_node<T>;

    safe_list()
        : m_head(nullptr)
        , m_tail(nullptr)
        , m_len(0)
    {
        
    }
    safe_list(const safe_list<T>& _cp) = delete;
    safe_list<T>& operator = (const safe_list<T>& _cp) = delete;
    safe_list(safe_list<T>&& _cp) = delete;

    ~safe_list<T>()
    {
        clear();
    }

    bool empty()
    {
        return m_len == 0;
    }
    size_t length()
    {
        return m_len;
    }
    void push_front(T* val)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_head == nullptr) {
            m_head = new list_node<T>();
            m_head->_Data = val;
            m_tail = m_head;
        } else {
            auto new_node = new list_node<T>();
            new_node->_Data = val;
            new_node->_Backward = m_head;
            m_head->_Forward = new_node;
            m_head = new_node;
        }
        m_len++;
    }
    /* 确保val为new出来的 */
    void push_back(T* val)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_head == nullptr) {
            m_head = new list_node<T>();
            m_head->_Data = val;
            m_tail = m_head;
        }
        else {
            auto new_node = new list_node<T>();
            new_node->_Data = val;
            new_node->_Forward = m_tail;
            m_tail->_Backward = new_node;
            m_tail = new_node;
        }
        m_len++;
    }
    T* front()
    {
        return m_head->_Data;
    }
    T* back()
    {
        return m_tail->_Data;
    }
    void pop_front()
    {
        if (empty()) {
            return;
        }
        std::lock_guard<std::mutex> lock(m_mutex);
        auto p_del = m_head;
        if (m_head->_Backward == nullptr) {
            m_head = nullptr;
            m_tail = nullptr;
        } else {
            m_head = m_head->_Backward;
            m_head->_Forward = nullptr;
        }
        m_len--;
        delete p_del->_Data;
        delete p_del;
    }
    void pop_back()
    {
        if (empty()) {
            return;
        }
        std::lock_guard<std::mutex> lock(m_mutex);
        auto p_del = m_tail;
        if (m_tail->_Forward == nullptr) {
            m_head = nullptr;
            m_tail = nullptr;
        }
        else {
            m_tail = m_tail->_Forward;
            m_tail->_Backward = nullptr;
        }
        m_len--;
        delete p_del->_Data;
        delete p_del;
    }

    /* 确保该iter为该list生产出来且曾经没有释放过 */
    void erase(iterator iter)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        list_node<T>* p_del = nullptr;
        if (m_len == 1) {
            p_del = m_head;
            m_head = m_tail = nullptr;
        } else {
            auto forward = iter._Forward;
            auto backward = iter._Backward;
            if (forward == nullptr) {
                // 释放的为头节点
                m_head = backward;
                p_del = m_head->_Forward;
                m_head->_Forward = nullptr;
                
            } else if (backward == nullptr) {
                // 释放的为尾节点
                m_tail = forward;
                p_del = m_tail->_Backward;
                m_tail->_Backward = nullptr;
            } else {
                p_del = backward->_Forward;
                forward->_Backward = backward;
                backward->_Forward = forward;
            }
        }
        m_len--;
        delete p_del->_Data;
        delete p_del;
    }
    
    T* emplace_back()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        push_back(T());
        return m_tail->_Data;
    }

    T* emplace_front()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        push_front(T());
        return m_head->_Data;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (empty()) {
            return;
        }
        auto iter = m_head;
        do {
            m_head = m_head->_Backward;
            delete iter->_Data;
            delete iter;
            iter = m_head;
        } while (iter != nullptr);
        m_tail = nullptr;
        m_len = 0;
    }

    iterator begin()
    {
        return *m_head;
    }
    iterator end()
    {
        return list_node<T>();
    }

    /* 该方法只允许使用在遍历list，配合begin和end */
    void lock()
    {
        m_mutex.lock();
    }
    /* 该方法只允许使用在遍历list，配合begin和end */
    void unlock()
    {
        m_mutex.unlock();
    }
private:
    std::mutex m_mutex;
    list_node<T>* m_head;
    list_node<T>* m_tail;
    size_t m_len;
};