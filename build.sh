#!/bin/bash

echo "=================================="
echo "CviCubeMX Build Script"
echo "=================================="

# 检查是否存在构建目录
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# 进入构建目录
cd build

# 运行CMake配置
echo "Configuring project with CMake..."
cmake ..

if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed!"
    exit 1
fi

# 编译项目
echo "Building project..."
cmake --build . --config Release

if [ $? -ne 0 ]; then
    echo "ERROR: Build failed!"
    exit 1
fi

echo "=================================="
echo "Build completed successfully!"
echo "=================================="
echo "You can find the executable in: build/bin/CviCubeMX"
echo

# 询问是否运行程序
read -p "Do you want to run the application now? (y/n): " run_app
if [[ "$run_app" =~ ^[Yy]$ ]]; then
    if [ -f "bin/CviCubeMX" ]; then
        echo "Running CviCubeMX..."
        ./bin/CviCubeMX &
    else
        echo "ERROR: Executable not found!"
    fi
fi
