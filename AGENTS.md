这个项目本来是C项目，现在要改成C++项目。

类名用snake case，比如 class my_type

private成员变量加m_前缀，比如：

    class my_type {
    private:
        int m_num;
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