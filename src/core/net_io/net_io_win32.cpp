#include "net_io_win32.h"

#ifdef OS_WIN

#include <core/tools/buffer.h>
#include <core/tools/gs_assert.h>

const static uint32_t MIN_SOCKET_IO_LEN = sizeof(int32_t);
const static uint32_t MIN_POST_READ_IO_SIZE = 2;
const static uint32_t MAX_WAIT_DETAIL_SOCKET = 100;
const static int32_t MAX_IN_DETAIL = 2;
struct PerSocketCtx
{
    net_link link;

    LPFN_ACCEPTEX accept_fn;

    /* 投递读取的IO数量 */
    uint32_t n_post_read;

    safe_recycle_list<PerSocketCtx>::iterator self;

    /* 接收到的数据 */
    const static uint32_t MAX_RECV_DATA = NET_MSG_MAX_LEN * 10; // 最多存在10条消息
    slide_buffer recv;

    /* 等待处理的排队数 */
    int32_t n_wait;

    PerSocketCtx()
        : link(INVALID_NET_LINK)
        , accept_fn(nullptr)
        , n_post_read(0)
        , n_wait(0)
    {

    }
    void reset()
    {
        link = INVALID_NET_LINK;
        accept_fn = nullptr;
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
    }
};

CNetIOWin32::CNetIOWin32()
    : m_iocp(INVALID_HANDLE_VALUE)
    , m_port(DEFAULT_LISTEN_PORT)
    , m_recv_thread_num(0)
    , m_work_thread_num(NET_DEFAULT_WORK_THREAD_NUM)
    , m_listen(INVALID_SOCKET)
    , m_finish(false)
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
    m_work_thread_num = num;
    if (num < NET_DEFAULT_WORK_THREAD_NUM) {
        SYSTEM_INFO sys_info;
        GetSystemInfo(&sys_info);
        m_work_thread_num = sys_info.dwNumberOfProcessors;
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

    LOG(INFO) << "init env finish! work_thread_num:" << m_work_thread_num << " port:" << m_port;
    return NET_OK;
}
int CNetIOWin32::CreateThread() {
    // 创建接收线程
    for (uint32_t i = 0; i < m_work_thread_num; ++i) {
        if (i & 1) {
            m_work_threads.emplace_back(CNetIOWin32::SendDataThread, this);
        } else {
            m_work_threads.emplace_back(CNetIOWin32::RecvDataThread, this);
        }
    }
    m_recv_thread_num = (m_work_threads.size() + 1) / 2;

    for (auto && thread : m_work_threads) {
        thread.detach();
    }
    return NET_OK;
}

net_link CNetIOWin32::CreateOneNetLink()
{
    net_link link = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (link == INVALID_NET_LINK) {
        LOG(ERROR) << "create socket fail! err:" << WSAGetLastError();
        return link;
    }
    int zero = 0;
    int ret = setsockopt(link, SOL_SOCKET, SO_SNDBUF, (char*)&zero, sizeof(zero));
    if (ret == SOCKET_ERROR) {
        LOG(ERROR) << "socket setsockopt error! err:" << WSAGetLastError();
        closesocket(link);
        return INVALID_NET_LINK;
    }
    return link;
}

