#include "test_net_io.h"

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <core/net_io/net_io_win32.h>

DEFINE_int32(port, 8000, "port");
DEFINE_int32(thread, 4, "thread");
DEFINE_string(peer_ip, "127.0.0.1", "peer ip");
DEFINE_int32(peer_port, 0, "peer port");


int test_net_io(int argv, char* args[])
{
    google::ParseCommandLineFlags(&argv, &args, true);
    google::InitGoogleLogging(args[0]);
    auto p = new CNetIOWin32();
    p->SetListenPort(FLAGS_port);
    p->SetWorkThreadNum(FLAGS_thread);
    p->Initiate();
    p->Run();
    LOG(INFO) << "port:" << FLAGS_port << " thread:" << FLAGS_thread << " peer:" << FLAGS_peer_ip << "," << FLAGS_peer_port;
    net_link peer_link = INVALID_NET_LINK;
    const char* str = "hello";
    if (FLAGS_peer_port != 0) {
        while (peer_link == INVALID_NET_LINK) {
            peer_link = p->AddConnect(FLAGS_peer_ip.c_str(), FLAGS_peer_port);
            Sleep(1000);
        }
        
        p->SendData(peer_link, str, 5);
    }
    NetMsgBufList msg;
    msg.data.emplace_back();
    msg.data.emplace_back();
    msg.data.emplace_back();
    msg.data.emplace_back();
    msg.data.emplace_back();
    msg.data.emplace_back();
    msg.data.emplace_back();

    while (true) {
        net_link link = INVALID_NET_LINK;
        p->RecvData(link, msg, 500);
        for (auto&& buf : msg.data) {
            if (buf.len == 0) {
                break;
            }
            buf.data[6] = '\0';
            printf("%s\n", buf.data);
            buf.reset();
            p->SendData(link, str, 5);
            // Sleep(1000);
        }
    }

    delete p;
    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();
    return 0;
}