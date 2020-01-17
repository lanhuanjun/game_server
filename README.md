# game_server

简单的游戏服务器，可以分布式部署。 有许多bug，仅用作个人学习。

## 1. 工程目录结构

```txt
├─assist 辅助代码生成脚本
│  └─annotation  解析代码中的注解
│      └─rmi 解析rmi注解
├─build 工程
├─cfg 配置
│  └─net_msg
├─src
│  ├─config 模块注册
│  ├─core 核心实现
│  ├─logic 业务
│  │  └─role 管理角色信息业务模块
│  ├─svc_launch 程序入口
│  ├─test 模块测试
│  └─third-party 第三方模块包装
├─third-party 第三方库
└─win32
    └─projects
```

## 2. 构建

windows:
使用vs打开: win32\svc_launch.sln
linux:
bash build_linux.sh

## 3. 添加一个功能模块

参照业务模块role

1. 复制win32\project\role.vcxproj win32\project\[新模块名].vcxproj；
2. 复制win32\project\role.vcxproj.filters win32\project\[新模块名].vcxproj.filters；
3. 更改[新模块名].vcxproj和[新模块名].vcxproj.filters里面ItemGroup节点，使文件指向新的工程文件；
4. 使用vs添加新的模块工程文件[新模块名].vcxproj；
5. 在src\config\manager_def.h的 “MANAGER”宏添加新的模块名。
6. 如果想在进程中启用该模块，则在cfg\svc_global_cfg.yaml中服务器类型的mngs添加新的模块名称。

## 3. 使用远程调用

参考role_manager.cpp

```c++
Rmi<IRoleManager> svc_lobby(__ANY_LOBBY__);
svc_lobby.RmiTest_Add(a, b);
```

## 4. 使用协程

参考role_manager.cpp

```c++
START_TASK(&CRoleManager::TestCall, this);
```
