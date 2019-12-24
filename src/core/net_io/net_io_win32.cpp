#include "net_io_win32.h"

#ifdef OS_WIN

#include <core/tools/gs_assert.h>

#define HEAP_ALLOC(s) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (s))
#define HEAP_FREE(p)   HeapFree(GetProcessHeap(), 0, (p))

#define NET_IO_LOG(SEVERITY)\
    LOG_IF(SEVERITY, false) << "[net_io][" <<__FUNCTION__<<"] "

const static uint32_t MIN_POST_READ_IO_SIZE = 5;
struct PerSocketCtx
{
    net_link link;
    LPFN_ACCEPTEX lpfn_acceptex;
    /* 投递读取的IO数量 */
    uint32_t n_post_read;
    PerSocketCtx()
        : link(INVALID_NET_LINK)
        , lpfn_acceptex(nullptr)
        , n_post_read(0)
    {

    }
    void reset()
    {
        link = INVALID_NET_LINK;
        lpfn_acceptex = nullptr;
    }
    void dec_read_io()
    {
        if (n_post_read > 0) {
            n_post_read--;
        }
    }
    void inc_read_io()
    {
        n_post_read++;
    }
    bool need_post_read()
    {
        return MIN_POST_READ_IO_SIZE > n_post_read;
    }
    static PerSocketCtx* Instance()
    {
        return (PerSocketCtx*)HEAP_ALLOC(sizeof(PerSocketCtx));
    }
    static void Release(PerSocketCtx* p)
    {
        HEAP_FREE(p);
    }
};

const static uint32_t PER_IO_CONTEXT_BUF_LEN = NET_MSG_DEFAULT_BUFFER_SIZE;

struct PerIOCtx : WSAOVERLAPPED
{
    NetIOOperation operation;
    safe_recycle_list<PerSocketCtx>::iterator socket_ctx;
    safe_recycle_list<PerIOCtx>::iterator self;
    /* 使用时自己初始化里面的数据 */
    WSABUF wsabuf;
    /* buf使用的数据量大小 */
    size_t buf_used;
    /* 相对于buf开始偏移大小 */
    size_t buf_offset;
    /* 缓冲数据 */
    char buf[PER_IO_CONTEXT_BUF_LEN];
    WSABUF& from_buf(const size_t& offset = 0)
    {
        wsabuf.buf = buf + offset;
        wsabuf.len = PER_IO_CONTEXT_BUF_LEN - offset;
        return wsabuf;
    }

    PerIOCtx()
        : operation(NET_IO_OPERATION_NONE)
        , buf_used(0)
        , buf_offset(0)
    {
        ZeroMemory(buf, sizeof(char) * PER_IO_CONTEXT_BUF_LEN);
    }
    void reset()
    {
        operation = NET_IO_OPERATION_NONE;
        ZeroMemory(buf, sizeof(char) * PER_IO_CONTEXT_BUF_LEN);
        buf_used = 0;
        buf_offset = 0;
        socket_ctx = safe_recycle_list<PerSocketCtx>::EndNode();
    }
    static PerIOCtx* Instance()
    {
        return (PerIOCtx*)HEAP_ALLOC(sizeof(PerIOCtx));
    }
    static void Release(PerIOCtx* p)
    {
        HEAP_FREE(p);
    }
};

CNetIOWin32::CNetIOWin32()
    : m_iocp(INVALID_HANDLE_VALUE)
    , m_port(DEFAULT_LISTEN_PORT)
    , m_work_thread_num(NET_DEFAULT_WORK_THREAD_NUM)
    , m_recv_work_thread_num(0)
    , m_listen(INVALID_SOCKET)
    , m_finish(false)
    , m_read_event(WSA_INVALID_EVENT)
    , m_write_event(WSA_INVALID_EVENT)
{

}

CNetIOWin32::~CNetIOWin32()
{

}

