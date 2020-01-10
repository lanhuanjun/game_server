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
    // ���������߳�
    int CreateThread();

    //  ����һ��socket
    net_link CreateOneNetLink();
    // ���������socket
    int CreateListenNetLink();
    // ����һ���ȴ��ͻ������ӵ�socket
    int CreateClientNetLink();
    // ���������¼�
    void DoAccept(PerIOCtxIter io_ctx, DWORD io_size);
    // �������¼�
    void DoSend(PerIOCtxIter io_ctx, DWORD io_size);
    // �����ȡ�¼�
    void DoRead(PerIOCtxIter io_ctx, DWORD io_size);

    /* ����һ����ȡ�¼� */
    bool PostRead(PerSocketCtxIter socket_ctx);

    /* ����һ��д���¼�
     * offset:����ڷ������ݵĻ�ַƫ��
     */
    bool PostWrite(PerIOCtxIter io_ctx, const size_t& offset = 0);

    /* �ر�һ��client���� */
    void CloseLink(net_link link);
private:
    // ��ɶ˿�
    HANDLE m_iocp;

    // �����˿�
    uint16_t m_port;

    uint32_t m_recv_thread_num;
    uint32_t m_work_thread_num;

    // �����߳�
    std::vector<std::thread> m_work_threads;

    // ����socket
    net_link m_listen;

    // �������ӵ�socket
    safe_recycle_list<PerSocketCtx, HeapAllocator<PerSocketCtx>> m_socket_ctx;

    // ���е�IO
    safe_recycle_list<PerIOCtx, HeapAllocator<PerIOCtx>> m_io_ctx;

    // �ȴ�������û�����
    std::mutex m_recv_mx;
    std::condition_variable m_recv_cv;
    std::list<PerSocketCtxIter> m_wait_detail;
    
    // �ȴ����͵��û�����
    std::mutex m_send_mx;
    std::condition_variable m_send_cv;
    std::list<PerIOCtxIter> m_wait_send;

    // �������
    bool m_finish;

    // �ͻ�������
    safe_map<net_link, PerSocketCtxIter, false> m_active_link;

    const static uint32_t TEMP_BUFFER_LEN = (NET_MSG_DEFAULT_BUFFER_SIZE * 10);
    char m_temp_buf[TEMP_BUFFER_LEN];
};

#endif
