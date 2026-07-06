# CviCubeMX - 芯片引脚配置工具

CviCubeMX 是基于 **Tauri v2 + React + TypeScript + Rust** 重构的现代化 CV 系列芯片引脚配置工具，支持芯片选型、引脚功能配置、外设设备树配置、时钟树计算、内存/Flash 分区管理以及 AI 智能辅助功能。

> ⚠️ **注意**：本项目已从 Qt C++ 架构完成重构，当前主线为 `TypeScript_tauri` 分支，基于 Tauri v2 桌面应用框架构建。旧 Qt 版本代码请切换至 `main` 分支查看。

---

## ✨ 功能特性

### 🔧 芯片选型
- 支持 6 款 CV 系列芯片：`cv1801c`、`cv1801h`、`cv1811c`、`cv1811h`、`cv1842cp`、`cv1842hp`
- 自动识别芯片封装类型（QFN/BGA）并渲染对应物理引脚视图

### 📌 引脚配置
- **QFN 封装**（`cv1801c`、`cv1811c`、`cv1842cp`）：方形引脚，四周逆时针排列
  - cv1801c/cv1801h：64 引脚；cv1811c/cv1811h：88 引脚；cv1842cp：88 引脚
- **BGA 封装**（`cv1801h`、`cv1811h`、`cv1842hp`）：圆形引脚，行列网格排布，自动剔除四角物理空焊点
  - cv1842hp：221 有效引脚（15×15 网格剔除四角）
- 右键点击引脚弹出功能选择菜单，按颜色编码区分功能类型：
  - 🔘 灰色：GPIO &emsp; 🔴 红色：ADC &emsp; 🔵 蓝色：I2C &emsp; 🟢 绿色：UART &emsp; 🟠 橙色：SPI &emsp; 🟣 紫色：PWM

### 🌳 外设配置（DTS 设备树）
- 自动解析 `cv184x_base.dtsi` 设备树文件
- 支持配置外设状态（`okay` / `disabled`）、时钟频率（`clock-frequency`）、PWM Cells（`#pwm-cells`）、UART 波特率（`current-speed`）
- SYSDMA 通道重映射（`ch-remap`），修改通道时联动自动更新受影响外设的 `dmas` / `dma-names` / `capability` 属性

### ⏱️ 时钟树计算
- 可视化时钟树，支持调节主 PLL / SubPLL 的倍频器和分频器
- 级联频率自动重算，叶子节点实时更新显示最终频率
- 一键导出超频/普通时钟选项至 `{chip_type}_defconfig`

### 💾 内存与 Flash 分区管理
- 内存区域配置：支持手动增删修改，重叠区域/地址约束硬校验报错
- Flash 分区表管理：容量进度条实时展示，超出额定容量自动红色预警
- 支持导出 JSON 配置和 defconfig 格式

### 🤖 AI 智能辅助（AI Chat）
- 内置 AI Chat 面板，支持流式 SSE 推送和 Markdown 渲染
- 支持自定义 API Key / Base URL / Model 配置

### 🖨️ 代码生成
- 基于 Handlebars 模板生成 `cvi_board_init.c` 初始化源码
- 支持增量更新已有文件：在 `return 0;` 前精准插入 PINMUX 配置块
- 自动生成 ETH / MIPI / Audio 的特殊寄存器初始化序列

---

## 🛠️ 开发环境

| 依赖 | 版本要求 |
|------|----------|
| Node.js | ≥ 20 |
| npm | ≥ 10 |
| Rust | stable (via rustup) |
| Tauri CLI | v2 |
| TypeScript | ^5.4 |
| React | ^18.3 |

**Windows 额外依赖**：Microsoft C++ Build Tools 或 Visual Studio 2022（供 Rust 编译使用）

---

## 🚀 快速开始

### 1. 克隆项目并切换分支

```bash
git clone <repo-url>
cd CviCubeMX
git checkout TypeScript_tauri
```

### 2. 安装依赖

```bash
npm install
```

### 3. 启动开发模式

```bash
npm run tauri:dev
# 等价于: npx tauri dev
```

这会启动 Vite Dev Server（端口 1420）并自动打开 Tauri 桌面窗口。

---

## 📦 构建与发布

### 开发构建（无打包）

仅验证前端和 Rust 桥接的构建完整性，不生成安装包：

```bash
npx tauri build --no-bundle
```

构建产物位于：`src-tauri/target/release/cvicubemx.exe`

### 生产构建（生成安装包）