int CNetIOWin32::SetListenPort(const uint16_t& port)
{
    AlwaysAssert(port > 1500);

    m_port = port;
    return NET_OK;
}

int CNetIOWin32::SetWorkThreadNum(const uint32_t& num)
{
    AlwaysAssert(num <= NET_MAX_WORK_THREAD_NUM);

    m_work_thread_num = num;
    if (num < NET_DEFAULT_WORK_THREAD_NUM) {
        SYSTEM_INFO sys_info;
        GetSystemInfo(&sys_info);
        m_work_thread_num = sys_info.dwNumberOfProcessors;
    }
    m_recv_work_thread_num = m_work_thread_num - (m_work_thread_num / RECV_WORK_THREAD_RATIO);
    if (m_recv_work_thread_num == m_work_thread_num) {
        /* 至少要分配一个发送线程 */
        m_recv_work_thread_num = m_work_thread_num - 1;
    }
    return NET_OK;
}

int CNetIOWin32::Initiate()
{
    // 初始化Socket环境
    WSADATA wsa_data;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (ret != 0) {
        return NET_INIT_FAIL;
    }

    for (size_t i = 0; i < NET_MAX_WORK_THREAD_NUM; ++i) {
        m_work_threads[i] = INVALID_HANDLE_VALUE;
    }

    // 初始化临界区
    InitializeCriticalSection(&m_critical_section);

    // 初始化事件
    m_read_event = WSACreateEvent();

    m_write_event = WSACreateEvent();

    NET_IO_LOG(INFO) << "init env finish! work_thread_num:" << m_work_thread_num
        << " recv_thread:" << m_recv_work_thread_num << " port:" << m_port;
    return NET_OK;
}
int CNetIOWin32::CreateThread() {
    // 创建接收线程
    HANDLE thread_handle = INVALID_HANDLE_VALUE;
    DWORD thread_id = 0;
    for (uint32_t thread_index = 0; thread_index < m_work_thread_num; ++thread_index) {
        if (thread_index < m_recv_work_thread_num) {
            // 创建接收线程
            thread_handle = ::CreateThread(nullptr, 0, RecvDataThread, this, 0, &thread_id);
        }
        else {
            // 创建发送线程
            thread_handle = ::CreateThread(nullptr, 0, SendDataThread, this, 0, &thread_id);
        }

        if (thread_handle == INVALID_HANDLE_VALUE) {
            NET_IO_LOG(ERROR) << "create work thread fail! err:" << WSAGetLastError();
            return NET_FAIL;
        }
        NET_IO_LOG(INFO) << "create thread success! thread_id:" << thread_id;
        m_work_threads[thread_index] = thread_handle;
    }
    return NET_OK;
}

net_link CNetIOWin32::CreateOneNetLink()
{
    net_link link = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (link == INVALID_NET_LINK) {
        NET_IO_LOG(ERROR) << "create socket fail! err:" << WSAGetLastError();
        return link;
    }
    int zero = 0;
    int ret = setsockopt(link, SOL_SOCKET, SO_SNDBUF, (char*)&zero, sizeof(zero));
    if (ret == SOCKET_ERROR) {
        NET_IO_LOG(ERROR) << "socket setsockopt error! err:" << WSAGetLastError();
        closesocket(link);
        return INVALID_NET_LINK;
    }
    return link;
}

