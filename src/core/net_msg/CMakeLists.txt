aux_source_directory(. SRC_LIST)
project(net_msg)
set(CMAKE_DEBUG_POSTFIX _d)
add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} net_io)
set(EXECUTABLE_OUTPUT_PATH ${INSTALL_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${INSTALL_DIR}/lib)
