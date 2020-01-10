#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

// 实现一个简单的线程安全的list.

#include <mutex>
#include <list>

template <typename T>
class safe_list
{
public:
    using iterator = typename std::list<T>::iterator;
    safe_list()
    {
        
    }
    safe_list(const safe_list<T>& _cp) = delete;
    safe_list<T>& operator = (const safe_list<T>& _cp) = delete;
    safe_list(safe_list<T>&& _cp) = delete;

    ~safe_list<T>()
    {
    }

    bool empty()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_data.empty();
    }
    size_t size()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_data.size();
    }
    void push_front(const T&& val)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_data.push_front(val);
    }
    void push_front(T&& val)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_data.push_front(val);
    }
    void push_front(const T& val)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_data.push_front(val);
    }
    void push_front(T& val)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_data.push_front(val);
    }
    void push_back(const T&& val)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_data.push_back(val);
    }
    void push_back(T&& val)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_data.push_back(val);
    }
    void push_back(const T& val)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_data.push_back(val);
    }
    void push_back(T& val)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_data.push_back(val);
    }
    T& front()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_data.front();
    }
    T& back()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_data.back();
    }
    void pop_front()
    {
        if (empty()) {
            return;
        }
        std::unique_lock<std::mutex> lock(m_mutex);
        m_data.pop_front();
    }
    void pop_back()
    {
        if (empty()) {
            return;
        }
        std::unique_lock<std::mutex> lock(m_mutex);
        m_data.pop_back();
    }

    /* 确保该iter为该list生产出来且曾经没有释放过 */
    void erase(const iterator&& iter)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_data.erase(iter);
    }

    template <typename Fn>
    void foreach(Fn fn)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        for (auto iter = m_data.begin(); iter != m_data.end(); ++iter) {
            fn((*iter));
        }
    }

    /* 当fn返回false时, 中断循环 */
    template <typename Fn>
    void foreach_break(Fn fn)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        for (auto iter = m_data.begin(); iter != m_data.end(); ++iter) {
            if (!fn((*iter))) {
                break;
            }
        }
    }
private:
    std::mutex m_mutex;
    std::list<T> m_data;
};