int CNetIOWin32::CreateListenNetLink()
{
    //创建一个socket
    struct addrinfo hints = { 0 };

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_IP;

    char listen_port_str[7];

    if (sprintf_s(listen_port_str, 7, "%hu", m_port) <= 0) {
        NET_IO_LOG(ERROR) << "port convert to string error ! " << m_port;
        return NET_LISTEN_PORT_FAIL;
    }
    struct addrinfo* p_addr_local = nullptr;
    if (0 != getaddrinfo(nullptr, listen_port_str, &hints, &p_addr_local)) {
        NET_IO_LOG(ERROR) << "getaddrinfo fail! err:" << WSAGetLastError();
        return NET_FAIL;
    }

    if (p_addr_local == nullptr) {
        NET_IO_LOG(ERROR) << "getaddrinfo() failed to resolve/convert the interface! err:" << WSAGetLastError();
        return NET_FAIL;
    }


    m_listen = CreateOneNetLink();
    if (m_listen == INVALID_NET_LINK) {
        freeaddrinfo(p_addr_local);
        return NET_CREATE_SOCKET_FAIL;
    }

    auto socket_ctx = m_socket_ctx.get_free_item();
    // 将listen socket绑定到iocp
    m_iocp = CreateIoCompletionPort((HANDLE)m_listen, m_iocp, (ULONG_PTR)socket_ctx.ptr(), m_recv_work_thread_num);
    if (m_iocp == INVALID_HANDLE_VALUE) {
        socket_ctx->reset();
        m_socket_ctx.recycle(socket_ctx);
        return NET_UPDATE_IOCP_FAIL;
    }

    int ret = bind(m_listen, p_addr_local->ai_addr, int(p_addr_local->ai_addrlen));
    if (SOCKET_ERROR == ret) {
        NET_IO_LOG(ERROR) << "bind! err:" << WSAGetLastError();
        freeaddrinfo(p_addr_local);
        return NET_LISTEN_PORT_FAIL;
    }

    // 设置最大连接数
    ret = listen(m_listen, SOMAXCONN);
    if (SOCKET_ERROR == ret) {
        NET_IO_LOG(ERROR) << "listen! err:" << WSAGetLastError();
        freeaddrinfo(p_addr_local);
        return NET_LISTEN_PORT_FAIL;
    }
    freeaddrinfo(p_addr_local);

    return NET_OK;
}

int CNetIOWin32::CreateClientNetLink()
{
    auto socket_ctx = m_socket_ctx.get_free_item();

    // 创建socket
    socket_ctx->link = CreateOneNetLink();

    if (socket_ctx->link == INVALID_NET_LINK) {
        NET_IO_LOG(ERROR) << "create one link fail! ";
        m_socket_ctx.recycle(socket_ctx);
        return NET_FAIL;
    }

    m_iocp = CreateIoCompletionPort((HANDLE)socket_ctx->link, m_iocp, (ULONG_PTR)(socket_ctx.ptr()), m_recv_work_thread_num);
    if (m_iocp == INVALID_HANDLE_VALUE) {
        socket_ctx->reset();
        m_socket_ctx.recycle(socket_ctx);
        return NET_UPDATE_IOCP_FAIL;
    }

    // 获取acceptex指针
    GUID acceptex_func_guid = WSAID_ACCEPTEX;
    DWORD bytes = 0;
    int ret = WSAIoctl(m_listen,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &acceptex_func_guid,
        sizeof(acceptex_func_guid),
        &socket_ctx->lpfn_acceptex,
        sizeof(socket_ctx->lpfn_acceptex),
        &bytes,
        nullptr,
        nullptr
    );
    // 获取accept func失败
    if (ret == SOCKET_ERROR) {
        NET_IO_LOG(ERROR) << "WSAIoctl fail! err:" << WSAGetLastError();
        socket_ctx->reset();
        m_socket_ctx.recycle(socket_ctx);
        return NET_FAIL;
    }

    auto io_ctx = m_io_ctx.get_free_item();
    io_ctx->socket_ctx = socket_ctx;
    io_ctx->self = io_ctx;
    io_ctx->operation = NET_IO_OPERATION_ACCEPT;

    // 投递一个监听事件
    DWORD recv_bytes_num = 0;
    ret = socket_ctx->lpfn_acceptex(m_listen,
        socket_ctx->link,
        io_ctx->buf,
        NET_MSG_DEFAULT_BUFFER_SIZE - (2 * (sizeof(SOCKADDR_STORAGE) + 16)),
        sizeof(SOCKADDR_STORAGE) + 16, sizeof(SOCKADDR_STORAGE) + 16,
        &recv_bytes_num,
        io_ctx.ptr()
    );
    if (ret == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())) {
        NET_IO_LOG(ERROR) << "AcceptEx fail! err:" << WSAGetLastError();
        io_ctx->reset();
        m_io_ctx.recycle(io_ctx);
        socket_ctx->reset();
        m_socket_ctx.recycle(socket_ctx);
        return NET_FAIL;
    }

    NET_IO_LOG(INFO) << "create client new link success. link:" << socket_ctx->link;
    return NET_OK;
}

