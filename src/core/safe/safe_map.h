#pragma once
/*****************************************************************************\
    *  @COPYRIGHT NOTICE
    *  @Copyright (c)2019 - 2030 lanyeo
    *  @file	 : safe_map.h
    *  @version  : ver 1.0
    
    *  @author   : lanyeo
    *  @date     : 2019Äê10ÔÂ27ÈÕ 12:00:00
    *  @brief    : 
\*****************************************************************************/
#include <map>
#include <unordered_map>
#include <mutex>

template<typename K, typename V, bool _IsHashMap>
class safe_map;

template<typename K, typename V>
class safe_map<K, V, false>
{
public:
    using iterator = typename std::map<K, V>::iterator;
    bool empty()
    {
        return m_data.empty();
    }
    size_t size()
    {
        return m_data.size();
    }
    auto insert(const std::pair<K, V>& val)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_data.insert(val);
    }
    
    void erase(const K& key)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto f_iter = m_data.find(key);
        if (f_iter == m_data.end()) {
            return ;
        }
        m_data.erase(f_iter);
    }
    std::pair<iterator, bool> find(const K& key)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto f_iter = m_data.find(key);
        if (f_iter == m_data.end()) {
            return std::make_pair(f_iter, false);
        } else {
            return std::make_pair(f_iter, true);
        }
    }
    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_data.clear();
    }
private:
    std::map<K, V> m_data;
    std::mutex m_mutex;
};

template<typename K, typename V>
class safe_map<K, V, true>
{
public:
    using iterator = typename std::unordered_map<K, V>::iterator;
    bool empty()
    {
        return m_data.empty();
    }
    size_t size()
    {
        return m_data.size();
    }
    auto insert(const std::pair<K, V>& val)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_data.insert(val);
    }

    void erase(const K& key)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto f_iter = m_data.find(key);
        if (f_iter == m_data.end()) {
            return;
        }
        m_data.erase(f_iter);
    }
    std::pair<iterator, bool> find(const K& key)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto f_iter = m_data.find(key);
        if (f_iter == m_data.end()) {
            return std::make_pair(f_iter, false);
        }
        else {
            return std::make_pair(f_iter, true);
        }
    }

    iterator end()
    {
        return m_data.end();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_data.clear();
    }
private:
    std::unordered_map<K, V> m_data;
    std::mutex m_mutex;
};