int CNetIOWin32::CreateListenNetLink()
{
    //创建一个socket
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_IP;

    char listen_port_str[7];

    if (sprintf_s(listen_port_str, 7, "%hu", m_port) <= 0) {
        LOG(ERROR) << "port convert to string error ! " << m_port;
        return NET_LISTEN_PORT_FAIL;
    }
    struct addrinfo* p_addr_local = nullptr;
    if (0 != getaddrinfo(nullptr, listen_port_str, &hints, &p_addr_local)) {
        LOG(ERROR) << "getaddrinfo fail! err:" << WSAGetLastError();
        return NET_FAIL;
    }

    if (p_addr_local == nullptr) {
        LOG(ERROR) << "getaddrinfo() failed to resolve/convert the interface! err:" << WSAGetLastError();
        return NET_FAIL;
    }


    m_listen = CreateOneNetLink();
    if (m_listen == INVALID_NET_LINK) {
        freeaddrinfo(p_addr_local);
        return NET_CREATE_SOCKET_FAIL;
    }

    auto socket_ctx = m_socket_ctx.get_free_item();
    socket_ctx->link = m_listen;
    socket_ctx->self = socket_ctx;
    // 将listen socket绑定到iocp
    m_iocp = CreateIoCompletionPort((HANDLE)m_listen, m_iocp, (ULONG_PTR)(&(*socket_ctx)), m_recv_thread_num);
    if (m_iocp == INVALID_HANDLE_VALUE) {
        return NET_UPDATE_IOCP_FAIL;
    }

    int ret = bind(m_listen, p_addr_local->ai_addr, int(p_addr_local->ai_addrlen));
    if (SOCKET_ERROR == ret) {
        LOG(ERROR) << "bind! err:" << WSAGetLastError();
        freeaddrinfo(p_addr_local);
        return NET_LISTEN_PORT_FAIL;
    }

    // 设置最大连接数
    ret = listen(m_listen, SOMAXCONN);
    if (SOCKET_ERROR == ret) {
        LOG(ERROR) << "listen! err:" << WSAGetLastError();
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
    socket_ctx->self = socket_ctx;

    if (socket_ctx->link == INVALID_NET_LINK) {
        LOG(ERROR) << "create one link fail! ";
        socket_ctx->reset();
        m_socket_ctx.recycle(socket_ctx);
        return NET_FAIL;
    }

    m_iocp = CreateIoCompletionPort((HANDLE)socket_ctx->link, m_iocp, (ULONG_PTR)(&(*socket_ctx)), m_recv_thread_num);
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
        &socket_ctx->accept_fn,
        sizeof(socket_ctx->accept_fn),
        &bytes,
        nullptr,
        nullptr
    );
    // 获取accept func失败
    if (ret == SOCKET_ERROR) {
        LOG(ERROR) << "WSAIoctl fail! err:" << WSAGetLastError();
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
    ret = socket_ctx->accept_fn(m_listen,
        socket_ctx->link,
        io_ctx->buf,
        NET_MSG_DEFAULT_BUFFER_SIZE - (2 * (sizeof(SOCKADDR_STORAGE) + 16)),
        sizeof(SOCKADDR_STORAGE) + 16, sizeof(SOCKADDR_STORAGE) + 16,
        &recv_bytes_num,
        &(*io_ctx)
    );
    if (ret == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())) {
        LOG(ERROR) << "AcceptEx fail! err:" << WSAGetLastError();
        io_ctx->reset();
        m_io_ctx.recycle(io_ctx);
        socket_ctx->reset();
        m_socket_ctx.recycle(socket_ctx);
        return NET_FAIL;
    }

    return NET_OK;
}

void CNetIOWin32::DoAccept(PerIOCtxIter io_ctx, DWORD io_size)
{
    auto socket_ctx = io_ctx->socket_ctx;
    int ret = setsockopt(socket_ctx->link,
        SOL_SOCKET,
        SO_UPDATE_ACCEPT_CONTEXT,
        (char*)&m_listen,
        sizeof(m_listen)
    );
    if (ret == SOCKET_ERROR) {
        LOG(ERROR) << "setsockopt fail. err:" << WSAGetLastError();
        CloseLink(socket_ctx->link);
        return;
    }
    LOG(INFO) << "new link. link:" << socket_ctx->link;
    m_active_link.insert({ socket_ctx->link, socket_ctx });

    if (io_size > 0) {
        DoRead(io_ctx, io_size);
    }
    

    if (!PostRead(socket_ctx)) {
        CloseLink(socket_ctx->link);
    }
    // 创建一个等待监听的socket
    CreateClientNetLink();
}

void CNetIOWin32::DoSend(PerIOCtxIter io_ctx, DWORD io_size)
{
    io_ctx->buf_offset += io_size;

    if (io_ctx->buf_offset >= io_ctx->buf_used) {
        if (!PostRead(io_ctx->socket_ctx)) {
            CloseLink(io_ctx->socket_ctx->link);
        }
        m_io_ctx.recycle(io_ctx);
    }
    else {
        if (!PostWrite(io_ctx, io_ctx->buf_offset)) {
            LOG(ERROR) << "send data fail! link:" << io_ctx->socket_ctx->link << " err:" << WSAGetLastError();
            m_io_ctx.recycle(io_ctx->self);
            CloseLink(io_ctx->socket_ctx->link);
        }
    }
}

