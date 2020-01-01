#!/bin/bash

function is_project_dir()
{
    return `ls -A $1|grep -E ".\.h|.\.c|.\.cpp|.\.cxx"|wc -w`
}

function is_no_src_project()
{
    return `ls -A $1|grep -E ".\.c|.\.cpp|.\.cxx"|wc -w`
}

function create_not_project_cmake_file()
{
    cmake_file_path=$1"/"CMakeLists.txt
    rm -f $cmake_file_path
    for sub_dir in `ls -A $1`;do
        if [ -d $1"/"$sub_dir ];then
            if ! is_no_src_project $1"/"$sub_dir;then
                echo "add_subdirectory("$sub_dir")" >> $cmake_file_path
            fi
        fi
    done
}

function create_project_cmake_file()
{
    if is_no_src_project $1;then
        return
    fi
    cmake_file_path=$1"/"CMakeLists.txt
    rm -f $cmake_file_path
    echo "aux_source_directory(. SRC_LIST)" >> $cmake_file_path
    echo "project($2)" >> $cmake_file_path
    echo "set(CMAKE_DEBUG_POSTFIX _d)" >> $cmake_file_path
    echo "add_library(\${PROJECT_NAME} STATIC \${SRC_LIST})" >> $cmake_file_path
    echo "set(EXECUTABLE_OUTPUT_PATH \${INSTALL_DIR}/bin)" >> $cmake_file_path
    echo "set(LIBRARY_OUTPUT_PATH \${INSTALL_DIR}/lib)" >> $cmake_file_path
}

function create_cmake_file()
{
    dir=$1
    if is_project_dir $1
    then
        create_not_project_cmake_file $1
    else
        create_project_cmake_file $1 $2
    fi
}

function is_main_project()
{
    if [ $1x == "svc_launch"x ];then
        return 0
    elif [ $1x == "test"x ];then
        return 0
    fi
    return 1
}

function generote_cmake_files()
{
    for file in `ls $1`       #注意此处这是两个反引号，表示运行系统命令
    do
        if [ -d $1"/"$file ]  #注意此处之间一定要加上空格，否则会报错
        then
            dir_path="$1/${file}"
            if ! is_main_project $file;then
                create_cmake_file $dir_path $file
            fi
            generote_cmake_files $1"/"$file
        fi
    done
}

function build_it()
{
    if [ ! -d "build_temp" ];then
        mkdir build_temp
    fi
    cd build_temp
    cmake .. -DCMAKE_BUILD_TYPE=Debug
    # make clean
    make -j
    cd ..
}

function install_it()
{
    if [ ! -d "build_temp/output/bin" ];then
        echo "not find dir!"
        return
    fi
    if [ ! -d "build/bin/" ];then
        mkdir -p build/bin/
    fi

    cp -r build_temp/output/bin/* build/bin/
}

function main()
{
    # generote_cmake_files src
    build_it
    install_it
}

main