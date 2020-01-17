# 为C++注解生成代码

# 1. rmi接口生成

调用方式，执行成功之后会自动在同级目录生成xxx_rmi.h和xxx_rmi.cpp

```shell
python.exe main.py  -r -f xxx_interface.h
```

例：

```c++
class IRoleManager : public IManager
{
public:

    /*
     * rmi test
     */
    Annotation(@RMI) // 添加该注解说明该接口为远程调用接口
    virtual int RmiTest_Add(int a, int b) = 0;
};
```