void CNetIOWin32::DoRead(PerIOCtxIter io_ctx, DWORD io_size)
{
    PerSocketCtxIter socket_ctx = io_ctx->socket_ctx;
    socket_ctx->dec_read_io();
    // LOG(INFO) << "read begin! size:" << io_size;
    // 锁定写入buf
    // 写入数据
    socket_ctx->recv.safe_write(io_ctx->buf, io_size);

    // 锁定待读取列表
    std::unique_lock<std::mutex> lock(m_recv_mx);
    m_recv_cv.wait(lock);

    // 将socket加入到队首
    if (socket_ctx->n_wait < MAX_IN_DETAIL) {
        socket_ctx->n_wait++;
        m_wait_detail.push_front(socket_ctx);
    }
    // 解锁读取列表
    lock.unlock();
    m_recv_cv.notify_all();

    // 回收
    io_ctx->reset();
    m_io_ctx.recycle(io_ctx);


    PostRead(socket_ctx);
    // LOG(INFO) << "read end! size:" << io_size;
}

bool CNetIOWin32::PostRead(PerSocketCtxIter socket_ctx)
{
    if (socket_ctx->recv.len() > PerSocketCtx::MAX_RECV_DATA && m_wait_detail.size() > MAX_WAIT_DETAIL_SOCKET) {
        return true;
    }
    DWORD flag = 0;
    while (socket_ctx->need_post_read()) {
        auto io_ctx = m_io_ctx.get_free_item();
        io_ctx->socket_ctx = socket_ctx;
        io_ctx->operation = NET_IO_OPERATION_READ;
        io_ctx->self = io_ctx;

        auto& wsabuf = io_ctx->from_buf();

        if (WSARecv(socket_ctx->link, &wsabuf, 1, nullptr, &flag, (&(*io_ctx)), nullptr) == SOCKET_ERROR) {
            if (WSAGetLastError() != WSA_IO_PENDING) {
                m_io_ctx.recycle(io_ctx);
                return false;
            }
        }
        socket_ctx->inc_read_io();
    }
    return true;
}

bool CNetIOWin32::PostWrite(PerIOCtxIter io_ctx, const size_t& offset /*= 0*/)
{
    io_ctx->operation = NET_IO_OPERATION_WRITE;

    auto& was_buf = io_ctx->from_buf(offset);

    was_buf.len = io_ctx->buf_used - offset;

    int ret = WSASend(io_ctx->socket_ctx->link, &was_buf, 1, nullptr, 0, (&(*io_ctx)), nullptr);
    if (ret != 0) {
        if (ret == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())) {
            return false;
        }
    }
    return true;
}

void CNetIOWin32::CloseLink(net_link link)
{
    LOG(WARNING) << "link:" << link;
    auto f_link = m_active_link.find(link);
    if (!f_link.second) {
        LOG(WARNING) << "not find link:" << link;
        return;
    }
    f_link.first->second->reset();

    // 回收
    m_socket_ctx.recycle(f_link.first->second->self);
    // 关闭客户端连接
    m_active_link.erase(link);

    if (0 != closesocket(link)) {
        LOG(ERROR) << "close socket error. socket:" << link << " err:" << WSAGetLastError();
    }

}

int CNetIOWin32::Run()
{
    // 创建一个完成端口
    m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, m_recv_thread_num);
    if (m_iocp == INVALID_HANDLE_VALUE) {
        LOG(ERROR) << "create IOCP fail! err:" << WSAGetLastError();
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
    socket_ctx->self = socket_ctx;
    // 创建socket
    socket_ctx->link = CreateOneNetLink();
    if (socket_ctx->link == INVALID_NET_LINK) {
        LOG(ERROR) << "CreateOneNetLink fail! ";
        m_socket_ctx.recycle(socket_ctx);
        return INVALID_NET_LINK;
    }
    // 绑定SOCKET
    m_iocp = CreateIoCompletionPort((HANDLE)socket_ctx->link, m_iocp, (ULONG_PTR)(&(*socket_ctx)), m_recv_thread_num);
    if (m_iocp == INVALID_HANDLE_VALUE) {
        LOG(FATAL) << "CreateIoCompletionPort fail! err:" << WSAGetLastError();
        m_socket_ctx.recycle(socket_ctx);
        return INVALID_NET_LINK;
    }

    sockaddr_in addr_in{};
    addr_in.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip.c_str(), &addr_in.sin_addr) <= 0) {
        LOG(FATAL) << "inet_pton fail! ip:" << ip;
        m_socket_ctx.recycle(socket_ctx);
        return INVALID_NET_LINK;
    }
    addr_in.sin_port = htons(port);

    if (WSAConnect(socket_ctx->link, (sockaddr*)(&addr_in), sizeof(addr_in), nullptr, nullptr,
        nullptr, nullptr) != 0) {
        LOG(ERROR) << "WSAConnect fail! ip:" << ip << " port:" << port
            << " err:" << WSAGetLastError();
        m_socket_ctx.recycle(socket_ctx);
        return INVALID_NET_LINK;
    }

    if (!PostRead(socket_ctx)) {
        LOG(ERROR) << "PostRead fail! ip:" << ip << " port:" << port
            << " err:" << WSAGetLastError();
        m_socket_ctx.recycle(socket_ctx);
        return INVALID_NET_LINK;
    }

    m_active_link.insert({ socket_ctx->link, socket_ctx });

    return socket_ctx->link;
}

