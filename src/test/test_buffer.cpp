#include "test_buffer.h"
#include <core/tools/buffer.h>
#include <cstdio>

void TestCase_1()
{
    slide_buffer buf;
    for (int i = 0; i < 5; ++i) {
        buf.write("hello", 5);
    }
    char t[10];
    for (int i = 0; i < 5; ++i) {
        size_t cnt = buf.read(t, 5);
        if (cnt == 0) {
            break;
        }
        t[cnt] = '\0';
        printf("%s\n", t);
    }
    for (int i = 0; i < 5; ++i) {
        buf.write("hello", 5);
        size_t cnt = buf.read(t, 5);
        if (cnt == 0) {
            break;
        }
        t[cnt] = '\0';
        printf("%s\n", t);
    }
}

void TestCase_2()
{
    slide_buffer buf;
    const char* str = "For example, memcpy might always copy addresses from low to high. If the destination overlaps after the source, this means some addresses will be overwritten before copied. memmove would detect this and copy in the other direction - from high to low - in this case. However, checking this and switching to another (possibly less efficient) algorithm takes time.";
    buf.write(str, strlen(str) - 1);
    char t[10];
    for (int i = 0; i < 100; ++i) {
        size_t cnt = buf.read(t, 5);
        if (cnt == 0) {
            break;
        }
        t[cnt] = '\0';
        printf("%s", t);
    }
    
}

int test_buffer(int argc, char* argv[])
{
    TestCase_2();
    return 0;
}