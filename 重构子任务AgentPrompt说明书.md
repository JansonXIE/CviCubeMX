# CviCubeMX 重构子任务 Agent Prompt 说明书

> 本文档为每个重构子任务提供系统级输入 Prompt，供多个 Agent 独立执行。
> 每个 Prompt 包含：任务背景、C++ 参考源码位置、目标输出文件、核心逻辑提取、验证标准。
> Agent 应严格按 Prompt 执行，输出文件到指定路径，完成后运行相关测试验证。

---

## 子任务 0: 初始化 Tauri 项目骨架 (M1.1)

### Agent Prompt

```
你是 CviCubeMX 项目的重构工程师。当前项目根目录是 C:\Users\jansonxie\Desktop\github_code\CviCubeMX。

任务: 初始化 Tauri v2 + React + TypeScript 项目骨架。

当前项目已有:
- package.json (含 vitest 测试依赖)
- vitest.config.ts (测试配置)
- tsconfig.json (TypeScript 配置)
- src/__tests__/ (5个测试文件, 121个通过的测试)
- C++ 源码在 src/ 目录 (mainwindow.h/cpp, chipconfig.h/cpp 等)

需要创建/安装:
1. 使用 npm create tauri-app@latest 在现有目录初始化 Tauri v2 项目 (React + TypeScript 模板), 但不要覆盖已有的 package.json 和测试文件
2. 安装前端依赖: React 18, React Router 6, Zustand 4, Tailwind CSS 3, Vite 5
3. 创建 src-tauri/ 目录结构:
   - Cargo.toml (Tauri v2 依赖)
   - src/main.rs (Tauri 入口)
   - src/lib.rs (命令注册)
   - tauri.conf.json (窗口配置: 标题"CviCubeMX", 尺寸1200x800)
4. 创建前端目录结构:
   - src/main.tsx (React 入口)
   - src/App.tsx (主应用组件，含 Sidebar + TopBar + TabBar + ContentArea 路由)
   - src/stores/ (Zustand store 目录)
   - src/components/ (React 组件目录)
   - src/pages/ (页面组件目录)
5. 验证 Tauri IPC 通信: 在 src-tauri/src/lib.rs 中添加一个测试命令 `greet(name: String) -> String`, 在前端调用并验证返回

C++ 参考源码 (理解原有结构):
- src/mainwindow.h: MainWindow 类定义，含所有 UI 组件和 slot 函数
- src/mainwindow.cpp (2058行): 整个应用的外壳和导航逻辑

验证标准:
1. npm run dev 能启动 Tauri 应用并显示窗口
2. invoke("greet", {name: "test"}) 返回正确字符串
3. 已有的 vitest 测试仍然全部通过 (npx vitest run)
4. React Router 能在 Sidebar 点击时正确切换页面
```

---

## 子任务 1: Rust 引脚数据模块 (M2.1 + M2.2 + M9)

### Agent Prompt

