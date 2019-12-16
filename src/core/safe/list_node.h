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