```bash
npx tauri build
```

生成 Windows NSIS 安装程序 / `.msi`，产物位于：

```
src-tauri/target/release/bundle/
├── nsis/          # NSIS 安装包 (.exe)
└── msi/           # Windows Installer (.msi)
```

### GitHub Release 自动发布

仓库内置 `.github/workflows/release.yml`，在 CI 校验通过后会分别构建 Windows 和 Ubuntu 安装包，并上传到 GitHub Release。

触发方式：

```bash
git tag v0.0.1
git push origin v0.0.1
```

也可以在 GitHub Actions 页面手动运行 `Build and Release` workflow，并填写发布 tag。

发布产物包括：

```
Windows: .exe(NSIS), .msi
Ubuntu: .deb, .AppImage
```

### 仅构建前端

```bash
npm run build
```

前端打包产物输出到 `dist/` 目录。

---

## 🧪 测试

### 前端单元测试（Vitest）

```bash
# 运行全套单元测试
npx vitest run

# 运行并生成覆盖率报告
npx vitest run --coverage

# 监听模式（开发时使用）
npx vitest
```

**当前测试通过状态**：
```
Test Files  7 passed (7)
     Tests  165 passed (165)
  Coverage  v8 (src/stores: ~57% | src/utils: ~97%)
```

### Rust 后端测试（cargo test）

```bash
# 运行全部测试（单元测试 + 集成测试）
cd src-tauri
cargo test

# 仅运行单元测试
cargo test --lib

# 仅运行集成测试
cargo test --test '*'
```

**当前测试通过状态**：
```
running 199 tests
test result: ok. 199 passed; 0 failed; 0 ignored
```

集成测试覆盖以下模块：
- `chip_spec_test` — 芯片规格加载与引脚数校验
- `pin_data_test` — BGA 四角剔除、PAD 名清洗、重映射规则
- `dts_parser_test` — 设备树属性解析与 DMA 联动修改
- `clock_calc_test` — PLL 时钟级联频率重算与 defconfig 导出
- `memory_validate_test` — 内存重叠与地址约束硬校验
- `flash_validate_test` — Flash 分区容量溢出与标签校验
- `codegen_test` — 代码生成与增量占位符合并

### 端到端一键测试脚本

```bash
# Linux / macOS / Git Bash
bash scripts/e2e_test.sh
```

此脚本自动串联执行：前端构建 → Vitest 覆盖率测试 → Rust 全套测试 → Tauri 打包验证。

---

## 🔄 CI/CD（GitHub Actions）

向 `TypeScript_tauri` 分支提交 PR 时，将自动触发三个强门禁 Job，**全部通过方可合并**：

| Job | 内容 |
|-----|------|
| `frontend-tests` | `npm ci` → `npm run build` → `npx vitest run --coverage` |
| `rust-tests` | `cargo build` → `cargo test --lib` |
| `integration-tests` | `cargo test --test '*'` → `npx tauri build --no-bundle` |

配置文件：[`.github/workflows/test.yml`](.github/workflows/test.yml)

---

## 📁 项目结构