```
你是 CviCubeMX 项目的重构工程师。当前项目根目录是 C:\Users\jansonxie\Desktop\github_code\CviCubeMX。

任务: 实现 Rust 后端的引脚数据模块 (对应重构计划 M2 + M9)。

核心参考数据 (已在 tests/fixtures/ 中):
- tests/fixtures/chip_specs_reference.json: 6款芯片的规格 (chipType, package, pinCount)
- tests/fixtures/function_name_remap.json: 8条 FUNCTION_NAME_REMAP 规则

C++ 参考源码 (必须读取并理解):
- src/chipconfig.h (35行): ChipConfig 类, getPinCountForChip() 返回每款芯片的引脚数
- src/chipconfig.cpp (73行): setChipType, getPinFunction, getAllPinFunctions 等方法
- src/pinfunction.cpp (2073行): 所有引脚的硬编码功能数据 (m_pinFunctions, m_defaultFunctions, m_functionMacros)
- src/pinfunction.h: PinFunction 类, getSupportedFunctions, getDefaultFunction 等
- boards_pinout/generate_pins.py (380行): 从 Excel 解析引脚数据的 Python 脚本, 含 FUNCTION_NAME_REMAP 和 Pin Name 清理逻辑

需要创建的 Rust 文件:
1. src-tauri/src/chip_spec.rs:
   - ChipSpec struct: chip_type, package (QFN/BGA), pin_count, layout params
   - 6款芯片的硬编码规格数据 (从 chipconfig.cpp 提取)
   - #[tauri::command] load_chip_spec(chip_type: String) -> Result<ChipSpec, String>
2. src-tauri/src/pin_data.rs:
   - PinInfo struct: pin_name, display_name, supported_functions, default_function, current_function, user_configured
   - 从 pinfunction.cpp 提取所有引脚功能数据到 Rust 代码或 JSON 文件
   - #[tauri::command] load_pin_data(chip_type: String) -> Result<Vec<PinInfo>, String>
   - #[tauri::command] set_pin_function(chip_type, pin_name, function, state) -> Result<(), String>
3. src-tauri/src/pin_data_tool.rs (M9 CLI 工具):
   - FUNCTION_NAME_REMAP 映射表 (8条规则, 从 generate_pins.py 提取)
   - Pin Name 清理逻辑 (___分割)
   - Description 列解析 ("0 : UART0_TX (default)" 格式)
   - 引脚自然排序 (A2 < A10)
   - BGA 四角剔除 (A1/A15/R1/R15)
4. 在 src-tauri/src/lib.rs 注册所有新命令

关键约束:
- QFN 引脚编号规则: 从左侧上端逆时针编号 (1→左→底→右→顶)
- BGA 四角排除: A1/A15/R1/R15 不出现; 行字母跳过 I
- FUNCTION_NAME_REMAP 必须与 generate_pins.py 中的 8 条规则完全一致
- Pin Name 含 ___ 时只取第一段 (PAD_ETH_RXM___EPHY_TXP → PAD_ETH_RXM)
- 默认功能查找逻辑: 有 (default) 标记取该功能; 无标记取第一个含 XGPIO 的; 否则取第一个

验证标准:
1. cargo test 通过所有 Rust 单元测试
2. npx vitest run 中 M2 相关测试 (chipStore.test.ts) 的参考数据与 Rust 实现一致
3. invoke("load_chip_spec", {chipType: "cv1842hp"}) 返回正确的 ChipSpec JSON
4. invoke("load_pin_data", {chipType: "cv1842hp"}) 返回的引脚数据与 pinfunction.cpp 硬编码一致
```

---

## 子任务 2: Rust DTS 解析/写入模块 (M3)

### Agent Prompt