void CNetIOWin32::DoAccept(PerIOCtx* p_io_ctx, DWORD io_size)
{
    auto socket_ctx = p_io_ctx->socket_ctx;
    NET_IO_LOG(INFO) << "link:" << socket_ctx->link;
    int ret = setsockopt(socket_ctx->link,
        SOL_SOCKET,
        SO_UPDATE_ACCEPT_CONTEXT,
        (char*)&m_listen,
        sizeof(m_listen)
    );
    if (ret == SOCKET_ERROR) {
        NET_IO_LOG(ERROR) << "setsockopt fail. err:" << WSAGetLastError();
        CloseLink(socket_ctx->link);
        return;
    }

    if (io_size > 0) {
        p_io_ctx->buf_used = io_size;
        m_wait_detail.push_front(new list_node<PerIOCtx>(p_io_ctx->self));
        SetEvent(m_read_event);
    }

    m_active_link.insert({ socket_ctx->link, socket_ctx });

    if (!PostRead(socket_ctx)) {
        CloseLink(socket_ctx->link);
    }
    // 创建一个等待监听的socket
    CreateClientNetLink();
}

void CNetIOWin32::DoSend(PerIOCtx* p_io_ctx, DWORD io_size)
{
    p_io_ctx->buf_offset += io_size;

    if (p_io_ctx->buf_offset >= p_io_ctx->buf_used) {
        NET_IO_LOG(INFO) << "link:" << p_io_ctx->socket_ctx->link;

        if (!PostRead(p_io_ctx->socket_ctx)) {
            CloseLink(p_io_ctx->socket_ctx->link);
        }
        m_io_ctx.recycle(p_io_ctx->self);
    }
    else {
        if (!PostWrite(p_io_ctx->self, p_io_ctx->buf_offset)) {
            NET_IO_LOG(ERROR) << "send data fail! link:" << p_io_ctx->socket_ctx->link << " err:" << WSAGetLastError();
            m_io_ctx.recycle(p_io_ctx->self);
            CloseLink(p_io_ctx->socket_ctx->link);
        }
    }
}

void CNetIOWin32::DoRead(PerIOCtx* p_io_ctx, DWORD io_size)
{
    NET_IO_LOG(INFO) << "link:" << p_io_ctx->socket_ctx->link;
    p_io_ctx->buf_used = io_size;
    m_wait_detail.push_front(new list_node<PerIOCtx>(p_io_ctx->self));
    socket_ctx_iter socket_ctx = p_io_ctx->socket_ctx;
    SetEvent(m_read_event);
}

bool CNetIOWin32::PostRead(socket_ctx_iter& socket_ctx)
{
    NET_IO_LOG(INFO) << "link:" << socket_ctx->link;
    DWORD flag = 0;
    while (socket_ctx->need_post_read()) {
        auto io_ctx = m_io_ctx.get_free_item();
        io_ctx->socket_ctx = socket_ctx;
        io_ctx->operation = NET_IO_OPERATION_READ;
        io_ctx->self = io_ctx;
        auto& wsabuf = io_ctx->from_buf();
        if (WSARecv(socket_ctx->link, &wsabuf, 1, nullptr, &flag, io_ctx.ptr(), nullptr) == SOCKET_ERROR) {
            if (WSAGetLastError() != WSA_IO_PENDING) {
                m_io_ctx.recycle(io_ctx);
                return false;
            }
        }
        socket_ctx->inc_read_io();
    }
    return true;
}

