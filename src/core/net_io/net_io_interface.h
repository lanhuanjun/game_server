#pragma once
/*****************************************************************************\
    *  @COPYRIGHT NOTICE
    *  @Copyright (c)2019 - 2030 lanyeo
    *  @file	 : net_io_interface.h
    *  @version  : ver 1.0
    
    *  @author   : lanyeo
    *  @date     : 2019��10��23�� 12:00:00
    *  @brief    : 
\*****************************************************************************/

#include "net_io_info.h"

class INetIO
{
public:

    const static uint32_t MSG_BUF_LEN = NET_MSG_DEFAULT_BUFFER_SIZE;

    virtual ~INetIO() = default;
    /*
     * ���÷����������˿� Ĭ�϶˿�Ϊ DEFAULT_LISTEN_PORT, ��ֵ����С��MIN_TCP_IP_PORT
     */
    virtual int SetListenPort(const uint16_t& port) = 0;


    /**
     * ���ù����߳�����������ܳ���NET_MAX_WORK_THREAD_NUM��
     * (num<2)��ʾʹ��PC�߼�����������,
     * Ĭ��NET_DEFAULT_WORK_THREAD_NUM��
     * ���н����̺߳ͷ����̱߳��������3��1
     * 0���ɹ� ����������
     */
    virtual int SetWorkThreadNum(const uint32_t& num) = 0;


    /**
     * ��ʼ���׽��ֵȹ���
     * 0���ɹ� ������ʧ��
     */
    virtual int Initiate() = 0;


    /**
     * �������绷��
     * 0���ɹ� ������ʧ��
     */
    virtual int Run() = 0;

    /*
     * ���ӵ��ķ��������÷���������Ҫ��Run֮��
     * ����һ��net_link
     */
    virtual net_link AddConnect(const std::string& ip, const uint16_t& port) = 0;

    /**
     * ��ȡ��������, ���̻߳������߳�ִ�У��������ó�ʱʱ�䣬�ο�����time_out
     * time_out ��ʱʱ��,��λ����, С��0��ʾ������ʱ��һֱ��������ִ��
     * return true�����ݣ�falseû������
     */
    virtual bool RecvData(net_link& socket, net_io_buf& recv_buf, const int32_t& time_out = -1) = 0;

    /**
     * �������ݵ�ĳ��socket
     */
    virtual int SendData(const net_link& socket, const char* data, const size_t& data_size) = 0;
    /**
     * �������绷��
     */
    virtual void CleanUp() = 0;
};