```
你是 CviCubeMX 项目的重构工程师。当前项目根目录是 C:\Users\jansonxie\Desktop\github_code\CviCubeMX。

任务: 实现 Rust 后端的 DTS (设备树) 解析和写入模块 (对应重构计划 M3)。

C++ 参考源码 (必须读取并理解):
- src/dtsconfig.h (117行): PeripheralInfo struct (name, status, clockName, clockFreq, clockFrequency, pwmCells, currentSpeed, sysdmaChannels, 各种 has* 标志, lineNumber)
- src/dtsconfig.cpp (949行):
  - parseDtsFile(): 解析整个 .dtsi 文件
  - parseNode(): 解析单个外设节点 (提取 status, clock-frequency, #pwm-cells, current-speed, ch-remap 等属性)
  - findNodePosition(): 查找节点在文件中的位置 (用于行级定位修改)
  - setPeripheralStatus/setPeripheralClockFrequency/setPeripheralPwmCells/setPeripheralCurrentSpeed: 各属性修改方法
  - updateSinglePeripheralContent(): 使用行级定位精确替换属性值
  - setPeripheralSysdmaChannels(): 修改 SYSDMA 通道映射
  - createSysdmaRemapNode(): 创建 sysdma_remap 子节点
  - updatePeripheralDmaConfigWithPrevious(): DMA 配置联动更新
  - getChannelNumber/getChannelName: SYSDMA 常量名和数字的互转

关键约束:
- DTS 文件格式: &node_name { property = value; }; 包含注释行
- 行级定位修改: 修改属性时只替换该行，不重写整个文件 (保留注释和格式)
- PeripheralInfo 默认构造: sysdmaChannels = ["0","5","12","13","42","42","4","7"]
- SYSDMA 通道映射修改时需要联动更新相关外设的 DMA 配置

需要创建的 Rust 文件:
1. src-tauri/src/dts_parser.rs:
   - DtsParser: 解析 .dtsi 文件内容为结构化数据
   - 解析 &node { ... }; 格式的节点
   - 提取属性: status, clock-frequency, #pwm-cells, current-speed, ch-remap 等
   - 保持每行的行号信息 (用于精确修改)
2. src-tauri/src/dts_writer.rs:
   - DtsWriter: 修改属性时只替换目标行，保留文件其他部分不变
   - 行级定位: 根据 lineNumber 精确替换属性值
   - 状态切换: status "okay" ↔ "disabled"
3. src-tauri/src/peripheral.rs:
   - PeripheralInfo struct (对应 C++ PeripheralInfo)
   - #[tauri::command] load_dts_peripherals(file_path: String) -> Result<Vec<PeripheralInfo>, String>
   - #[tauri::command] set_peripheral_status(peripheral, status) -> Result<(), String>
   - #[tauri::command] set_peripheral_clock_frequency(peripheral, frequency) -> Result<(), String>
   - #[tauri::command] set_peripheral_pwm_cells(peripheral, cells) -> Result<(), String>
   - #[tauri::command] set_peripheral_current_speed(peripheral, speed) -> Result<(), String>
   - #[tauri::command] set_peripheral_sysdma_channels(peripheral, channels) -> Result<(), String>
4. 在 src-tauri/src/lib.rs 注册所有新命令

验证标准:
1. cargo test 通过所有 Rust 单元测试
2. DTS 解析: 输入 &i2c0 { status = "okay"; clock-frequency = <100000>; }; → 输出正确的 PeripheralInfo
3. DTS 写入保留格式: 修改 status 后写回，未修改部分保持原样
4. SYSDMA 默认通道: ["0","5","12","13","42","42","4","7"]
5. npx vitest run 中 M3 相关测试 (peripheralStore.test.ts) 参考数据与实现一致
```

---

## 子任务 3: Rust 时钟计算模块 (M4)

### Agent Prompt

