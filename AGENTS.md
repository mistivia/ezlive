这个项目本来是C项目，现在要改成C++项目。

类名用snake case，比如 class my_type

private成员变量加m_前缀，比如：

    class my_type {
    private:
        int m_num;
    };

但是struct和public成员不用加：

    struct struc {
        int pub_member;
    };

    class my_type {
    public:
        int pub_member;
    };

函数的大括号另起一行，其他大括号照旧：

    void my_func()
    {
        if (ok) {

        }
        while (!quit) {

        }
    }

namespace的右大括号要加注释：

    namespace ns {
        ...
    } //namespace ns

缩进4个空格

头文件guard用 #pragma once

这个项目里面的class一律放在namespace ezlive中

构造器一律用explicit

测试全部在 tests/test_*.cc

测试断言只使用 <cassert>

用make test运行测试