bool CNetIOWin32::PostWrite(io_ctx_iter& io_ctx, const size_t& offset /*= 0*/)
{
    NET_IO_LOG(INFO) << "link:" << io_ctx->socket_ctx->link;
    io_ctx->operation = NET_IO_OPERATION_WRITE;
    auto& wasbuf = io_ctx->from_buf(offset);
    wasbuf.len = io_ctx->buf_used - offset;
    int ret = WSASend(io_ctx->socket_ctx->link, &wasbuf, 1, nullptr, 0, io_ctx.ptr(), nullptr);
    if (ret != 0) {
        if (ret == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())) {
            return false;
        }
    }
    return true;
}

void CNetIOWin32::CloseLink(net_link link)
{
    NET_IO_LOG(WARNING) << "link:" << link;
    auto f_link = m_active_link.find(link);
    if (!f_link.second) {
        NET_IO_LOG(WARNING) << "not find link:" << link;
        return;
    }
    f_link.first->second->reset();

    // 回收
    m_socket_ctx.recycle(f_link.first->second);
    // 关闭客户端连接
    m_active_link.erase(link);

    if (0 != closesocket(link)) {
        NET_IO_LOG(ERROR) << "close socket error. socket:" << link << " err:" << WSAGetLastError();
    }

}

int CNetIOWin32::Run()
{
    // 创建一个完成端口
    m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, m_recv_work_thread_num);
    if (m_iocp == INVALID_HANDLE_VALUE) {
        NET_IO_LOG(ERROR) << "create IOCP fail! err:" << WSAGetLastError();
        return NET_CREATE_IOCP_FAIL;
    }
    if (CreateThread() != NET_OK) {
        return NET_FAIL;
    }

    int ret = CreateListenNetLink();
    if (ret != NET_OK) {
        return ret;
    }
    ret = CreateClientNetLink();
    if (ret != NET_OK) {
        return ret;
    }
    return NET_OK;
}

net_link CNetIOWin32::AddConnect(const std::string& ip, const uint16_t& port)
{
    auto socket_ctx = m_socket_ctx.get_free_item();
    // 创建socket
    socket_ctx->link = CreateOneNetLink();
    if (socket_ctx->link == INVALID_NET_LINK) {
        NET_IO_LOG(ERROR) << "CreateOneNetLink fail! ";
        m_socket_ctx.recycle(socket_ctx);
        return INVALID_NET_LINK;
    }
    // 绑定SOCKET
    m_iocp = CreateIoCompletionPort((HANDLE)socket_ctx->link, m_iocp, (ULONG_PTR)(socket_ctx.ptr()), m_recv_work_thread_num);
    if (m_iocp == INVALID_HANDLE_VALUE) {
        NET_IO_LOG(FATAL) << "CreateIoCompletionPort fail! err:" << WSAGetLastError();
        m_socket_ctx.recycle(socket_ctx);
        return INVALID_NET_LINK;
    }

    sockaddr_in addr_in{};
    addr_in.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip.c_str(), &addr_in.sin_addr) <= 0) {
        NET_IO_LOG(FATAL) << "inet_pton fail! ip:" << ip;
        m_socket_ctx.recycle(socket_ctx);
        return INVALID_NET_LINK;
    }
    addr_in.sin_port = htons(port);

    if (WSAConnect(socket_ctx->link, (sockaddr*)(&addr_in), sizeof(addr_in), nullptr, nullptr,
        nullptr, nullptr) != 0) {
        NET_IO_LOG(ERROR) << "WSAConnect fail! ip:" << ip << " port:" << port
            << " err:" << WSAGetLastError();
        m_socket_ctx.recycle(socket_ctx);
        return INVALID_NET_LINK;
    }

    if (!PostRead(socket_ctx)) {
        NET_IO_LOG(ERROR) << "PostRead fail! ip:" << ip << " port:" << port
            << " err:" << WSAGetLastError();
        m_socket_ctx.recycle(socket_ctx);
        return INVALID_NET_LINK;
    }

    m_active_link.insert({ socket_ctx->link, socket_ctx });

    return socket_ctx->link;
}

