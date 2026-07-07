NwShip — 潜艇探索助手
=====================

简介
----
NwShip 是一个基于 Qt 的桌面应用，用于计算并展示潜艇探索航线的建议。程序使用 CMake 构建，支持 Debug/Release 模式。

要求
----
- Qt 6（或 Qt 5）Widgets
- CMake 3.16+
- MinGW-w64（Windows）或等价的编译工具链

快速构建
--------
在项目根目录运行：

```bash
cmake -S . -B build/Desktop_Qt_6_11_1_MinGW_64_bit_Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build/Desktop_Qt_6_11_1_MinGW_64_bit_Debug --parallel
```

发布（Release）构建：

```bash
cmake -S . -B build/Desktop_Qt_6_11_1_MinGW_64_bit_Release -DCMAKE_BUILD_TYPE=Release
cmake --build build/Desktop_Qt_6_11_1_MinGW_64_bit_Release --config Release --parallel
```

运行
----
构建后可执行文件位于构建目录，如：

- build/Desktop_Qt_6_11_1_MinGW_64_bit_Debug/NwShip.exe
- build/Desktop_Qt_6_11_1_MinGW_64_bit_Release/NwShip.exe


许可与贡献
----------
感谢 xivapi/ffxiv-datamining 项目的数据开源

---
