#include "net_io_linux.h"
#ifdef OS_LINUX
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <glog/logging.h>
#include <core/tools/gs_assert.h>
#include <core/tools/buffer.h>


const uint32_t MAX_EPOLL_EVENT_NUM = 10;
const int32_t INVALID_EPOLL = -1;
const static uint32_t MIN_SOCKET_IO_LEN = sizeof(int32_t);
const static uint32_t MIN_POST_READ_IO_SIZE = 2;
const static uint32_t MAX_WAIT_DETAIL_SOCKET = 100;
const static int32_t MAX_IN_DETAIL = 2;
const static size_t WARNING_SEND_IO_NUM = 100;

struct PerSocketCtx
{
    net_link link;
    int32_t epoll;
    slide_buffer recv;
    int32_t n_wait;
    struct epoll_event event;
    safe_recycle_list<PerSocketCtx>::iterator self;
    PerSocketCtx()
        : link(INVALID_NET_LINK)
        , epoll(INVALID_EPOLL)
        , n_wait(0)
    {

    }
};

const static uint32_t PER_IO_CONTEXT_BUF_LEN = NET_MSG_DEFAULT_BUFFER_SIZE;
struct PerIOCtx
{
    char data[PER_IO_CONTEXT_BUF_LEN];
    size_t data_len;
    safe_recycle_list<PerSocketCtx>::iterator sd;
    safe_recycle_list<PerIOCtx>::iterator self;

    PerIOCtx()
        : data_len(0)
    {

    }
};

CNetIOLinux::CNetIOLinux()
    : m_port(0)
    , m_epoll(-1)
{

}
CNetIOLinux::~CNetIOLinux()
{

}

int CNetIOLinux::SetListenPort(const uint16_t& port)
{
    AlwaysAssert(port > 1500);
    m_port = port;
    return NET_OK;
}
int CNetIOLinux::SetWorkThreadNum(const uint32_t& num)
{
    m_thread_num = num;
    if (num < NET_DEFAULT_WORK_THREAD_NUM) {
        m_thread_num = std::thread::hardware_concurrency() * 2;
    }
    m_recv_thread_num = (m_thread_num + 1) / 2;
    return NET_OK;
}

int CNetIOLinux::Initiate()
{
    return NET_OK;
}

int CNetIOLinux::Run()
{
    if (!CreateListenSocket()) {
        return NET_CREATE_SOCKET_FAIL;
    }
    if (!CreateAcceptEpoll()) {
        CleanUp();
        return NET_CREATE_EPOLL_FAIL;
    }
    if (!CreateRecvEpoll()) {
        CleanUp();
        return NET_CREATE_EPOLL_FAIL;
    }
    if (!AddSocketToEpoll(m_listen, m_epoll)) {
        LOG(FATAL) << "add listen socket to epoll fail!";
        return NET_FAIL;
    }
    if (!CreateThread()) {
        LOG(FATAL) << "create thread fail!";
        return NET_FAIL;
    }
    return NET_OK;
}

net_link CNetIOLinux::AddConnect(const std::string& ip, const uint16_t& port)
{
    net_link link = INVALID_NET_LINK;
    link = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (link == INVALID_NET_LINK) {
        LOG(ERROR) << "create socket fail! err:" << errno;
        return link;
    }

    struct sockaddr_in sa;
    bzero(&sa, sizeof(struct sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &sa.sin_addr) <= 0) {
        LOG(ERROR) << "invalid ip:" << ip << " err:" << errno;
        close(link);
        return INVALID_NET_LINK;
    }

    if (connect(link, (struct sockaddr*)(&sa), sizeof(sa)) != 0) {
        LOG(ERROR) << "connect fail! ip:" << ip << " port:"<< port <<" err:" << errno;
        close(link);
        return INVALID_NET_LINK;
    }

    if (!AddSocketToEpoll(link, m_recv_epoll[link % m_recv_thread_num])) {
        LOG(ERROR) << "add to epoll fail!";
        close(link);
        return INVALID_NET_LINK;
    }
    LOG(INFO) << "add link ok! link:" << link;
    return link;
}

bool CNetIOLinux::RecvData(net_link& socket, NetMsgBufList& recv_buf, const int32_t& time_out /*= -1*/)
{
     if (time_out < 0 && m_recv_sd.empty()) {
        return false;
    }

    /* 取得队尾的量 */
    std::unique_lock<std::mutex> lock(m_recv_mx);

    if (time_out == 0) {
        m_recv_cv.wait(lock);
    } else {
        m_recv_cv.wait_for(lock, std::chrono::microseconds(time_out));
    }

    if (m_recv_sd.empty()) {
        lock.unlock();
        m_recv_cv.notify_all();
        return false;
    }

    auto socket_ctx = m_recv_sd.back();

    m_recv_sd.pop_back();

    LOG_IF(WARNING, m_recv_sd.size() > MAX_WAIT_DETAIL_SOCKET) << " recv_size:" << m_recv_sd.size();

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
    // LOG_IF(INFO, recv_buf.count > 0) << "recv data. sd:" << socket << " cnt:" << recv_buf.count;
    return true;
}

