#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.
#include <core/platform/os_macro.h>

#ifdef OS_WIN

#include "net_io_interface.h"
#include <thread>
#include <vector>
#include <condition_variable>
#include <mutex>

struct PerIOCtx;
struct PerSocketCtx;

#define HEAP_ALLOC(s) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (s))
#define HEAP_FREE(p)   HeapFree(GetProcessHeap(), 0, (p))

template <typename T>
class HeapAllocator
{
public:
    typedef T           value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;

    template <typename U>
    struct rebind
    {
        using other = HeapAllocator<U>;
    };
    constexpr HeapAllocator() noexcept {}

    constexpr HeapAllocator(const HeapAllocator&) noexcept = default;

    template <class _Other>
    constexpr HeapAllocator(const HeapAllocator<_Other>&) noexcept {}

    pointer allocate(size_type n, const void* hint = nullptr) {
        return (pointer)HEAP_ALLOC(sizeof(T) * n);
    }

    void deallocate(pointer p, size_type n) {
        HEAP_FREE(p);
    }

    void destroy(pointer p) {
        HEAP_FREE(p);
    }

    pointer address(reference x) {
        return (pointer)&x;
    }

    const_pointer address(const_reference x) {
        return (const_pointer)&x;
    }

    size_type max_size() const {
        return size_type(UINTMAX_MAX / sizeof(T));
    }
};

class CNetIOWin32 : public INetIO
{
    using PerIOCtxIter = safe_recycle_list<PerIOCtx, HeapAllocator<PerIOCtx>>::iterator;
    using PerSocketCtxIter = safe_recycle_list<PerSocketCtx, HeapAllocator<PerSocketCtx>>::iterator;
public:
    CNetIOWin32();
    virtual ~CNetIOWin32();
public:
    int SetListenPort(const uint16_t& port) override;
    int SetWorkThreadNum(const uint32_t& num) override;
    int Initiate() override;
    int Run() override;
    net_link AddConnect(const std::string& ip, const uint16_t& port) override;
    bool RecvData(net_link& socket, NetMsgBufList& recv_buf, const int32_t& time_out) override;
    int SendData(const net_link& socket, const char* data, const uint32_t& data_size) override;
    void CleanUp() override;
private:
    static void RecvDataThread(CNetIOWin32* pThis);
    static void SendDataThread(CNetIOWin32* pThis);
    // 创建工作线程
    int CreateThread();

    //  创建一个socket
    net_link CreateOneNetLink();
    // 创建服务端socket
    int CreateListenNetLink();
    // 创建一个等待客户端连接的socket
    int CreateClientNetLink();
    // 处理连接事件
    void DoAccept(PerIOCtxIter io_ctx, DWORD io_size);
    // 处理发送事件
    void DoSend(PerIOCtxIter io_ctx, DWORD io_size);
    // 处理读取事件
    void DoRead(PerIOCtxIter io_ctx, DWORD io_size);

    /* 发送一个读取事件 */
    bool PostRead(PerSocketCtxIter socket_ctx);

    /* 发送一个写入事件
     * offset:相对于发送数据的基址偏移
     */
    bool PostWrite(PerIOCtxIter io_ctx, const size_t& offset = 0);

    /* 关闭一个client连接 */
    void CloseLink(net_link link);
private:
    // 完成端口
    HANDLE m_iocp;

    // 监听端口
    uint16_t m_port;

    uint32_t m_recv_thread_num;
    uint32_t m_work_thread_num;

    // 工作线程
    std::vector<std::thread> m_work_threads;

    // 监听socket
    net_link m_listen;

    // 所有连接的socket
    safe_recycle_list<PerSocketCtx, HeapAllocator<PerSocketCtx>> m_socket_ctx;

    // 所有的IO
    safe_recycle_list<PerIOCtx, HeapAllocator<PerIOCtx>> m_io_ctx;

    // 等待处理的用户数据
    std::mutex m_recv_mx;
    std::condition_variable m_recv_cv;
    std::list<PerSocketCtxIter> m_wait_detail;
    
    // 等待发送的用户数据
    std::mutex m_send_mx;
    std::condition_variable m_send_cv;
    std::list<PerIOCtxIter> m_wait_send;

    // 服务结束
    bool m_finish;

    // 客户端连接
    safe_map<net_link, PerSocketCtxIter, false> m_active_link;

    const static uint32_t TEMP_BUFFER_LEN = (NET_MSG_DEFAULT_BUFFER_SIZE * 10);
    char m_temp_buf[TEMP_BUFFER_LEN];
};

#endif
