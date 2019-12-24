#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.
#include <core/platform/os_macro.h>

#ifdef OS_WIN

#include "net_io_interface.h"

struct PerIOCtx;
struct PerSocketCtx;
class CNetIOWin32 : public INetIO
{
    using socket_ctx_iter = safe_recycle_list<PerSocketCtx>::iterator;
    using io_ctx_iter = safe_recycle_list<PerIOCtx>::iterator;
public:
    CNetIOWin32();
    virtual ~CNetIOWin32();
public:
    int SetListenPort(const uint16_t& port) override;
    int SetWorkThreadNum(const uint32_t& num) override;
    int Initiate() override;
    int Run() override;
    net_link AddConnect(const std::string& ip, const uint16_t& port) override;
    bool RecvData(net_link& socket, net_io_buf& recv_buf, const int32_t& time_out) override;
    int SendData(const net_link& socket, const char* data, const size_t& data_size) override;
    void CleanUp() override;
    static DWORD WINAPI RecvDataThread(LPVOID p_net_cs);
    static DWORD WINAPI SendDataThread(LPVOID p_net_cs);
private:
    // ���������߳�
    int CreateThread();

    //  ����һ��socket
    net_link CreateOneNetLink();
    // ���������socket
    int CreateListenNetLink();
    // ����һ���ȴ��ͻ������ӵ�socket
    int CreateClientNetLink();
    // ���������¼�
    void DoAccept(PerIOCtx* p_io_ctx, DWORD io_size);
    // �������¼�
    void DoSend(PerIOCtx* p_io_ctx, DWORD io_size);
    // �����ȡ�¼�
    void DoRead(PerIOCtx* p_io_ctx, DWORD io_size);

    /* ����һ����ȡ�¼� */
    bool PostRead(socket_ctx_iter& socket_ctx);

    /* ����һ��д���¼�
     * offset:����ڷ������ݵĻ�ַƫ��
     */
    bool PostWrite(io_ctx_iter& io_ctx, const size_t& offset = 0);

    /* �ر�һ��client���� */
    void CloseLink(net_link link);
private:
    // ��ɶ˿�
    HANDLE m_iocp;

    // �����˿�
    uint16_t m_port;

    // �����߳�����
    uint32_t m_work_thread_num;

    // �����߳�����
    uint32_t m_recv_work_thread_num;

    // �����߳�
    HANDLE m_work_threads[NET_MAX_WORK_THREAD_NUM];

    // �ٽ���
    CRITICAL_SECTION m_critical_section;

    // ����socket
    net_link m_listen;

    // �������ӵ�socket
    safe_recycle_list<PerSocketCtx> m_socket_ctx;

    // ���е�IO
    safe_recycle_list<PerIOCtx> m_io_ctx;

    // �ȴ�������û�����
    safe_list<io_ctx_iter> m_wait_detail;

    // �ȴ����͵��û�����
    safe_list<io_ctx_iter> m_wait_send;

    // �������
    bool m_finish;

    // �ͻ�������
    safe_map<net_link, socket_ctx_iter, false> m_active_link;

    // ��д����
    WSAEVENT m_read_event;
    WSAEVENT m_write_event;
};

#endif