int CNetIOLinux::SendData(const net_link& socket, const char* data, const uint32_t& data_size)
{
    if (data == nullptr || data_size == 0) {
        return NET_OK;
    }

    if (data_size > NET_MSG_MAX_LEN) {
        LOG(ERROR) << "msg too large! max:" << NET_MSG_MAX_LEN << " your:" << data_size;
        return NET_MSG_TOO_LARGE;
    }

    auto f_client = m_link_map.find(socket);
    if (!f_client.second) {
        LOG(ERROR) << "not find socket! socket:" << socket;
        return NET_CLIENT_CLOSED;
    }
    uint32_t send_size = data_size;
    bool add_len = true;
    while (send_size > 0) {
        auto io_ctx = m_io_ctx.get_free_item();
        io_ctx->sd = f_client.first->second;
        io_ctx->self = io_ctx;
        io_ctx->data_len = 0;
        if (add_len) {
            io_ctx->data_len = MIN_SOCKET_IO_LEN;
            memcpy(io_ctx->data, &data_size, MIN_SOCKET_IO_LEN);
            add_len = false;
        }

        if (send_size >= PER_IO_CONTEXT_BUF_LEN - io_ctx->data_len) {
            memcpy(io_ctx->data + MIN_SOCKET_IO_LEN, data, PER_IO_CONTEXT_BUF_LEN - io_ctx->data_len);
            io_ctx->data_len = PER_IO_CONTEXT_BUF_LEN;
            send_size -= PER_IO_CONTEXT_BUF_LEN - io_ctx->data_len;
        } else {
            memcpy(io_ctx->data + MIN_SOCKET_IO_LEN, data, send_size);
            io_ctx->data_len += send_size;
            send_size = 0;
        }
        std::unique_lock<std::mutex> lock(m_send_mx);
        m_send_io.push_front(io_ctx);
        lock.unlock();
        m_send_cv.notify_all();
    }
    return NET_OK;
}

void CNetIOLinux::CleanUp()
{
    if (m_listen != INVALID_NET_LINK) {
        close(m_listen);
        m_listen = INVALID_NET_LINK;
    }
}

bool CNetIOLinux::CreateAcceptEpoll()
{
    m_epoll = epoll_create1(0);
    LOG_IF(FATAL, m_epoll == -1) << "can't create accept epoll handle. err:" << errno;
    return true;
}

bool CNetIOLinux::CreateRecvEpoll()
{
    for (uint32_t i = 0; i < m_recv_thread_num; ++i) {
        epoll_t e = epoll_create1(0);
        LOG_IF(FATAL, e == -1) << "can't create recv epoll handle. err:" << errno;
        m_recv_epoll.emplace_back(e);
    }
    LOG(INFO) << "recv epoll num:" << m_recv_epoll.size();
    return true;
}

bool CNetIOLinux::CreateThread()
{
    int32_t epoll_index = 0;
    for (uint32_t i = 0; i < m_thread_num; ++i) {
        if (i & 1) {
            m_send_data.emplace_back(std::thread(SendDataThread, this));
            m_send_data.back().detach();
        } else {
            m_recv_data.emplace_back(std::thread(RecvDataThread, this, m_recv_epoll[epoll_index++]));
            m_recv_data.back().detach();
        }
    }
    m_accept = std::thread(AcceptThread, this);
    return true;
}