```
你是 CviCubeMX 项目的重构工程师。当前项目根目录是 C:\Users\jansonxie\Desktop\github_code\CviCubeMX。

任务: 实现 Rust 后端的时钟树频率计算模块 (对应重构计划 M4)。

C++ 参考源码 (必须读取并理解):
- src/clockconfig.h (703行):
  - PLLConfig struct: name, enabled, inputFreq, outputFreq, divider (支持小数), multiplier, source
  - ClockOutput struct: name, source, divider, multiplier, frequency, enabled
  - ModulePosition struct: moduleName, x, y, width, height
  - 常量: OSC_FREQUENCY (25MHz), RTC_FREQUENCY (32.768kHz)
  - 大量子节点类型: clk_1M, clk_cam1pll, clk_raw_axi, clk_cam0pll, clk_disppll, clk_sys_disp, clk_a0pll, clk_rvpll, clk_appll, clk_fpll, clk_tpll, clk_mpll, clk_fab_100M, clk_xtal_misc, clk_i2c, clk_apb_i2c, clk_apb_vcsys, clk_x2p, clk_hsperi, clk_rtc_sys, clk_vip_sys_0/1/2/3, clk_spi, clk_keyscan_xclk, clk_wgn_xclk
- src/clockconfig.cpp (9745行!):
  - updatePLLFrequency(): PLL 频率计算 = input * multiplier / divider
  - updateSubPLLFrequency(): SubPLL 级联计算
  - updateOutputFrequency(): 输出频率 = source / divider * multiplier
  - 各子节点频率更新函数 (updateClk1MSubNodeFrequency 等)
  - exportToDefconfig(): 导出到 defconfig 文件

关键计算公式:
- PLL: output_freq = input_freq * multiplier / divider (divider 支持小数)
- SubPLL: 级联, input = PLL output
- Clock Output: output_freq = source / divider * multiplier
- clk_1M 子节点: output = 1MHz / divider
- OSC = 25MHz, RTC = 32.768kHz

需要创建的 Rust 文件:
1. src-tauri/src/clock_calc.rs:
   - 常量定义: OSC_FREQUENCY_MHZ = 25.0, RTC_FREQUENCY_KHZ = 32.768
   - PLLConfig, ClockOutput, ModulePosition structs
   - compute_pll_frequency(input_freq, multiplier, divider) -> f64
   - compute_subpll_frequency(pll_output, multiplier, divider) -> f64
   - compute_output_frequency(source_freq, divider, multiplier) -> f64
   - compute_clk_1m_subnode_frequency(divider) -> f64
   - 所有子节点类型名称列表 (从 clockconfig.h 的常量定义提取)
2. src-tauri/src/clock_commands.rs:
   - #[tauri::command] compute_clock_tree(configs: HashMap<String, PllConfig>) -> Result<ClockTreeResult, String>
   - #[tauri::command] save_module_positions(positions: HashMap<String, ModulePosition>) -> Result<(), String>
   - #[tauri::command] load_module_positions() -> Result<HashMap<String, ModulePosition>, String>
   - #[tauri::command] export_clock_defconfig(source_path, chip_type, configs) -> Result<(), String>
3. 在 src-tauri/src/lib.rs 注册所有新命令

验证标准:
1. cargo test 通过所有 Rust 单元测试
2. PLL 计算: OSC(25) * 54 / 1 = 1350MHz ✓
3. SubPLL 级联: 1350 * 2 / 3 = 900MHz ✓
4. 输出分频: 900 / 3 * 1 = 300MHz ✓
5. clk_1M: 1 / 10 = 0.1MHz ✓
6. npx vitest run 中 M4 相关测试 (clockStore.test.ts) 参考数据与实现一致
```

---

## 子任务 4: Rust 内存/Flash 校验模块 (M5 + M6)

### Agent Prompt

