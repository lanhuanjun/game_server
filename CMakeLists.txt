cmake_minimum_required(VERSION 3.2)
project(svc_launch)

IF (CMAKE_BUILD_TYPE MATCHES "Release" 
        OR CMAKE_BUILD_TYPE MATCHES "None")
    SET(CMAKE_CXX_FLAGS_RELEASE "${ENV{CXXFLAGS} -O3 -Wall")
ELSE (CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
ENDIF ()

IF (WIN32)
    SET(OS win)
ELSEIF (APPLE)
    SET(OS mac)
ELSE ()
    SET(OS unix)
ENDIF ()

set(INSTALL_DIR ${PROJECT_BINARY_DIR}/output)

# 设置支持的C++版本
set(CMAKE_CXX_STANDARD 17)

# 添加头文件搜索目录
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/third-party/${OS}/include
    /usr/local/include
    /usr/include
)

# 添加链接子目录
link_directories(
    ${PROJECT_SOURCE_DIR}/third-party/${OS}/lib
    /usr/local/lib
    /usr/lib
)

# 添加子目录
add_subdirectory(src)