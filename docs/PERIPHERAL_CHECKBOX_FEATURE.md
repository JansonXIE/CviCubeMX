# 外设复选框功能说明

## 功能概述
已成功为CviCubeMX工具的配置面板添加了外设复选框功能。每个外设子项（PWM、I2C、SPI、UART、GPIO、ADC）左侧现在都有一个复选框，用户可以点击来启用或禁用相应的外设。

## 功能特性

### 1. 复选框状态
- **默认状态**: 根据defconfig文件中的CONFIG配置项自动设置
- **动态加载**: 程序启动时自动读取当前芯片对应的defconfig文件
- **实时保存**: 用户更改复选框状态时，立即更新defconfig文件

### 2. 支持的外设及对应CONFIG项

#### PWM
- CONFIG_PWM=y
- CONFIG_PWM_SYSFS=y  
- CONFIG_CVI_PWM=y

#### I2C
- CONFIG_I2C=y
- CONFIG_I2C_SMBUS=y
- CONFIG_I2C_CHARDEV=y
- CONFIG_I2C_DESIGNWARE_PLATFORM=y

#### SPI
- CONFIG_SPI=y

#### UART
- CONFIG_SERIAL_8250=y
- CONFIG_SERIAL_8250_CONSOLE=y
- CONFIG_SERIAL_8250_DW=y

#### GPIO
- CONFIG_GPIOLIB=y
- CONFIG_GPIO_SYSFS=y
- CONFIG_GPIO_DWAPB=y

#### ADC
- CONFIG_IIO=y
- CONFIG_IIO_BUFFER=y
- CONFIG_IIO_TRIGGER=y

### 3. defconfig文件路径
根据选择的芯片类型，自动选择对应的defconfig文件：
- 路径格式: `boards/cv184x/{chiptype}/linux/cvitek_{chiptype}_defconfig`
- 例如cv1842hp芯片: `boards/cv184x/cv1842hp_wevb_0014a_emmc/linux/cvitek_cv1842hp_wevb_0014a_emmc_defconfig`

### 4. 状态逻辑
- **启用外设**: 所有相关CONFIG项都设置为 `CONFIG_XXX=y`
- **禁用外设**: 所有相关CONFIG项都设置为 `# CONFIG_XXX is not set`
- **状态检查**: 只有当所有相关CONFIG项都启用时，外设才显示为勾选状态

## 使用方法

1. **启动程序**: 程序会自动读取当前芯片的defconfig文件，加载外设的启用/禁用状态
2. **查看状态**: 配置面板中每个外设前的复选框会显示当前状态（勾选=启用，未勾选=禁用）
3. **更改状态**: 点击复选框可以切换外设的启用/禁用状态
4. **自动保存**: 状态更改会立即保存到对应的defconfig文件中
5. **错误处理**: 如果无法写入defconfig文件，会显示警告并恢复原状态

## 技术实现

### 新增的主要方法
- `loadPeripheralStates()`: 从defconfig文件加载外设状态
- `savePeripheralStates()`: 保存外设状态到defconfig文件
- `getDefconfigPath()`: 获取当前芯片对应的defconfig文件路径
- `getPeripheralConfigs()`: 获取每个外设对应的CONFIG配置项列表
- `onPeripheralCheckBoxChanged()`: 处理复选框状态变化的槽函数

### 数据结构
- `m_peripheralStates`: QMap<QString, bool> 存储外设的启用/禁用状态

### UI改进
- 外设子项现在使用QCheckBox作为widget，替代了原来的纯文本显示
- 保持了原有的右键菜单功能（PWM0-3、I2C0-2等选项）
- 复选框文本显示外设名称，保持UI的一致性

## 注意事项

1. **文件权限**: 确保程序对defconfig文件具有读写权限
2. **备份建议**: 修改defconfig文件前建议备份原始文件
3. **依赖关系**: 某些CONFIG项可能存在依赖关系，禁用时需要注意
4. **芯片兼容**: 不同芯片型号的CONFIG项可能略有差异

## 测试状态
- ✅ 代码编译成功
- ✅ defconfig文件解析正确
- ✅ CONFIG项映射正确（PWM、I2C、SPI、UART、GPIO、ADC）
- ✅ 错误处理机制完善