bool CNetIOLinux::CreateListenSocket()
{
    m_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    LOG_IF(FATAL, m_listen == -1) << "can't create listen scoket!" << errno;
    struct sockaddr_in sa;
    bzero(&sa, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(m_port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(m_listen, (struct sockaddr*)(&sa), sizeof(struct sockaddr)) == -1) {
        LOG(FATAL) << "can't bind listen scoket!" << errno;
        return false;
    }
    if (!SetSocketNonblocking(m_listen)) {
        LOG(FATAL) << "set listen scoket to noblock fail!";
        return false;
    }
    if (listen(m_listen, SOMAXCONN) == -1) {
        LOG(FATAL) << "listen scoket to noblock fail! err:" << errno;
        return false;
    }
    return true;
}
bool CNetIOLinux::SetSocketNonblocking(net_link sd)
{
    int flags = fcntl(sd, F_GETFL, 0);
    if (flags == -1) {
        LOG(ERROR) << "fcntl F_GETFL fail! err:" << errno;
        return false;
    }
    if (fcntl(sd, F_SETFL, flags | O_NONBLOCK) == -1) {
        LOG(ERROR) << "fcntl F_SETFL fail! err:" << errno;
        return false;
    }
    return true;
}

bool CNetIOLinux::AddSocketToEpoll(net_link sd, epoll_t epoll)
{
    auto socket_ctx = m_socket_ctx.get_free_item();
    socket_ctx->self = socket_ctx;
    socket_ctx->link = sd;
    socket_ctx->epoll = epoll;

    bzero(&socket_ctx->event, sizeof(epoll_event));
    socket_ctx->event.data.ptr = &(*socket_ctx);
    socket_ctx->event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, sd, &socket_ctx->event) == -1) {
        LOG(ERROR) << "add socket to epoll fail! err:" << errno;
        m_socket_ctx.recycle(socket_ctx);
        return false;
    }
    m_link_map.insert({sd, socket_ctx});
    LOG(INFO) << "new client link. sd:" << sd << " size:" << m_link_map.size() << " epoll:" << epoll;
    return true;
}

void CNetIOLinux::AcceptThread(CNetIOLinux* pThis)
{
    LOG(INFO) << "create accept thread successful! id:" << std::this_thread::get_id();
    struct epoll_event events[MAX_EPOLL_EVENT_NUM];
    bzero(events, sizeof(struct epoll_event) * MAX_EPOLL_EVENT_NUM);
    int nRetEvent = 0;
    while (true) {
        nRetEvent = epoll_wait(pThis->m_epoll, events, MAX_EPOLL_EVENT_NUM, -1);
        if (nRetEvent < -1) {
            LOG(ERROR) << "epoll wait fail! err:" << errno;
            break;
        }
        for (int i = 0; i < nRetEvent; ++i) {
            PerSocketCtx* p_socket = (PerSocketCtx*)(events[i].data.ptr);
            if (p_socket == nullptr) {
                LOG(ERROR) << "epoll event data ptr is null!";
                continue;
            }

            if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                pThis->DoError(p_socket->self);
                continue;
            }
            
            // 有新连接到了
            if (p_socket->link == pThis->m_listen) {
                if (events[i].events & EPOLLIN) {
                    pThis->DoAccept();
                } else {
                    LOG(WARNING) << "may error!";
                }
                continue;
            }
            // 不应该走到这
            LOG(WARNING) << "unknow event. sd:" << p_socket->link << " event:" << events[i].events;
        }
    }
}

void CNetIOLinux::RecvDataThread(CNetIOLinux* pThis, epoll_t epoll)
{
    // LOG(INFO) << "create recv thread successful! id:" << std::this_thread::get_id() << " epoll:" << epoll;
    struct epoll_event events[MAX_EPOLL_EVENT_NUM];
    bzero(events, sizeof(struct epoll_event) * MAX_EPOLL_EVENT_NUM);
    int nRetEvent = 0;
    while (true) {
        nRetEvent = epoll_wait(epoll, events, MAX_EPOLL_EVENT_NUM, -1);
        if (nRetEvent < -1) {
            LOG(ERROR) << "epoll wait fail! err:" << errno;
            break;
        }
        for (int i = 0; i < nRetEvent; ++i) {
            PerSocketCtx* p_socket = (PerSocketCtx*)(events[i].data.ptr);
            if (p_socket == nullptr) {
                LOG(ERROR) << "epoll event data ptr is null!";
                continue;
            }

            if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                pThis->DoError(p_socket->self);
                continue;
            }

            // 处理读取事件
            if (events[i].events & EPOLLIN) {
                pThis->DoRead(p_socket->self);
                continue;
            } else if(events[i].events & EPOLLOUT) {
                pThis->PostRead(p_socket->self);
                continue;
            }
            LOG(WARNING) << "unknow event! epoll:" << epoll << " sd:" << p_socket->link << " event:" << events[i].events;
        }
    }
}