bool CNetIOWin32::RecvData(net_link& socket, net_io_buf& recv_buf, const int32_t& time_out)
{
    if (m_wait_detail.empty()) {
        if (time_out == 0) {
            return false;
        }
        else if (time_out < 0) {
            WaitForSingleObject(m_read_event, INFINITE);
        }
        else {
            WaitForSingleObject(m_read_event, time_out);
        }
    }
    if (m_wait_detail.empty()) {
        return false;
    }

    WSAResetEvent(m_read_event);

    auto iter = m_wait_detail.back();
    auto io_ctx = iter->data();
    memcpy_s(recv_buf.data, recv_buf.LEN, io_ctx->buf, io_ctx->buf_used);
    recv_buf.used = io_ctx->buf_used;

    socket = io_ctx->socket_ctx->link;
    io_ctx->socket_ctx->dec_read_io();
    io_ctx->reset();
    // 放回可使用池
    m_io_ctx.recycle(io_ctx->self);

    m_wait_detail.pop_back();

    return true;
}

int CNetIOWin32::SendData(const net_link& socket, const char* data, const size_t& data_size)
{
    auto f_client = m_active_link.find(socket);
    if (!f_client.second) {
        NET_IO_LOG(ERROR) << "not find socket! socket:" << socket;
        return NET_CLIENT_CLOSED;
    }
    size_t send_size = data_size;
    while (send_size > 0) {
        auto io_ctx = m_io_ctx.get_free_item();
        io_ctx->socket_ctx = f_client.first->second;
        io_ctx->operation = NET_IO_OPERATION_WRITE;
        io_ctx->self = io_ctx;
        memcpy_s(io_ctx->buf, PER_IO_CONTEXT_BUF_LEN, data, data_size);
        io_ctx->buf_used = data_size;
        m_wait_send.push_front(new list_node<PerIOCtx>(io_ctx));
        if (send_size > PER_IO_CONTEXT_BUF_LEN) {
            send_size -= PER_IO_CONTEXT_BUF_LEN;
        }
        else {
            send_size = 0;
        }
    }
    SetEvent(m_write_event);
    return NET_OK;
}

void CNetIOWin32::CleanUp()
{
    // 等待所有线程结束
    WSAWaitForMultipleEvents(m_work_thread_num, m_work_threads, TRUE, WSA_INFINITE, FALSE);

    // 关闭线程HANDLE
    for (size_t i = 0; i < m_work_thread_num; ++i) {
        if (m_work_threads[i] != INVALID_HANDLE_VALUE) {
            CloseHandle(m_work_threads[i]);
            m_work_threads[i] = INVALID_HANDLE_VALUE;
        }
    }
    // 关闭服务端socket
    if (m_listen != INVALID_SOCKET) {
        closesocket(m_listen);
        m_listen = INVALID_SOCKET;
    }

    // 释放正在使用的节点
    m_socket_ctx.clear();

    // 关闭IOCP
    if (m_iocp != INVALID_HANDLE_VALUE) {
        CloseHandle(m_iocp);
        m_iocp = INVALID_HANDLE_VALUE;
    }

    // 关闭事件
    if (m_read_event != WSA_INVALID_EVENT) {
        WSACloseEvent(m_read_event);
    }
    if (m_write_event != WSA_INVALID_EVENT) {
        WSACloseEvent(m_write_event);
    }
    DeleteCriticalSection(&m_critical_section);
    WSACleanup();
}

