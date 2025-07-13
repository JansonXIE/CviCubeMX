# CviCubeMX 快速开始指南

## 环境准备

### 1. 安装Qt开发环境
- 下载并安装Qt 6.x版本
- 确保包含Qt Creator和CMake工具

### 2. 安装编译器
- **Windows**: 安装MinGW或Visual Studio
- **Linux**: 安装gcc/g++编译器

## 构建和运行

### 方法1：使用构建脚本（推荐）

**Windows用户：**
```cmd
双击运行 build.bat
```

**Linux用户：**
```bash
chmod +x build.sh
./build.sh
```

### 方法2：手动构建

1. 创建构建目录
```bash
mkdir build
cd build
```

2. 配置CMake
```bash
cmake ..
```

3. 编译项目
```bash
cmake --build . --config Release
```

4. 运行程序
```bash
./bin/CviCubeMX      # Linux
.\bin\CviCubeMX.exe  # Windows
```

### 方法3：使用Qt Creator

1. 打开Qt Creator
2. 选择"打开项目"
3. 选择`CMakeLists.txt`文件
4. 配置工具链和构建套件
5. 点击"构建"按钮
6. 运行程序

## 使用步骤

### 1. 启动应用
应用启动后会显示主界面，包含：
- 芯片选型下拉框
- 开始项目按钮
- 生成代码按钮

### 2. 选择芯片型号
在下拉框中选择以下型号之一：
- cv1801c (QFN封装)
- cv1801h (BGA封装)
- cv1811c (QFN封装)
- cv1811h (BGA封装)

### 3. 开始项目
点击"开始项目"按钮，界面会显示：
- **QFN封装**: 方形引脚按钮分布在芯片外围
- **BGA封装**: 圆形引脚按钮分布在芯片内部

### 4. 配置引脚功能
- 点击任意引脚按钮
- 从弹出菜单中选择功能：
  - GPIO (默认)
  - ADC
  - I2C
  - UART
  - SPI
  - PWM
  - Timer

### 5. 生成代码
- 点击"生成代码"按钮
- 选择保存位置
- 生成的`cvi_board_init.c`文件包含完整配置

## 故障排除

### 常见问题

1. **CMake配置失败**
   - 检查Qt是否正确安装
   - 确认CMake版本≥3.16

2. **编译错误**
   - 确认编译器支持C++17
   - 检查Qt版本是否为6.x

3. **运行时错误**
   - 确认Qt运行库在系统PATH中
   - 检查是否缺少必要的DLL文件

### 获取帮助

如果遇到问题，请检查：
1. 系统是否满足环境要求
2. 错误信息和日志输出
3. 参考README.md获取更多信息

### 快速修复常见问题

**问题：找不到Qt6Widgets.dll**
```bash
# 运行DLL修复脚本
.\fix_dll.bat

# 或运行完整部署脚本
.\deploy.bat
```

**问题：编译失败**
```bash
# 清理并重新构建
rmdir /s /q build
.\build.bat
```

## 项目文件说明

- `src/`: 源代码文件
- `include/`: 头文件
- `resources/`: 资源文件
- `build/`: 构建输出目录
- `CMakeLists.txt`: CMake构建配置
- `README.md`: 项目说明文档
