aux_source_directory(. SRC_LIST)
project(svc_launch)

add_executable(${PROJECT_NAME} main.cpp)

link_libraries(${PROJECT_SOURCE_DIR}/third-party/lib)
target_link_libraries(${PROJECT_NAME} svc svc_info cfg_parser config glog gflags net_io rpc net_msg role yaml-cpp libgo coroutine cfg_parser pthread dl)

SET(EXECUTABLE_OUTPUT_PATH ${INSTALL_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${INSTALL_DIR}/lib)