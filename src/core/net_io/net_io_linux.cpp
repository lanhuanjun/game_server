#include "net_io_linux.h"
#ifdef OS_LINUX
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <glog/logging.h>
#include <core/tools/gs_assert.h>


const uint32_t MAX_EPOLL_EVENT_NUM = 10;

struct PerSocketCtx
{
    net_link link;

    safe_recycle_list<PerSocketCtx>::iterator self;

    PerSocketCtx()
        : link(INVALID_NET_LINK)
    {

    }
    SAFE_RECYCLE_LIST_NODE_DECLARE(PerSocketCtx)
};

struct PerIOCtx
{
    char data[NET_MSG_DEFAULT_BUFFER_SIZE];
    size_t data_len;
    
    PerSocketCtx* p_sd;
    safe_recycle_list<PerIOCtx>::iterator self;

    PerIOCtx()
        : data_len(0)
    {

    }
    SAFE_RECYCLE_LIST_NODE_DECLARE(PerIOCtx)
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
    if (!CreateEpollHandle()) {
        CleanUp();
        return NET_CREATE_EPOLL_FAIL;
    }
    if (!AddSocketToEpoll(m_listen)) {
        LOG(FATAL) << "add listen socket to epoll fail!";
        return NET_FAIL;
    }
    m_recv_data = std::thread(RecvDataThread, this);
    m_send_data = std::thread(SendDataThread, this);
    m_recv_data.detach();
    m_send_data.detach();
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

    if (!AddSocketToEpoll(link)) {
        LOG(ERROR) << "add to epoll fail!";
        close(link);
        return INVALID_NET_LINK;
    }
    LOG(INFO) << "add link ok! link:" << link;
    return link;
}

bool CNetIOLinux::RecvData(net_link& socket, net_io_buf& recv_buf, const int32_t& time_out /*= -1*/)
{
    
    if (time_out == 0) {
        if (m_recv_io.empty()) {
            return false;
        }
    } else {
        if (m_recv_io.empty()) {
            std::unique_lock<std::mutex> lock(m_recv_mx);
            if (time_out < 0) {
                m_recv_cv.wait(lock);
            } else {
                m_recv_cv.wait_for(lock, std::chrono::milliseconds(time_out));
            }
            lock.unlock();
            if (m_recv_io.empty()) {
                return false;
            }
        }
    }
    auto pIO = m_recv_io.back();
    socket = pIO->p_sd->link;
    memcpy(recv_buf.data, pIO->data, pIO->data_len);
    recv_buf.used = pIO->data_len;
    m_recv_io.pop_back();
    return true;
}

int CNetIOLinux::SendData(const net_link& socket, const char* data, const size_t& data_size)
{
    if (data == nullptr || data_size == 0) {
        return NET_FAIL;
    }
    auto f_link = m_link_map.find(socket);
    if (!f_link.second) {
        LOG(ERROR) << "link is broken!";
        return NET_LINK_BROKEN;
    }
    PerSocketCtx* pSd = f_link.first->second;
    size_t data_len = data_size;
    while (data_len > 0) {
        auto pIO = m_io_ctx.get_free_item();
        pIO->self = pIO;
        pIO->p_sd = pSd;
        if (data_len >= NET_MSG_DEFAULT_BUFFER_SIZE) {
            memcpy(pIO->data, data, NET_MSG_DEFAULT_BUFFER_SIZE);
            pIO->data_len = NET_MSG_DEFAULT_BUFFER_SIZE;
            data_len -= NET_MSG_DEFAULT_BUFFER_SIZE;
        } else {
            memcpy(pIO->data, data, data_len);
            pIO->data_len = data_len;
            data_len = 0;
        }
        m_send_io.push_front(pIO.ptr());
    }
    m_send_cv.notify_all();
    return NET_OK;
}

void CNetIOLinux::CleanUp()
{
    if (m_listen != INVALID_NET_LINK) {
        close(m_listen);
        m_listen = INVALID_NET_LINK;
    }
}

