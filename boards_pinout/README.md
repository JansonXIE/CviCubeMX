# 引脚配置代码生成工具使用指南

## 概述

`generate_pins.py` 是一个用于从 Excel 引脚定义文件自动生成 C++ 代码的 Python 脚本。该工具可以解析芯片引脚配置 Excel 表格，自动生成用于 Qt 应用程序的引脚功能映射代码。

## 功能特性

- ✅ 从 Excel 文件读取引脚定义数据
- ✅ 自动解析引脚功能和默认配置
- ✅ 支持引脚名称清理（处理 `___` 分隔符）
- ✅ 功能名称重映射（处理特殊命名规则）
- ✅ 自动排序引脚（按字母+数字自然排序）
- ✅ 生成两个 C++ 代码文件：
  - `initializePinNameMappings.cpp` - 引脚编号到名称的映射
  - `initializePinFunctions.cpp` - 引脚功能定义和配置

## 环境要求

### Python 版本
- Python 3.6 或更高版本

### 依赖库
安装所需的 Python 库：

```bash
pip install pandas openpyxl
```

或使用 requirements.txt（如果提供）：

```bash
pip install -r requirements.txt
```

## 使用方法

### 基本用法

```bash
python generate_pins.py <Excel文件路径>
```

### 示例

```bash
# 使用相对路径
python generate_pins.py ./cv184x/CV184xC__PINOUT_V13_CN.xlsx 

# 使用默认文件（pins.xlsx）
python generate_pins.py
```

## Excel 文件要求

### 表单结构

脚本默认读取 **第 4 个工作表**（索引为 3），可在脚本配置中修改 `SHEET_NAME` 参数。

### 必需的列

Excel 文件必须包含以下列（列名需完全匹配，包括大小写和空格）：

| 列名 | 说明 | 示例 |
|------|------|------|
| `Pin Num` | 引脚编号 | A1, A2, B3 等 |
| `Pin Name` | 引脚名称 | PAD_ETH_RXM, PAD_GPIO0 等 |
| `Description` | 功能描述 | 包含多行功能定义的文本 |

### Description 列格式

Description 列应包含如下格式的功能定义：

```
function select
0 : XGPIOA_0 (default)
1 : UART0_TX
2 : SPI0_MOSI
3 : I2C0_SDA
```

- 每行格式：`索引 : 功能名称`
- 带 `(default)` 的功能将被标记为默认功能
- 支持方括号 `[]` 自动转换为下划线 `_`

## 输出文件

### 1. initializePinNameMappings.cpp

生成引脚编号到引脚名称的映射代码：

```cpp
void MainWindow::initializePinNameMappings()
{
    // BGA引脚映射（字母数字格式）- 由脚本自动生成并排序
    m_pinNameMappings["A2"] = "PAD_MIPI_TXM4";
    m_pinNameMappings["A3"] = "PAD_MIPI_TXP4";
    // ...
}
```

### 2. initializePinFunctions.cpp

生成完整的 PinFunction 类实现，包括：

```cpp
#include "pinfunction.h"

PinFunction::PinFunction()
{
    initializePinFunctions();
}

QStringList PinFunction::getSupportedFunctions(const QString& pinName) const
{
    return m_pinFunctions.value(pinName, QStringList() << "GPIO");
}

// ... 其他成员函数

void PinFunction::initializePinFunctions()
{
    // A2 引脚功能定义（Pin name: PAD_MIPI_TXM4）
    m_pinFunctions["PAD_MIPI_TXM4"] = QStringList() << "XGPIOA_0" << "UART0_TX";
    m_defaultFunctions["PAD_MIPI_TXM4"] = "XGPIOA_0";
    // ...
}
```

## 配置选项

可以在脚本顶部修改以下配置：

```python
# Excel 配置
SHEET_NAME = 3              # 工作表索引（0 开始）
HEADER_ROW_INDEX = 0        # 标题行索引

# 列名配置
COL_PIN_NUM = "Pin Num"
COL_PIN_NAME = "Pin Name"
COL_FUNCTIONS_LIST = "Description"

# 输出文件名
OUTPUT_PIN_FUNCTIONS_CPP = 'initializePinFunctions.cpp'
OUTPUT_PIN_MAPPINGS_CPP = 'initializePinNameMappings.cpp'
```