```
你是 CviCubeMX 项目的重构工程师。当前项目根目录是 C:\Users\jansonxie\Desktop\github_code\CviCubeMX。

任务: 实现 Rust 后端的内存配置校验和 Flash 分区校验模块 (对应重构计划 M5 + M6)。

C++ 参考源码 (必须读取并理解):
- src/memoryconfig.h (177行):
  - MemoryRegion struct: name, startAddress, endAddress, size, sizeString, isEditable, description
  - 常量: TOTAL_MEMORY_SIZE (256MB = 0x10000000), MEMORY_BASE_ADDRESS (0x80000000)
  - 方法: saveConfig, loadConfig, exportToJson, importFromJson, exportToDefconfig
  - 校验: validateMemoryLayout, checkMemoryOverlap, validateMemoryConstraints
  - 格式化: formatSize (字节→M/K/B), formatAddress, parseAddress
- src/memoryconfig.cpp (1627行): 全部实现逻辑
- src/flashconfig.h (174行):
  - FlashPartition struct: partitionNumber, label, size(KB), sizeString, file, mountpoint, type, enabled
  - 方法: saveConfig, loadConfig, exportToJson, importFromJson, exportToDefconfig
  - 校验: validatePartitionLayout
- src/flashconfig.cpp (1412行): 全部实现逻辑

需要创建的 Rust 文件:
1. src-tauri/src/memory.rs:
   - 常量: TOTAL_MEMORY_SIZE = 0x10000000, MEMORY_BASE_ADDRESS = 0x80000000
   - MemoryRegion struct
   - validate_memory_layout(regions) -> Result<(), String>: 检查重叠、地址范围
   - check_memory_overlap(region1, region2) -> bool
   - format_size(bytes) -> String ("256M", "1K", "100B")
   - format_address(addr) -> String ("0x80000000")
   - parse_address(str) -> u64
   - #[tauri::command] load_memory_regions() -> Result<Vec<MemoryRegion>, String>
   - #[tauri::command] validate_memory(regions) -> Result<(), String>
   - #[tauri::command] export_memory_json(regions, path) -> Result<(), String>
   - #[tauri::command] export_memory_defconfig(regions, source_path, chip_type) -> Result<(), String>
2. src-tauri/src/flash.rs:
   - FlashPartition struct
   - validate_partition_layout(partitions, flash_size_kb) -> Result<(), String>
   - #[tauri::command] load_partitions() -> Result<Vec<FlashPartition>, String>
   - #[tauri::command] validate_partitions(partitions) -> Result<(), String>
   - #[tauri::command] export_flash_json(partitions, path) -> Result<(), String>
   - #[tauri::command] export_flash_defconfig(partitions, source_path, chip_type) -> Result<(), String>
3. 在 src-tauri/src/lib.rs 注册所有新命令

验证标准:
1. cargo test 通过所有 Rust 单元测试
2. 内存重叠检测: 两个重叠区域报错 ✓
3. 大小格式化: 268435456 → "256M" ✓
4. 地址格式化: 0x80000000 → "0x80000000" ✓
5. Flash 分区校验: 总大小超过 Flash 容量报错 ✓
6. 禁用分区不计入总大小 ✓
7. npx vitest run 中 M5/M6 相关测试 (memoryFlashStore.test.ts) 参考数据与实现一致
```

---

## 子任务 5: Rust 代码生成模块 (M7)

### Agent Prompt

```
你是 CviCubeMX 项目的重构工程师。当前项目根目录是 C:\Users\jansonxie\Desktop\github_code\CviCubeMX。

任务: 实现 Rust 后端的代码生成器模块 (对应重构计划 M7)。

C++ 参考源码 (必须读取并理解):
- src/codegenerator.h (42行): CodeGenerator 类
- src/codegenerator.cpp (638行):
  - generateCode(): 生成 cvi_board_init.c 代码
  - updateExistingFile(): 增量更新已有文件 (找到 "// Generated PINMUX configurations" 块并替换)
  - functionToMacro(): 将引脚功能转换为 PINMUX 宏名称
  - generatePinmuxFunction(): 生成引脚复用配置函数
  - generatePinmuxConfig(): 生成 PINMUX 配置块
  - generateEthSequence/generateMipiSequence/generateAudioSequence(): 特殊寄存器序列
  - isGpioMode(): 判断是否为 GPIO 模式
- include/cvi_board_init.h: 头文件定义

关键逻辑:
- PINMUX 宏格式: PINMUX(PAD_name, FUNCTION_name)
- 增量更新: 找到 "// Generated PINMUX configurations" 块，替换为新生成的配置
- ETH 序列: 包含 RMII0/EPHY/PAD_ETH 相关功能时生成 ETH 寄存器序列
- MIPI 序列: 包含 MIPI/VI0_D/VI1_D 相关功能时生成 MIPI 序列
- Audio 序列: 包含 IIS/IIC/AUD/SPK 相关功能时生成 Audio 序列
- 仅输出 user_configured = true 的引脚配置

需要创建的 Rust 文件:
1. src-tauri/src/codegen.rs:
   - 使用 Tera 或 Handlebars 模板引擎
   - templates/cvi_board_init.c.hbs 模板文件
   - generate_pinmux_macro(pin_name, function) -> String
   - generate_code(chip_type, pin_configs, output_path) -> Result<String, String>
   - update_existing_code(file_path, pin_configs) -> Result<String, String>
   - generate_eth_sequence/generate_mipi_sequence/generate_audio_sequence
   - is_gpio_mode(function) -> bool
   - ETH_KEYWORDS: ["RMII0", "EPHY", "PAD_ETH"]
   - MIPI_KEYWORDS: ["MIPI", "VI0_D", "VI1_D", "VI2_D"]
   - AUDIO_KEYWORDS: ["IIS", "IIC", "AUD", "SPK"]
2. src-tauri/src/codegen_commands.rs:
   - #[tauri::command] generate_code(chip_type, pin_configs, output_path) -> Result<String, String>
   - #[tauri::command] update_existing_code(file_path, pin_configs) -> Result<String, String>
3. 在 src-tauri/src/lib.rs 注册所有新命令

验证标准:
1. cargo test 通过所有 Rust 单元测试
2. PINMUX 宏格式: PINMUX(PAD_MIPI_TXM4, XGPIOC_18) ✓
3. 增量更新: 找到 Generated 块并正确替换 ✓
4. ETH/MIPI/Audio 序列关键词正确识别 ✓
5. npx vitest run 中 M7 相关测试 (codegenAIPinTool.test.ts) 参考数据与实现一致
```