bool CNetIOWin32::RecvData(net_link& socket, NetMsgBufList& recv_buf, const int32_t& time_out)
{
    if (time_out < 0 && m_wait_detail.empty()) {
        return false;
    }

    /* 取得队尾的量 */
    std::unique_lock<std::mutex> lock(m_recv_mx);

    if (time_out == 0) {
        m_recv_cv.wait(lock);
    } else {
        m_recv_cv.wait_for(lock, std::chrono::microseconds(time_out));
    }

    if (m_wait_detail.empty()) {
        lock.unlock();
        m_recv_cv.notify_all();
        return false;
    }

    auto socket_ctx = m_wait_detail.back();

    m_wait_detail.pop_back();

    LOG_IF(WARNING, m_wait_detail.size() > MAX_WAIT_DETAIL_SOCKET) << " recv_size:" << m_wait_detail.size();

    lock.unlock();
    m_recv_cv.notify_all();

    socket = socket_ctx->link;
    /* 循环读取数据包 */
    auto& buf = socket_ctx->recv;
    auto iter = recv_buf.data.begin();
    recv_buf.count = 0;
    uint32_t buf_len = 0;
    buf.lock();
    while (buf.len() >= MIN_SOCKET_IO_LEN) {
        buf.read((char*)&buf_len, MIN_SOCKET_IO_LEN, false);
        if (buf_len + MIN_SOCKET_IO_LEN > buf.len()) {
            break;
        }
        if (buf_len > NET_MSG_MAX_LEN) {
            LOG(WARNING) << "recv msg too large! max:" << NET_MSG_MAX_LEN << " your:" << buf_len;
            buf.skip(buf_len + MIN_SOCKET_IO_LEN);
            continue;
        }
        buf.skip(MIN_SOCKET_IO_LEN);
        buf.read(iter->data, buf_len);
        iter->len = buf_len;
        recv_buf.count++;
        ++iter;
        if (iter == recv_buf.data.end()) {
            break;
        }
    }
    socket_ctx->n_wait--;
    buf.unlock();
    PostRead(socket_ctx);
    
    return true;
}

int CNetIOWin32::SendData(const net_link& socket, const char* data, const uint32_t& data_size)
{
    if (data == nullptr || data_size == 0) {
        return NET_OK;
    }

    if (data_size > NET_MSG_MAX_LEN) {
        LOG(ERROR) << "msg too large! max:" << NET_MSG_MAX_LEN << " your:" << data_size;
        return NET_MSG_TOO_LARGE;
    }

    auto f_client = m_active_link.find(socket);
    if (!f_client.second) {
        LOG(ERROR) << "not find socket! socket:" << socket;
        return NET_CLIENT_CLOSED;
    }

    size_t send_size = data_size;
    bool add_len = true;
    while (send_size > 0) {
        auto io_ctx = m_io_ctx.get_free_item();
        io_ctx->socket_ctx = f_client.first->second;
        io_ctx->operation = NET_IO_OPERATION_WRITE;
        io_ctx->self = io_ctx;
        if (add_len) {
            io_ctx->buf_used = MIN_SOCKET_IO_LEN;
            memcpy_s(io_ctx->buf, PER_IO_CONTEXT_BUF_LEN, &data_size, MIN_SOCKET_IO_LEN);
            add_len = false;
        }

        memcpy_s(io_ctx->buf + io_ctx->buf_used, PER_IO_CONTEXT_BUF_LEN - io_ctx->buf_used, data, data_size);

        if (send_size >= PER_IO_CONTEXT_BUF_LEN - io_ctx->buf_used) {
            send_size -= PER_IO_CONTEXT_BUF_LEN - io_ctx->buf_used;
            io_ctx->buf_used = PER_IO_CONTEXT_BUF_LEN;
        }
        else {
            send_size = 0;
            io_ctx->buf_used += data_size;
        }

        std::unique_lock<std::mutex> lock(m_send_mx);
        m_wait_send.push_front(io_ctx);
        lock.unlock();
        m_send_cv.notify_all();
    }
    return NET_OK;
}