DWORD CNetIOWin32::RecvDataThread(LPVOID p_net_cs)
{
    CNetIOWin32* p_net = static_cast<CNetIOWin32*>(p_net_cs);
    if (p_net == nullptr) {
        NET_IO_LOG(ERROR) << "p_net_cs is nullptr! thread_id:" << GetCurrentThreadId();
        return NET_FAIL;
    }
    BOOL iocp_ok = FALSE;
    DWORD io_size = 0;
    PerSocketCtx* p_socket_ctx = nullptr;
    PerIOCtx* p_io_ctx = nullptr;
    LPWSAOVERLAPPED p_overlapped = nullptr;
    NET_IO_LOG(INFO) << "start successful! thread_id:" << GetCurrentThreadId();
    while (!p_net->m_finish) {
        iocp_ok = GetQueuedCompletionStatus(
            p_net->m_iocp,
            &io_size,
            (PULONG_PTR)(&p_socket_ctx),
            (LPOVERLAPPED*)(&p_overlapped),
            INFINITE
        );
        if (!iocp_ok) {
            NET_IO_LOG(ERROR) << "GetQueuedCompletionStatus fail! err:" << WSAGetLastError();
        }

        if (p_socket_ctx == nullptr) {
            NET_IO_LOG(ERROR) << "p_socket_ctx is null! err:" << WSAGetLastError();
            break;
        }

        p_io_ctx = (PerIOCtx*)(p_overlapped);

        if (p_io_ctx->operation != NET_IO_OPERATION_ACCEPT) {
            if (!iocp_ok || (iocp_ok && io_size == 0)) {
                NET_IO_LOG(WARNING) << "client broken! link:" << p_io_ctx->socket_ctx->link
                                    << " op:" << p_io_ctx->operation;
                p_net->CloseLink(p_io_ctx->socket_ctx->link);
                p_io_ctx->reset();
                p_net->m_io_ctx.recycle(p_io_ctx->self);
                continue;
            }
        }
        NET_IO_LOG(INFO) << "link:" << p_socket_ctx->link << " op:" << p_io_ctx->operation;
        switch (p_io_ctx->operation) {
        case NET_IO_OPERATION_ACCEPT: {
            p_net->DoAccept(p_io_ctx, io_size);
        }break;
        case NET_IO_OPERATION_READ: {
            p_net->DoRead(p_io_ctx, io_size);
        }break;
        case NET_IO_OPERATION_WRITE: {
            p_net->DoSend(p_io_ctx, io_size);
        }break;
        default: {
            NET_IO_LOG(ERROR) << "[net] WorkThread unknown operation. err:" << p_io_ctx->operation;
        }
        }
    }

    return NET_OK;
}

DWORD CNetIOWin32::SendDataThread(LPVOID p_net_cs)
{
    CNetIOWin32* p_net = static_cast<CNetIOWin32*>(p_net_cs);
    if (p_net == nullptr) {
        NET_IO_LOG(ERROR) << "param is nullptr! thread_id:" << GetCurrentThreadId();
        return FALSE;
    }
    while (!p_net->m_finish) {
        if (p_net->m_wait_send.empty()) {
            WaitForSingleObject(p_net->m_write_event, INFINITE);
        }
        else {
            WSAResetEvent(p_net->m_write_event);
            auto iter = p_net->m_wait_send.back();
            auto send_io_ctx = iter->ptr();
            auto socket_ctx = send_io_ctx->socket_ctx;
            // 当前链接已经失效
            if (!p_net->m_active_link.find(socket_ctx->link).second) {
                NET_IO_LOG(WARNING) << "send link not connect! link:" << socket_ctx->link;
                p_net->m_wait_send.pop_back();
                p_net->m_io_ctx.recycle(send_io_ctx->self);
                continue;
            }
            if (!p_net->PostWrite(send_io_ctx->self)) {
                p_net->m_io_ctx.recycle(send_io_ctx->self);
                p_net->m_active_link.erase(socket_ctx->link);
            }
            p_net->m_wait_send.pop_back();
        }
    }
    return TRUE;
}

#endif