void CNetIOLinux::DoError(PerSocketCtxIter sd_ctx)
{
    LOG(ERROR) << "link will close! sd:" << sd_ctx->link;

    if (epoll_ctl(sd_ctx->epoll, EPOLL_CTL_DEL, sd_ctx->link, &sd_ctx->event) != 0) {
        LOG(ERROR) << "rm link fail! link:" << sd_ctx->link << " err:" << errno;
    }

    if (sd_ctx->link != INVALID_NET_LINK) {
        close(sd_ctx->link);
    }
    m_link_map.erase(sd_ctx->link);
    m_socket_ctx.recycle(sd_ctx->self);
    
}
void CNetIOLinux::DoRead(PerSocketCtxIter sd_ctx)
{
    // LOG(INFO) << "read 1 sd:" << sd_ctx->link << " epoll:" << sd_ctx->epoll;
    int cnt = 0;
    while (true) {
        auto io_ctx = m_io_ctx.get_free_item();
        io_ctx->self = io_ctx;
        cnt = read(sd_ctx->link, io_ctx->data, NET_MSG_DEFAULT_BUFFER_SIZE);
        if (cnt < 0) {
            if (errno != EAGAIN) {
                LOG(ERROR) << "read link data fail! link:" << sd_ctx->link << " err:" << errno;
                m_io_ctx.recycle(io_ctx);
                DoError(sd_ctx);
            }
            break;
        } else if (cnt > 0) {
            io_ctx->data_len = cnt;
            io_ctx->sd = sd_ctx->self;
            // LOG(INFO) << "read sd:" << sd_ctx->link << " len:" << cnt;
            // 锁定待读取列表
            std::unique_lock<std::mutex> lock(m_recv_mx);
            m_recv_cv.wait(lock);
            sd_ctx->recv.safe_write(io_ctx->data, io_ctx->data_len);
            if (sd_ctx->n_wait < MAX_IN_DETAIL) {
                sd_ctx->n_wait++;
                m_recv_sd.push_front(io_ctx->sd);
            }
            lock.unlock();
            m_recv_cv.notify_all();
            
        } else {
            break;
        }
    }
    PostRead(sd_ctx);
    // LOG(INFO) << "read 2 sd:" << sd_ctx->link << " epoll:" << sd_ctx->epoll;
}

void CNetIOLinux::PostRead(PerSocketCtxIter sd_ctx)
{
    // 投递读取事件
    bzero(&sd_ctx->event, sizeof(struct epoll_event));
    sd_ctx->event.events = EPOLLIN | EPOLLET;
    sd_ctx->event.data.ptr = &(*sd_ctx);
    if (epoll_ctl(sd_ctx->epoll, EPOLL_CTL_MOD, sd_ctx->link, &sd_ctx->event) != 0) {
        LOG(ERROR) << "modify epoll fail! link:" << sd_ctx->link << " err:" << errno;
    }
}

void CNetIOLinux::DoWrite(PerIOCtxIter io_ctx)
{
    if (!m_link_map.find(io_ctx->sd->link).second) {
        LOG(ERROR) << "link is broken!";
        return;
    }
    int cnt = write(io_ctx->sd->link, io_ctx->data, io_ctx->data_len);
    if (cnt < 0) {
        DoError(io_ctx->sd);
        return;
    }
    // LOG(INFO) << "do send data. sd:" << io_ctx->sd->link << " size:" << io_ctx->data_len;
    struct epoll_event event;
    bzero(&event, sizeof(struct epoll_event));
    event.events = EPOLLOUT | EPOLLET;
    event.data.ptr = &(*(io_ctx->sd));
    if (epoll_ctl(io_ctx->sd->epoll, EPOLL_CTL_MOD, io_ctx->sd->link, &event) != 0) {
        LOG(ERROR) << "modify epoll fail! link:" << io_ctx->sd->link << " err:" << errno;
    }
}
void CNetIOLinux::DoAccept()
{
    struct sockaddr_in sa;
    socklen_t len = sizeof(sa);
    net_link link = accept(m_listen, (struct sockaddr*)(&sa), &len);
    if (link == INVALID_NET_LINK) {
        LOG(ERROR) << "new link accpet fail! err:" << errno;
        return;
    }
    if (!SetSocketNonblocking(link)) {
        LOG(ERROR) << "set new link to noblocking fail! err:" << errno;
        return;
    }
    if (!AddSocketToEpoll(link, m_recv_epoll[link % m_recv_thread_num])) {
        close(link);
        LOG(ERROR) << "accept link fail!";
        return;
    }
    return;
}
void CNetIOLinux::SendDataThread(CNetIOLinux* pThis)
{
    LOG(INFO) << "create send thread successful! id:" << std::this_thread::get_id();
    while (true) {
        std::unique_lock<std::mutex> lock(pThis->m_send_mx);
        while (pThis->m_send_io.empty()) {
            pThis->m_send_cv.wait(lock);
        }
        LOG_IF(WARNING, pThis->m_send_io.size() > WARNING_SEND_IO_NUM) << "send data->size:" << pThis->m_send_io.size();
        auto socket_ctx = pThis->m_send_io.back()->self;
        pThis->m_send_io.pop_back();
        lock.unlock();
        pThis->m_send_cv.notify_all();
        pThis->DoWrite(socket_ctx);
    }
}
#endif