void CNetIOWin32::CleanUp()
{
    // 关闭服务端socket
    if (m_listen != INVALID_SOCKET) {
        closesocket(m_listen);
        m_listen = INVALID_SOCKET;
    }

    // 关闭IOCP
    if (m_iocp != INVALID_HANDLE_VALUE) {
        CloseHandle(m_iocp);
        m_iocp = INVALID_HANDLE_VALUE;
    }

    WSACleanup();
}

void CNetIOWin32::RecvDataThread(CNetIOWin32* pThis)
{
    BOOL iocp_ok = FALSE;
    DWORD io_size = 0;
    PerSocketCtx* p_socket_ctx = nullptr;
    PerIOCtx* p_io_ctx = nullptr;
    LPWSAOVERLAPPED p_overlapped = nullptr;
    LOG(INFO) << "start recv data thread successful! thread_id:" << GetCurrentThreadId();
    while (!pThis->m_finish) {
        iocp_ok = GetQueuedCompletionStatus(
            pThis->m_iocp,
            &io_size,
            (PULONG_PTR)(&p_socket_ctx),
            (LPOVERLAPPED*)(&p_overlapped),
            INFINITE
        );
        if (!iocp_ok) {
            LOG(ERROR) << "GetQueuedCompletionStatus fail! err:" << WSAGetLastError();
        }

        if (p_socket_ctx == nullptr) {
            LOG(ERROR) << "p_socket_ctx is null! err:" << WSAGetLastError();
            break;
        }

        p_io_ctx = (PerIOCtx*)(p_overlapped);

        if (p_io_ctx->operation != NET_IO_OPERATION_ACCEPT) {
            if (!iocp_ok || (iocp_ok && io_size == 0)) {
                LOG(WARNING) << "client broken! link:" << p_io_ctx->socket_ctx->link
                                    << " op:" << p_io_ctx->operation;
                pThis->CloseLink(p_io_ctx->socket_ctx->link);
                p_io_ctx->reset();
                pThis->m_io_ctx.recycle(p_io_ctx->self);
                continue;
            }
        }
        // LOG(INFO) << "link:" << p_socket_ctx->link << " op:" << p_io_ctx->operation;
        switch (p_io_ctx->operation) {
        case NET_IO_OPERATION_ACCEPT: {
            pThis->DoAccept(p_io_ctx->self, io_size);
        }break;
        case NET_IO_OPERATION_READ: {
            pThis->DoRead(p_io_ctx->self, io_size);
        }break;
        case NET_IO_OPERATION_WRITE: {
            pThis->DoSend(p_io_ctx->self, io_size);
        }break;
        default: {
            LOG(ERROR) << "[net] WorkThread unknown operation. err:" << p_io_ctx->operation;
        }
        }
    }
}

void CNetIOWin32::SendDataThread(CNetIOWin32* pThis)
{
    LOG(INFO) << "start send data thread successful! thread_id:" << GetCurrentThreadId();
    while (!pThis->m_finish) {
        std::unique_lock<std::mutex> lock(pThis->m_send_mx);
        while (pThis->m_wait_send.empty()) {
            pThis->m_send_cv.wait(lock);
        }
        auto send_io_ctx = pThis->m_wait_send.back();
        pThis->m_wait_send.pop_back();
        LOG_IF(INFO, pThis->m_wait_send.size() > 50) << " send_size:" << pThis->m_wait_send.size();
        lock.unlock();
        pThis->m_send_cv.notify_all();

        auto socket_ctx = send_io_ctx->socket_ctx;
        // 当前链接已经失效
        if (!pThis->m_active_link.find(socket_ctx->link).second) {
            LOG(WARNING) << "send link not connect! link:" << socket_ctx->link;
            send_io_ctx->reset();
            pThis->m_io_ctx.recycle(send_io_ctx);
            continue;
        }
        if (!pThis->PostWrite(send_io_ctx)) {
            send_io_ctx->reset();
            pThis->m_io_ctx.recycle(send_io_ctx->self);
            pThis->m_active_link.erase(socket_ctx->link);
        }
    }
}

#endif