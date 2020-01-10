#include "gs_co.h"

#include <thread>

static co::Scheduler* __co_instance__ = nullptr;



co::Scheduler* co_create()
{
    if (__co_instance__ == nullptr) {
        __co_instance__ = co::Scheduler::Create();
        
        // 启动4个线程执行新创建的调度器
        std::thread co_thread([] {
            __co_instance__->Start(1, std::thread::hardware_concurrency() * 2);
        });
        co_thread.detach();
    }
    return __co_instance__;
}

bool is_in_co()
{
    return ::co::GetCurrentCoroID() > 0;
}