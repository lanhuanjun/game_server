#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.
#include <cassert>
#include <malloc.h>
#include <cstring>
#include <mutex>
/* 固定大小的缓冲区 */
template <size_t N>
struct fixed_buf final
{
    const static size_t BUF_LEN = N;
    char* data;
    size_t len; // 数据长度
    size_t offset; // 定义一个偏移量
    fixed_buf()
        : data(nullptr)
        , len(0)
        , offset(0)
    {
        data = (char*)calloc(sizeof(char), BUF_LEN);
    }
    fixed_buf(const fixed_buf& _cp) = delete;
    fixed_buf& operator = (const fixed_buf& _cp) = delete;
    fixed_buf(fixed_buf&& _mv) noexcept
        : data(_mv.data)
        , len(_mv.len)
        , offset(_mv.offset)
    {
        _mv.data = nullptr;
        _mv.len = 0;
        _mv.offset = 0;
    }

    ~ fixed_buf()
    {
        if (data) {
            free(data);
        }
    }

    size_t write(const char* _src, const size_t& _size)
    {
        if (_src == nullptr || _size == 0) {
            return 0;
        }
        len = _size;
        memcpy(data, _src, len);
        return len;
    }

    void reset()
    {
        memset(data, 0, BUF_LEN * sizeof(char));
        len = 0;
        offset = 0;
    }
};

/* 实现滑动的buffer */
class slide_buffer final
{
    /* 数据指针 */
    char* data;

    /* 当前缓冲区大小 */
    size_t buf_len;

    /* 数据大小 */
    size_t data_len;

    /* 数据读取的游标位置 */
    size_t _i_read;

    /* 数据写入的游标位置 */
    size_t _i_write;

    std::mutex mx;
public:
    explicit slide_buffer(const size_t& _init_size = 1024)
        : data(nullptr)
        , buf_len(_init_size)
        , data_len(0)
        , _i_read(0)
        , _i_write(0)
    {
        data = (char*)calloc(_init_size, sizeof(char));
        assert(data != nullptr);
    }

    void write(const char* _src, const size_t& _size)
    {
        if (_src == nullptr || _size == 0) {
            return;
        }
        if (buf_len - data_len < _size) {
            enlarge(_size);
        }
        if (_i_read <= _i_write) {
            if (buf_len - _i_write >= _size) {
                memcpy(data + _i_write, _src, _size);
                _i_write += _size;
            } else {
                memcpy(data + _i_write, _src, buf_len - _i_write);
                memcpy(data, _src + (buf_len - _i_write), _size - (buf_len - _i_write));
                _i_write = _size - (buf_len - _i_write);
            } 
        } else {
            memcpy(data + _i_write, _src, _size);
            _i_write += _size;
        }
        data_len += _size;
    }

    void safe_write(const char* _src, const size_t& _size)
    {
        std::lock_guard<std::mutex> lock(mx);
        write(_src, _size);
    }
    /*
     * 读取一段数据
     * _dst: 需保证不为空
     * _size: _dst大小
     * _mv_cursor: 是否移动读取指针，如果为false则内部不会移动读取指针和改变位读取大小
     */
    size_t read(char* _dst, size_t _size, bool _mv_cursor = true)
    {
        /* 没有数据可以拷贝 */
        if (_i_read == _i_write) {
            return 0;
        }

        if (_i_read < _i_write) {
            if (_i_write - _i_read > _size) {
                memcpy(_dst, data + _i_read, _size);
            } else {
                _size = _i_write - _i_read;
                memcpy(_dst, data + _i_read, _size);
            }
            if (_mv_cursor) {
                _i_read += _size;
            }
        } else {
            if (buf_len - _i_read >= _size) {
                memcpy(_dst, data + _i_read, _size);
                if (_mv_cursor) {
                    _i_read += _size;
                }
            } else {
                memcpy(_dst, data + _i_read, buf_len - _i_read);
                const size_t residue = _size - (buf_len - _i_read);
                if (_i_write >= residue) {
                    memcpy(_dst + (buf_len - _i_read), data, residue);
                    if (_mv_cursor) {
                        _i_read = residue;
                    }
                } else {
                    memcpy(_dst + (buf_len - _i_read), data, _i_write);
                    if (_mv_cursor) {
                        _i_read = _i_write;
                    }
                    _size -= (residue - _i_write);
                }
            }
        }
        if (_mv_cursor) {
            assert(data_len >= _size);
            data_len -= _size;
        }
        if (data_len == 0) {
            reset();
        }
        return _size;
    }
    size_t safe_read(char* _dst, size_t _size, bool _mv_cursor = true)
    {
        std::lock_guard<std::mutex> lock(mx);
        return read(_dst, _size, true);
    }

    void skip(size_t _size)
    {
        /* 没有数据可以拷贝 */
        if (_i_read == _i_write) {
            return ;
        }
        if (_i_read < _i_write) {
            if (_i_write - _i_read < _size) {
                _size = _i_write - _i_read;
            }
            _i_read += _size;
        }
        else {
            if (buf_len - _i_read >= _size) {
                _i_read += _size;
            }
            else {
                const size_t residue = _size - (buf_len - _i_read);
                if (_i_write >= residue) {
                    _i_read = residue;
                }
                else {
                    _i_read = _i_write;
                    _size -= (residue - _i_write);
                }
            }
        }
        assert(data_len >= _size);
        data_len -= _size;
    }

    void safe_skip(size_t _size)
    {
        std::lock_guard<std::mutex> lock(mx);
        skip(_size);
    }

    /* 未读取的数据长度 */
    const size_t& len()
    {
        return data_len;
    }

    /* 重置 */
    void reset()
    {
        data_len = 0;
        _i_read = 0;
        _i_write = 0;
        memset(data, 0, buf_len);
    }

    void safe_reset()
    {
        std::lock_guard<std::mutex> lock(mx);
        reset();
    }

    void lock()
    {
        mx.lock();
    }
    void unlock()
    {
        mx.unlock();
    }
    ~slide_buffer()
    {
        if (data) {
            free(data);
        }
    }
private:
    slide_buffer(const slide_buffer& _cp) = delete;
    slide_buffer& operator = (const slide_buffer& _cp) = delete;
    slide_buffer(const slide_buffer&& _mv) = delete;

    void enlarge(size_t inc)
    {
        if (inc < 1024) {
            inc = 1024;
        }
        char* tmp = (char*)realloc(data, buf_len + inc);
        if (tmp == nullptr) {
            free(data);
            assert(tmp == nullptr);
            return;
        }
        data = tmp;
        size_t new_buf_len = buf_len + inc;
        /* 如果当前读指针在写指针后这需要移动指针 */
        if (_i_write < _i_read) {
            if (_i_write <= inc) {
                memcpy(data + buf_len, data, _i_write);
                _i_write += buf_len;
            } else {
                memcpy(data + buf_len, data, inc);
                memmove(data, data + inc, (_i_write - inc));
                _i_write -= inc;
            }
        }
        buf_len = new_buf_len;
    }
};
