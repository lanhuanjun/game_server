#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.



// 链表节点结构


template <typename T>
struct list_node
{
    typedef T DataType;

    list_node()
        : _Forward(nullptr)
        , _Backward(nullptr)
        , _Data(nullptr)
        , _Origin(this)
    {
        
    }

    list_node(const list_node& _node)
        : _Forward(_node._Forward)
        , _Backward(_node._Backward)
        , _Data(_node._Data)
        , _Origin(_node._Origin)
    {
        
    }
    list_node& operator = (const list_node& _node)
    {
        this->_Data = _node._Data;
        this->_Backward = _node._Backward;
        this->_Forward = _node._Forward;
        this->_Origin = _node._Origin;
        return *this;
    }
    list_node(list_node&& _node) noexcept
        : _Forward(_node._Forward)
        , _Backward(_node._Backward)
        , _Data(_node._Data)
        , _Origin(_node._Origin)
    {

    }
    ~ list_node<T> ()
    {
    }
    T& operator * ()
    {
        return *_Data;
    }
    T* operator -> ()
    {
        return _Data;
    }

    list_node* operator ++ ()
    {
        return _Forward;
    }
    list_node* operator -- ()
    {
        return _Backward;
    }

    bool operator == (const list_node<T>& _cmp)
    {
        return this->_Data == _cmp._Data
        && this->_Forward == _cmp._Forward
        && this->_Backward == _cmp._Backward;
    }

    T* ptr()
    {
        return _Data;
    }

    T* data()
    {
        return _Data;
    }

    list_node<T>* node_ptr()
    {
        return _Origin;
    }

    list_node* _Forward;
    list_node* _Backward;
    T* _Data;
    list_node* _Origin;
};