---

## 子任务 6: Rust SSE 流式 + 前端 AI Chat (M8)

### Agent Prompt

````
你是 CviCubeMX 项目的重构工程师。当前项目根目录是 C:\Users\jansonxie\Desktop\github_code\CviCubeMX。

任务: 实现 Rust SSE 流式推送和前端 AI 聊天面板 (对应重构计划 M8)。

C++ 参考源码 (必须读取并理解):
- src/aichatdialog.h (86行): AIChatDialog 类
  - m_apiKey, m_baseUrl, m_model: API 配置
  - sendMessageToAI(): 发送 HTTP POST 请求到 AI API
  - onNetworkReplyReadyRead(): 解析 SSE chunk (data: {"choices":[...]} 格式)
  - isMarkdownContent(): 检测内容是否为 Markdown
  - updateAIResponseStreamContent(): 流式追加 AI 响应
- src/aichatdialog.cpp (881行): SSE 流式接收和 Markdown 检测的完整实现

关键逻辑:
- SSE 响应格式: data: {"choices":[{"delta":{"content":"..."}}]}\n\n
- 结束标记: data: [DONE]
- Markdown 检测: 含 #, -列表, ```代码块, **bold**, 数字列表 的内容
- Tauri Event System: Rust 收到 chunk → window.emit("ai-chunk", payload) → React listen() 接收

需要创建的文件:
1. src-tauri/src/ai_chat.rs:
   - AiApiConfig struct: api_key, base_url, model
   - SSE 解析: parse_sse_line(line) -> Option<SseChunk>
   - 使用 reqwest 发送 HTTP POST 请求 (stream: true)
   - 使用 Tauri Event System 推送: app_handle.emit("ai-chunk", payload)
   - #[tauri::command] send_ai_message(message, config) -> Result<(), String> (触发 SSE 流)
   - #[tauri::command] save_ai_config(config) -> Result<(), String>
   - #[tauri::command] load_ai_config() -> Result<AiApiConfig, String>
2. src/stores/chatStore.ts:
   - Zustand store: messages[], aiConfig, isStreaming, currentAIResponse
   - listen("ai-chunk") 接收流式推送并追加到消息末尾
   - sendMessage() 触发 invoke("send_ai_message")
3. src/components/ChatPanel.tsx:
   - 聊天界面: 消息列表 + 输入框 + 发送按钮
   - Markdown 渲染 (使用 react-markdown + remark-gfm)
   - 代码高亮
