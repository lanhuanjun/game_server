#include "binary_stream_test.h"
#include <glog/logging.h>
#include <libgo/libgo.h>
#pragma comment(lib, "ws2_32.lib")
void fun_c()
{
    LOG(INFO) << co::GetCurrentCoroID();
}
void fun_b()
{
    LOG(INFO) << co::GetCurrentCoroID();
    fun_c();
}
#define RMI /\/\/["@rmi"]
void fun_a()
{
    LOG(INFO) << co::GetCurrentCoroID();
    fun_b();
}

int main(int argc, char* argv[])
{
    //binary_stream_test();
    go[]{
        fun_a();
    };

    co_sched.Start();
    co_sleep(100);
    return 0;
}