bool CNetIOLinux::CreateEpollHandle()
{
    m_epoll = epoll_create1(0);
    LOG_IF(FATAL, m_epoll == -1) << "can't create epoll handle. err:" << errno;
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

bool CNetIOLinux::AddSocketToEpoll(net_link sd)
{
    auto socket_ctx = m_socket_ctx.get_free_item();
    socket_ctx->self = socket_ctx;
    socket_ctx->link = sd;

    struct epoll_event event;
    bzero(&event, sizeof(struct epoll_event));
    event.data.ptr = socket_ctx.ptr();
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(m_epoll, EPOLL_CTL_ADD, sd, &event) == -1) {
        LOG(ERROR) << "add socket to epoll fail! err:" << errno;
        m_socket_ctx.recycle(socket_ctx);
        return false;
    }
    m_link_map.insert({sd, socket_ctx.ptr()});
    LOG(INFO) << "new client link. sd:" << link << " size:" << m_link_map.size();
    return true;
}
void CNetIOLinux::RecvDataThread(CNetIOLinux* pThis)
{
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
                pThis->DoError(p_socket);
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

            // 处理读取事件
            if (events[i].events & EPOLLIN) {
                pThis->DoRead(p_socket);
                continue;
            }
            // 处理写入事件
            // 不处理，在另外一个线程处理
        }
    }
    
}
void CNetIOLinux::DoError(PerSocketCtx* pSd)
{
    LOG(ERROR) << "link will close! sd:" << pSd->link;

    struct epoll_event event;
    if (epoll_ctl(m_epoll, EPOLL_CTL_DEL, pSd->link, &event) != 0) {
        LOG(ERROR) << "rm link fail! link:" << pSd->link << " err:" << errno;
    }

    if (pSd->link != INVALID_NET_LINK) {
        close(pSd->link);
    }
    m_link_map.erase(pSd->link);
    m_socket_ctx.recycle(pSd->self);
    
}
void CNetIOLinux::DoRead(PerSocketCtx* pSd)
{
    int cnt = 0;
    while (true) {
        auto io_ctx = m_io_ctx.get_free_item();
        io_ctx->self = io_ctx;
        cnt = read(pSd->link, io_ctx->data, NET_MSG_DEFAULT_BUFFER_SIZE);
        if (cnt < 0) {
            if (errno != EAGAIN) {
                LOG(ERROR) << "read link data fail! link:" << pSd->link << " err:" << errno;
                m_io_ctx.recycle(io_ctx);
                DoError(pSd);
            }
            break;
        } else if (cnt > 0) {
            io_ctx->data_len = cnt;
            io_ctx->p_sd = pSd;
            m_recv_io.push_front(io_ctx.ptr());

            m_recv_cv.notify_all();

            struct epoll_event event;
            bzero(&event, sizeof(struct epoll_event));
            event.events = EPOLLOUT | EPOLLET;
            event.data.ptr = pSd;
            if (epoll_ctl(m_epoll, EPOLL_CTL_MOD, pSd->link, &event) != 0) {
                LOG(ERROR) << "modify epoll fail! link:" << pSd->link << " err:" << errno;
            }
        } else {
            break;
        }
    }
    
}
void CNetIOLinux::DoWrite(PerIOCtx* pIO)
{
    if (!m_link_map.find(pIO->p_sd->link).second) {
        LOG(ERROR) << "link is broken!";
        return;
    }
    int cnt = write(pIO->p_sd->link, pIO->data, pIO->data_len);
    if (cnt < 0) {
        DoError(pIO->p_sd);
        return;
    }
    struct epoll_event event;
    bzero(&event, sizeof(struct epoll_event));
    event.events = EPOLLIN | EPOLLET;
    event.data.ptr = pIO->p_sd;
    if (epoll_ctl(m_epoll, EPOLL_CTL_MOD, pIO->p_sd->link, &event) != 0) {
        LOG(ERROR) << "modify epoll fail! link:" << pIO->p_sd->link << " err:" << errno;
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
    if (!AddSocketToEpoll(link)) {
        close(link);
        LOG(ERROR) << "accept link fail!";
        return;
    }
    return;
}
void CNetIOLinux::SendDataThread(CNetIOLinux* pThis)
{
    while (true) {
        if (pThis->m_send_io.empty()) {
            std::unique_lock<std::mutex> lock(pThis->m_send_mx);
            pThis->m_send_cv.wait(lock);
            lock.unlock();
        }
        LOG_IF(INFO, pThis->m_send_io.length() > 100) << "send data->size:" << pThis->m_send_io.length();
        while (!pThis->m_send_io.empty()) {
            auto back = pThis->m_send_io.back();
            pThis->DoWrite(back);
            pThis->m_send_io.pop_back();
        }
    }
}
#endif