4. src/utils/markdownDetect.ts:
   - isMarkdownContent(): 检测内容是否为 Markdown
5. 在 src-tauri/src/lib.rs 注册所有新命令

验证标准:
1. SSE chunk 解析: data: {"choices":[{"delta":{"content":"Hello"}}]} → content="Hello" ✓
2. data: [DONE] → null ✓
3. Markdown 检测: # 标题、```代码块、**bold**、-列表 ✓
4. npx vitest run 中 M8 相关测试 (codegenAIPinTool.test.ts 的 M8 部分) 参考数据与实现一致
````

---

## 子任务 7: 前端 Store + 组件实现 (M2 UI + M3 UI + M4 UI + M5/M6 UI)

### Agent Prompt

```
你是 CviCubeMX 项目的重构工程师。当前项目根目录是 C:\Users\jansonxie\Desktop\github_code\CviCubeMX。

前提: 子任务 0 (Tauri 骨架) 和子任务 1-6 (Rust 后端) 已完成。

任务: 实现前端 Zustand Store 和 React 组件 (对应重构计划 M2 UI + M3 UI + M4 UI + M5/M6 UI)。

C++ 参考源码 (理解原有 UI 交互):
- src/mainwindow.h (186行): 所有 UI 组件和 slot 函数
- src/mainwindow.cpp (2058行): 整个应用 UI 交互逻辑
- src/pinwidget.h/cpp (92+282行): PinWidget 引脚按钮组件
- src/peripheralconfigdialog.h/cpp (79+629行): 外设配置弹窗

需要创建的前端文件:

1. src/stores/chipStore.ts (M2 Store):
   interface ChipStore {
     chipType: string | null;
     chipSpec: ChipSpec | null;
     pins: Map<string, PinInfo>;
     searchText: string;
     highlightedPins: Set<string>;
     selectChip(type: string): Promise<void>;  // invoke("load_chip_spec") + invoke("load_pin_data")
     setPinFunction(pin: string, fn: string): Promise<void>;  // invoke("set_pin_function")
     searchPin(text: string): void;
   }

2. src/stores/peripheralStore.ts (M3 Store):
   - peripherals: PeripheralInfo[]
   - loadPeripherals(path): invoke("load_dts_peripherals")
   - setPeripheralStatus, setClockFrequency, setPwmCells, setCurrentSpeed, setSysdmaChannels

3. src/stores/clockStore.ts (M4 Store):
   - pllConfigs: Map<string, PLLConfig>
   - outputs: Map<string, ClockOutput>
   - modulePositions: Map<string, ModulePosition>
   - computeClockTree(): invoke("compute_clock_tree")

4. src/stores/memoryStore.ts (M5 Store):
   - regions: MemoryRegion[]
   - addRegion, removeRegion, validateMemory, exportDefconfig

5. src/stores/flashStore.ts (M6 Store):
   - partitions: FlashPartition[]
   - addPartition, removePartition, validatePartitions, exportDefconfig

6. src/components/PinButton.tsx (M2 引脚按钮):
   - 右键 ContextMenu 选择功能
   - 选中后变色 (颜色编码: GPIO灰/ADC红/I2C蓝/UART绿/SPI橙/PWM紫)
   - 搜索时闪烁高亮

7. src/components/ChipCanvas.tsx (M2 芯片视图):
   - QFN 布局: CSS Grid 四边逆时针排列
   - BGA 布局: CSS Grid 网格排列, 四角缺失

8. src/components/PeripheralTree.tsx (M3 外设树):
   - 递归 Tree 组件显示外设列表
   - 点击展开配置弹窗

9. src/components/ConfigDialog.tsx (M3 配置弹窗):
   - 表单: 时钟/频率/PWM cells/波特率/SYSDMA 通道

10. src/components/PartitionTable.tsx + MemoryTable.tsx (M5/M6):
    - TanStack Table 数据表格
    - 添加/删除/重置按钮

11. src/pages/ 下各页面组件:
    - PinoutPage, PeripheralPage, ClockPage, MemoryPage, FlashPage, CodeGenPage, AIChatPage

验证标准:
1. npx vitest run 全部测试通过 (特别是之前 skip 的 M2-T8~T12, M3-T11/T12 等测试)
2. 选择芯片后引脚视图正确渲染 (QFN/BGA)
3. 设置引脚功能后 Store 和 UI 同步更新
4. 外设状态切换后 Store 和 DTS 文件同步更新
```