```
CviCubeMX/
├── src/                        # 前端 React 源码
│   ├── App.tsx                 # 主应用路由与布局
│   ├── main.tsx                # 入口文件
│   ├── components/             # 可复用 UI 组件
│   │   ├── ChipCanvas.tsx      # 芯片物理引脚视图（QFN/BGA）
│   │   ├── PinButton.tsx       # 引脚交互按钮
│   │   ├── PeripheralTree.tsx  # 外设资源树
│   │   ├── ConfigDialog.tsx    # 外设参数配置弹窗
│   │   ├── MemoryTable.tsx     # 内存区域配置表格
│   │   ├── PartitionTable.tsx  # Flash 分区配置表格
│   │   └── ChatPanel.tsx       # AI Chat 对话面板
│   ├── pages/                  # 页面组件
│   │   ├── PinoutPage.tsx      # 引脚配置页
│   │   ├── ClockPage.tsx       # 时钟树配置页
│   │   ├── MemoryPage.tsx      # 内存配置页
│   │   ├── FlashPage.tsx       # Flash 分区页
│   │   ├── AIChatPage.tsx      # AI 智能助手页
│   │   └── CodeGenPage.tsx     # 代码生成页
│   ├── stores/                 # Zustand 状态管理
│   │   ├── chipStore.ts        # 芯片选型 Store
│   │   ├── clockStore.ts       # 时钟树 Store
│   │   ├── peripheralStore.ts  # 外设配置 Store
│   │   ├── memoryStore.ts      # 内存配置 Store
│   │   ├── flashStore.ts       # Flash 分区 Store
│   │   └── chatStore.ts        # AI Chat Store
│   └── __tests__/              # Vitest 单元测试
│
├── src-tauri/                  # Rust 后端（Tauri）
│   ├── src/
│   │   ├── lib.rs              # Crate 入口，注册所有 Tauri 命令
│   │   ├── chip_spec.rs        # 芯片规格模块（M2）
│   │   ├── pin_data.rs         # 引脚数据模块（M2）
│   │   ├── pin_data_tool.rs    # 引脚工具函数
│   │   ├── peripheral.rs       # 外设信息与 DTS 状态（M3）
│   │   ├── dts_parser.rs       # 设备树解析器（M3）
│   │   ├── dts_writer.rs       # 设备树写入器（M3）
│   │   ├── clock_calc.rs       # 时钟频率计算（M4）
│   │   ├── clock_commands.rs   # 时钟 Tauri 命令（M4）
│   │   ├── memory.rs           # 内存配置与校验（M5）
│   │   ├── flash.rs            # Flash 分区与校验（M6）
│   │   ├── codegen.rs          # 代码生成器（M7）
│   │   ├── codegen_commands.rs # 代码生成 Tauri 命令（M7）
│   │   └── ai_chat.rs          # AI SSE 流式请求（M8）
│   ├── tests/                  # Rust 集成测试
│   │   ├── chip_spec_test.rs
│   │   ├── pin_data_test.rs
│   │   ├── dts_parser_test.rs
│   │   ├── clock_calc_test.rs
│   │   ├── memory_validate_test.rs
│   │   ├── flash_validate_test.rs
│   │   └── codegen_test.rs
│   ├── templates/
│   │   └── cvi_board_init.c.hbs    # Handlebars 代码生成模板
│   ├── pin_data.json               # 芯片引脚原始数据
│   ├── Cargo.toml
│   └── tauri.conf.json
│
├── scripts/
│   ├── e2e_test.sh             # 端到端一键测试脚本
│   └── run_all_tests.sh        # 前后端测试脚本
│
├── .github/
│   └── workflows/
│       └── test.yml            # GitHub Actions CI/CD
│
├── package.json
├── vite.config.ts
├── vitest.config.ts
├── tsconfig.json
└── README.md
```

---

## 💡 生成代码示例

配置完引脚后，生成的 `cvi_board_init.c` 代码示例：

```c
/**
 * @file cvi_board_init.c
 * @brief Board initialization file generated by CviCubeMX
 */

#include "cvi_board_init.h"
#include "pinmux.h"

int cvi_board_init(void)
{
    // Generated PINMUX configurations
    PINMUX_CONFIG(PAD_MIPI_TXM4, XGPIOC_18);

    // UART0 group
    PINMUX_CONFIG(UART0_TX, UART0_TX);
    PINMUX_CONFIG(UART0_RX, UART0_RX);

    /* Special sequence: configure EPHY for GPIO on ETH pads */
    mmio_write_32(0x03009804, mmio_read(0x03009804) | 0x1);
    // ... (ETH / MIPI / Audio 特殊寄存器序列)

    return 0;
}
```

---

## ❓ 故障排除

### `cargo build` 失败：找不到 MSVC 工具链
- 安装 [Microsoft C++ Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/) 或 Visual Studio 2022（需勾选「使用 C++ 的桌面开发」）
- 或通过 `rustup target add x86_64-pc-windows-msvc` 添加目标

### `npx tauri dev` 启动失败：WebView2 未安装
- 下载并安装 [Microsoft Edge WebView2 Runtime](https://developer.microsoft.com/zh-cn/microsoft-edge/webview2/)

### 前端 Vitest 测试失败
- 确认已运行 `npm install` 安装所有依赖，包括 `@vitest/coverage-v8`
- 检查 `vitest.config.ts` 中的 `setupFiles` 路径是否正确

### Rust 测试失败：集成测试找不到模块
- 确认在 `src-tauri/src/lib.rs` 中所有模块已以 `pub mod` 形式导出
- 集成测试需通过 `use cvicubemx_lib::<module>` 引用

---

## 📄 许可证

本项目采用 MIT 许可证。详情请参见 LICENSE 文件。

## 📬 联系方式

如有问题或建议，请通过 GitHub Issues 联系项目维护者。
