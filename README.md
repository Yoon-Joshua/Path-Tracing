# 基于蒙特卡洛方法的路径追踪渲染器

![](images/demo.png)

## Build

克隆本仓库，执行根目录的 CmakeLists.txt 进行构建。构建成功后会在根目录生成可执行文件 main.exe 。在根目录直接运行。

本仓库开发环境：Windows、GCC、C++17

## Features

- 基于 Surface Area Heuristic 的 BVH 构建
- 针对光源的重要性采样
- 针对漫反射材质的余弦重要性采样
- 基于联合双边滤波的降噪

## Todo

- GPU并行的 BVH 构建
- 边缘反走样
- 滤波优化