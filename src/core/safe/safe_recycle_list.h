#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

#include <list>
#include <mutex>

template <typename T, typename Alloc = std::allocator<T>>
class safe_recycle_list
{
public:
    using _ClsType = std::list<T, Alloc>;
    using iterator = typename std::list<T, Alloc>::iterator;

    safe_recycle_list()
    {
    }
    safe_recycle_list(const safe_recycle_list& _cp) = delete;
    safe_recycle_list& operator = (const safe_recycle_list& _cp) = delete;
    safe_recycle_list(safe_recycle_list&& _cp) = delete;
    ~safe_recycle_list()
    {

    }
    /* ��ȡһ������ʹ�õ��� */
    iterator get_free_item()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_free.empty()) {
            m_data.emplace_front();
            m_used.push_front(m_data.begin());
        } else {
            m_used.push_front(m_free.back());
            m_free.pop_back();
        }
        return (m_used.front());
    }
    /* ����һ��item���Ҫ��֤���iterator���ڸ��� */
    void recycle(iterator iter)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_used.remove(iter);
        m_free.push_back(iter);
    }

    /* �����ܵ���С�ռ� */
    void shrink()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        for (auto && iter : m_free) {
            m_data.erase(iter);
        }
        m_free.clear();
    }

    /* free��use���ܴ�С */
    size_t capacity()
    {
        return m_data.size();
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
private:
    std::mutex m_mutex;
    std::list<iterator> m_free;
    std::list<iterator> m_used;
    std::list<T, Alloc> m_data;
};