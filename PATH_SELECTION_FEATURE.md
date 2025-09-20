# 路径选择功能说明

## 功能概述

CviCubeMX 现在支持在启动时选择源代码目录路径，替代了之前在代码中硬编码的文件路径。这样可以让用户灵活地指定包含设备树文件、defconfig 和 cvi_board_init.c 的源代码根目录。

## 功能特性

### 1. 启动时路径选择
- 启动 CviCubeMX.exe 时，会首先显示欢迎对话框
- 用户需要选择包含源代码文件的根目录
- 如果取消选择，程序将退出

### 2. 路径验证
程序会验证所选目录是否包含必要的文件结构：
- `build/` - 编译配置目录

### 3. 运行时路径切换
- 在界面的控制区域新增了"选择源码路径"按钮
- 用户可以随时重新选择源代码路径
- 路径更改后会立即更新相关配置

### 4. 动态文件路径
所有文件路径现在基于用户选择的根目录动态生成：
- defconfig 文件路径：`{根目录}/build/boards/cv184x/{芯片型号}_wevb_0014a_emmc/linux/cvitek_{芯片型号}_wevb_0014a_emmc_defconfig`
- 设备树文件路径：`{根目录}/build/boards/default/dts/cv184x/cv184x_base.dtsi`
- cvi_board_init.c 文件路径：`{根目录}/build/boards/cv184x/{芯片型号}_wevb_0014a_emmc/u-boot/cvi_board_init.c`

## 使用说明

### 首次启动
1. 运行 `CviCubeMX.exe`
2. 在欢迎对话框中点击"确定"
3. 在文件夹选择对话框中选择源代码根目录
4. 程序会验证目录结构的有效性
5. 验证通过后，程序正常启动

### 运行时更改路径
1. 在主界面点击"选择源码路径"按钮
2. 选择新的源代码根目录
3. 程序会重新验证并更新路径配置
4. 界面上的路径显示会自动更新

### 预期的目录结构
```
源代码根目录/
├── build/
    ├── boards/
    │   ├── cv184x/
    │   │   ├── cv1801c_wevb_0014a_emmc/
    │   │   ├── cv1801h_wevb_0014a_emmc/
    │   │   ├── cv1842cp_wevb_0014a_emmc/
    │   │   └── cv1842hp_wevb_0014a_emmc/
    │   │       ├── linux/
    │   │       │   └── cvitek_cv1842hp_wevb_0014a_emmc_defconfig
    │   │       └── u-boot/
    │   │           └── cvi_board_init.c
    │   └── default/
    │       └── dts/
    │           └── cv184x/
    │               └── cv184x_base.dtsi
    └── include/
        └── cvi_board_init.h
```

## 技术实现

### 主要修改的文件
- `src/mainwindow.h` - 添加了路径选择相关的方法和成员变量
- `src/mainwindow.cpp` - 实现了路径选择、验证和UI更新逻辑
- `src/codegenerator.h` - 添加了源代码路径设置方法
- `src/codegenerator.cpp` - 修改为使用动态路径而非硬编码路径

### 新增的方法
- `MainWindow::selectSourcePath()` - 启动时的路径选择
- `MainWindow::showPathSelectionDialog()` - 显示路径选择对话框
- `MainWindow::validateSourcePath()` - 验证所选路径的有效性
- `MainWindow::onSelectSourcePath()` - 运行时路径切换处理
- `CodeGenerator::setSourcePath()` - 设置代码生成器的源代码路径
- `CodeGenerator::getDefaultBoardInitFilePath()` - 动态获取 cvi_board_init.c 文件路径

### 兼容性说明
- 保持了原有的芯片选择和代码生成功能不变
- 新增功能完全向后兼容
- 如果路径验证失败，用户可以重新选择或取消操作

## 注意事项

1. **首次使用**：程序首次启动时必须选择有效的源代码路径才能继续使用
2. **路径验证**：程序会严格验证所选路径的目录结构，确保包含必要的文件
3. **芯片支持**：目前支持的芯片型号包括 cv1801c、cv1801h、cv1811c、cv1811h、cv1842cp、cv1842hp
4. **文件权限**：确保所选择的目录具有读写权限，以便程序能够更新配置文件

## 故障排除

### 路径验证失败
如果出现"路径验证失败"的错误：
1. 确认所选目录包含 `build/` 子目录

### 文件路径问题
如果生成代码时出现文件不存在的错误：
1. 确认所选芯片型号对应的目录存在
2. 检查文件路径权限
3. 尝试重新选择源代码路径

通过这个新功能，CviCubeMX 现在可以适应不同的开发环境和项目结构，提高了工具的灵活性和可用性。