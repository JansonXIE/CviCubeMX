# 外设配置功能更新说明

## 功能说明

### 单外设配置保存
当用户在外设配置对话框中点击"应用"按钮时：

1. **之前的行为**：
   - 会遍历所有外设并更新它们的配置
   - 可能会意外修改其他外设的设置

2. **现在的行为**：
   - 只更新当前正在配置的外设节点
   - 其他外设的配置保持不变
   - 提供更精确的配置控制

### 配置示例

假设用户正在配置 `i2c0` 外设：

**原始节点：**
```dts
i2c0: i2c@04000000 {
    compatible = "snps,designware-i2c";
    clocks = <&clk CV184X_CLK_I2C>;
    reg = <0x0 0x04000000 0x0 0x1000>;
    clock-frequency = <400000>;
    #size-cells = <0x0>;
    #address-cells = <0x1>;
    resets = <&rst RST_I2C0>;
    reset-names = "i2c0";
};
```

**用户配置后（状态设为 disabled）：**
```dts
i2c0: i2c@04000000 {
    compatible = "snps,designware-i2c";
    clocks = <&clk CV184X_CLK_I2C>;
    reg = <0x0 0x04000000 0x0 0x1000>;
    clock-frequency = <400000>;
    #size-cells = <0x0>;
    #address-cells = <0x1>;
    resets = <&rst RST_I2C0>;
    reset-names = "i2c0";
    status = "disabled";
};
```

**重要说明：**
- 只有 `i2c0` 节点被修改
- 其他外设节点（如 `i2c1`, `spi0`, `uart0` 等）保持原状
- 制表符格式正确（`\t\tstatus` 和 `\t\tclocks`）

## 使用方法

1. 选择芯片并开始项目
2. 在左侧配置面板中选择外设（如 I2C）
3. 从菜单中选择具体外设实例（如 I2C0）
4. 在配置对话框中：
   - 选择要配置的外设
   - 设置状态（okay/disabled）
   - 配置时钟（如果支持）
5. 点击"应用"按钮
6. 系统会显示："外设 [名称] 配置已保存到设备树文件！"

## 技术实现

### 关键方法

```cpp
// 保存单个外设配置
bool DtsConfig::savePeripheralConfig(const QString &peripheral)

// 更新单个外设内容  
void DtsConfig::updateSinglePeripheralContent(const QString &peripheral)
```

### 调用流程

1. 用户点击"应用" → `PeripheralConfigDialog::onApplyClicked()`
2. 保存当前配置 → `savePeripheralConfig()`  
3. 调用单外设保存 → `DtsConfig::savePeripheralConfig(m_currentPeripheral)`
4. 更新文件内容 → `updateSinglePeripheralContent(peripheral)`
5. 写入文件 → `QFile::open()` + `QTextStream::operator<<()`

这样的设计确保了配置的精确性和安全性，避免了意外修改其他外设配置的风险。
