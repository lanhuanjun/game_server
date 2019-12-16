#include <exception>
#include <iostream>
#include <core/svc/svc.h>
#include <glog/logging.h>
#include <Windows.h>

int main(int argc, char* argv[])
{
    try {
        int ret = __svc_run__(argc, argv);
    } catch (const std::exception& e) {
        LOG(ERROR) << e.what() <<GetLastError();
    }
    return 0;
}