---

## 子任务 8: CI/CD 和集成测试完善 (Step 4)

### Agent Prompt

```
你是 CviCubeMX 项目的重构工程师。当前项目根目录是 C:\Users\jansonxie\Desktop\github_code\CviCubeMX。

前提: 子任务 0-7 全部完成。

任务: 完善 CI/CD Pipeline 和集成测试。

当前已有:
- .github/workflows/test.yml: GitHub Actions 测试 Pipeline
- scripts/run_all_tests.sh: 一键测试脚本
- src/__tests__/stores/: 5个测试文件 (121 passed + 20 skipped)
- tests/fixtures/: 参考数据 JSON 文件

需要完善:

1. 更新 .github/workflows/test.yml:
   - 增加构建验证步骤 (cargo build)
   - 增加 Tauri 应用打包步骤
   - 增加覆盖率报告
   - 配置 PR 门禁: 测试必须全部通过才能合并

2. 启用所有之前 skip 的测试:
   - 将 chipStore.test.ts 中的 5 个 skip 测试改为实际测试
   - 将 peripheralStore.test.ts 中的 5 个 skip 测试改为实际测试
   - 将 clockStore.test.ts 中的 4 个 skip 测试改为实际测试
   - 将 memoryFlashStore.test.ts 中的 6 个 skip 测试改为实际测试
   - 将 codegenAIPinTool.test.ts 中的 3 个 skip 测试改为实际测试

3. 添加 Rust 集成测试:
   - src-tauri/tests/chip_spec_test.rs
   - src-tauri/tests/pin_data_test.rs
   - src-tauri/tests/dts_parser_test.rs
   - src-tauri/tests/clock_calc_test.rs
   - src-tauri/tests/memory_validate_test.rs
   - src-tauri/tests/flash_validate_test.rs
   - src-tauri/tests/codegen_test.rs

4. 创建端到端测试脚本:
   - scripts/e2e_test.sh: 启动 Tauri 应用, 验证 IPC 通信, 各模块交互

验证标准:
1. npx vitest run: 0 skipped, 全部通过
2. cd src-tauri && cargo test: 全部通过
3. GitHub Actions Pipeline: frontend-tests + rust-tests + integration-tests 三个 job 全部通过
4. PR 合入 TypeScript_tauri 分支时自动触发测试
```

---

## 执行顺序建议

子任务之间有依赖关系，建议按以下顺序执行:

```
子任务 0 (Tauri 骨架)  ← 必须最先完成, 所有后续任务依赖此
  ↓
子任务 1 (Rust 引脚) ─── 子任务 2 (Rust DTS) ─── 子任务 3 (Rust 时钟)
  ↓ (可并行)              ↓ (可并行)               ↓ (可并行)
子任务 4 (Rust 内存/Flash) ─── 子任务 5 (Rust 代码生成) ─── 子任务 6 (Rust AI)
  ↓ (可并行)
子任务 7 (前端 Store + 组件) ← 依赖子任务 1-6 的 Rust commands
  ↓
子任务 8 (CI/CD 完善) ← 依赖子任务 7 全部完成
```

**并行分组:**

- **Group A** (可同时启动): 子任务 0
- **Group B** (可同时启动): 子任务 1, 2, 3, 4, 5, 6
- **Group C** (可同时启动): 子任务 7
- **Group D** (最后): 子任务 8