### 功能名称重映射

脚本支持自定义功能名称重映射规则（`FUNCTION_NAME_REMAP`），用于处理特殊的命名冲突：

```python
FUNCTION_NAME_REMAP = {
    "CR_4WTMS": "CV_2WTMS_CR_4WTMS",
    "CR_4WTCK": "CV_2WTCK_CR_4WTCK",
    # ... 添加更多映射规则
}
```

## 特殊处理

### 引脚名称清理

脚本会自动清理包含 `___` 的引脚名称：

- 输入：`PAD_ETH_RXM___EPHY_TXP`
- 输出：`PAD_ETH_RXM`

### 自然排序

引脚会按照字母+数字进行自然排序：

```
A2, A3, A10, A11  （而不是 A10, A11, A2, A3）
B1, B2, B10, B11
```

### 备用引脚支持

生成的代码包含对未定义引脚的备用支持：

- **QFN 引脚**：1-88 的数字编号
- **BGA 引脚**：A-R 行（跳过 I），1-15 列（排除四个角）

## 错误处理

### 常见错误及解决方法

1. **文件未找到**
   ```
   错误: 找不到文件 'xxx.xlsx'。
   ```
   解决：检查文件路径是否正确，确保文件存在。

2. **工作表不存在**
   ```
   错误: 找不到工作表。请确保索引 '3' (即第四个工作表) 存在。
   ```
   解决：检查 Excel 文件是否有足够的工作表，或修改 `SHEET_NAME` 配置。

3. **缺少必要的列**
   ```
   错误: Excel 文件缺少必要的列。
   需要: {'Pin Num', 'Pin Name', 'Description'}
   ```
   解决：检查 Excel 文件的列名是否与配置完全匹配（区分大小写）。

4. **空行警告**
   ```
   警告: 跳过第 X 行，Pin Num 或 Pin Name 为空。
   ```
   这是正常提示，脚本会自动跳过空行。

## 工作流程

```mermaid
graph LR
    A[Excel 文件] --> B[读取数据]
    B --> C[解析引脚信息]
    C --> D[应用重映射规则]
    D --> E[排序引脚]
    E --> F[生成 C++ 代码]
    F --> G[initializePinNameMappings.cpp]
    F --> H[initializePinFunctions.cpp]
```

## 示例输出

运行脚本后的控制台输出：

```
使用命令行指定的文件: ./cv184x/CV184xH__PINOUT_V13_CN.xlsx
成功从 './cv184x/CV184xH__PINOUT_V13_CN.xlsx' 的第 4 个工作表加载 150 行数据。
    -> 提示: Pin Name 'PAD_ETH_RXM___EPHY_TXP' 已清理为 'PAD_ETH_RXM'.
成功解析 148 个引脚。
正在生成 initializePinNameMappings.cpp...
已根据 Pin Num (Ax, Bx, ...) 排序。
成功写入 initializePinNameMappings.cpp。
--------------------
正在生成 initializePinFunctions.cpp...
已根据 Pin Num (Ax, Bx, ...) 排序。
成功写入 initializePinFunctions.cpp。
--------------------
脚本执行完毕。
```

## 集成到项目

生成的 C++ 文件可直接集成到基于 Qt 的项目中：

1. 将生成的两个 `.cpp` 文件复制到项目源码目录中的initializePinNameMappings和initializePinFunctions函数

## 维护和扩展

### 添加新的功能重映射

在 `FUNCTION_NAME_REMAP` 字典中添加新的映射规则：

```python
FUNCTION_NAME_REMAP = {
    "旧名称": "新名称",
    # 添加更多...
}
```

### 修改输出格式

编辑 `generate_pin_mappings_cpp()` 和 `generate_pin_functions_cpp()` 函数来自定义输出格式。

### 支持其他 Excel 格式

修改 `load_pin_data()` 函数中的列名和解析逻辑。

## 技术支持

如遇到问题或需要功能扩展，请联系开发团队或提交 Issue。

## 版本历史

- **v1.0** - 初始版本，支持基本的引脚代码生成
- **v1.1** - 添加命令行参数支持
- **v1.2** - 添加引脚名称清理和功能重映射

---

**最后更新**: 2025年11月17日
