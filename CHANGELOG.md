# 更新日志 (CHANGELOG)

## [未发布] - 2026-06-15

### 变更

- **时钟关系流程图宽度与保存逻辑修复**：
  - 修复了修改时钟节点卡片默认宽度（例如将子节点的 `width` 改为 200）后在界面上不生效的 Bug。
  - **分析原因**：位置合并逻辑使用 `{ ...DEFAULT_MODULE_POSITIONS, ...modulePositions }`，导致从后端缓存文件 `module_positions.json` 中读取出来的历史宽度（例如 180）直接覆盖了前端代码设定的新默认宽度。
  - **解决方案**：重构了 `ClockPage.tsx` 中的位置合并逻辑，确保卡片的 `width` 和 `height` 始终强制采用代码中定义的默认尺寸（`DEFAULT_MODULE_POSITIONS`），仅合并保存的 `x` 和 `y` 坐标。同时，在用户拖拽卡片触发自动存盘时，用最新的默认宽度和高度更新后端 JSON 文件。
- **时钟连线修复**：
  - 修复了 `clk_mpll子节点` 卡片中的 `clk_spi` 项到 `clk_spi子节点` 卡片之间没有引线的 Bug。原因是 `CONNECTIONS` 配置中 `fromNode` 错误指定为了非存存的 `"clk_mpll"` 节点名，已将其更正为真正的 `"clk_mpll子节点"`。
- **时钟默认节点坐标同步**：
  - 获取并读取了本地缓存数据库 `module_positions.json` 中保存的各时钟节点的拖拽自定义排版坐标，并同步修正了 `ClockPage.tsx` 中 `DEFAULT_MODULE_POSITIONS` 定义的默认 `x` 和 `y` 坐标值（涉及 `锁相环`、`clk_fpll子节点`、`clk_xtal_misc子节点`、`clk_i2c子节点`、`子锁相环`、`clk_fab_100m子节点`、`clk_apb_vcsys子节点`、`clk_1M子节点`），使系统默认布局更加整洁合规。
- **支持默认ND重置与OD超频配置**：
  - 移植了原 C++ 模块中的时钟配置重置和超频功能，在前端 `ClockPage.tsx` 顶部操作栏中新增了“重置为默认ND”和“OD超频配置”操作按钮。
  - **重置为默认ND**：将所有锁相环及子锁相环的倍频、分频系数重置为出厂默认参数，并自动更新重算整个时钟级联频率。
  - **OD超频配置**：实现一键应用超频参数（将 `clk_appll` 倍频设为 44，`clk_rvpll` 倍频设为 64），重算级联树，并在配置了 SDK 源码路径和芯片类型时自动向后端的板级 `defconfig` 导出超频标志 `CONFIG_OD_CLK_SEL=y`。

## [未发布] - 2026-06-09

### 变更

- **物理内存配置页 (MemoryPage) UI 重构与大小编辑功能**：
  - 将页面由原先的“上下垂直堆叠”重构为**“左右双栏自适应布局”**（左侧 65% 表格，右侧 35% 分布图），极大提高了宽屏显示器下的空间利用率。
  - **支持核心与自定义内存大小修改**：在左侧映射表操作列中，为所有可编辑区域引进了“编辑”按钮。用户可通过弹窗修改其大小（Size）和起始物理地址（支持 0x 十六进制及十进制输入），解决内置区域尺寸固定的限制。
  - **实现物理地址级联联动计算**：将 [memoryconfig.cpp](file:///c:/Users/jansonxie/Desktop/github_code/CviCubeMX/src/memoryconfig.cpp) 中高科技的内存映射联动重算机制移植到了前端 [memoryStore.ts](file:///c:/Users/jansonxie/Desktop/github_code/CviCubeMX/src/stores/memoryStore.ts) 中。当修改 `ION` 或 `RTOS_ION` 的大小时，会自动根据 256MB 边界及各自的大小占比重算起始物理地址和结束物理地址；并且会同步联动改写与 `ION` 绑定起始物理地址的 `H26X_BITSTREAM`、`H26X_ENC_BUFF`、`ISP_MEM_BASE` 的起止地址，避免空间重叠。
  - **只读输入安全性体验**：在编辑弹窗中，当编辑 `ION` 及其绑定的三个关联段时，其“起始物理地址”输入框将自动进入灰色只读模式并提示“只读/级联绑定”，确保用户在交互上不会产生混淆。
  - **安全性限制**：引入 `BUILTIN_REGIONS` 识别内置块，对系统核心内置段隐藏了“删除”按钮，仅保留“编辑”，避免误删导致配置崩溃。
  - **同步写入 build/.config**：完善了 Rust 后端导出命令 `export_memory_defconfig`。点击“保存并导出”时，不仅更新 `defconfig` 文件，还会**同步在运行环境 `build/.config` 配置文件中更新 `CONFIG_RTOS_ION_SIZE`, `CONFIG_ION_SIZE`, `CONFIG_RTOS_LOGO_SIZE` 等三个参数值**，支持完整的 RTOS_LOGO_SIZE 大小导出，实现与 SDK 编译系统无缝接轨。
  - 在右侧设计并引入了具有科技感和发光效果的**“垂直物理内存堆栈模型 (Vertical Concept Layout)”**，代表基地址 0x80000000 到 0x90000000 的 256MB DDR 物理空间。
  - 自动提取并填补了物理内存中未被分配的碎片化间隙，将其渲染为“[FREE] 未分配空间”，并配备了低调细致的**灰色斜条纹理背景**，使内存占比一目了然。
  - 使用了**“受限比例高度算法 (Clamp Proportional Height)”**（高度范围限缩在 34px 至 120px 之间），解决了原来超窄内存块（如 1K 大小）在 256MB DDR 总空间里被挤压为 0 像素而导致无法看见或交互的问题。
  - 实现了左侧表格与右侧分布图的**“左右双向交互高亮联动”**（Hover 左侧行，右侧块闪烁呼吸灯发光；Hover 右侧块，左侧行自动上色），显著增强了工具软件的交互质感。
  - 优化了 [MemoryTable](file:///c:/Users/jansonxie/Desktop/github_code/CviCubeMX/src/components/MemoryTable.tsx) 内部的布局高度逻辑，使用 `flex-grow` 和 `min-h-0` 替换写死的百分比，确保表格数据很多时依然可以在内部触发独立且平滑的滚动条，两边卡片在视觉上实现完美的像素级齐平。
- **设备树实时预览功能增强**：
  - 在 peripherals（外设）页面中，将“DTS 配置文件实时预览”的数据源从静态占位文本 `SAMPLE_DTS_CONTENT` 替换为 SDK 源码路径下的实际 `build/boards/default/dts/cv184x/cv184x_base.dtsi` 设备树配置文件。
  - 在 Rust 后端新增 `get_dts_content` 的 Tauri Command 接口，用于返回 `DtsParser` 中加载的设备树文件最新文本。
  - 在前端 `peripheralStore.ts` 中维护了 `dtsContent` 状态字段，并在外设加载（`loadPeripherals`）及每次属性更新操作（包括修改外设启用状态、修改时钟频率、PWM cells、波特率、SYSDMA 映射等）成功后，自动触发重新拉取，确保界面预览区域实时联动更新。
  - 修复了标题中由于 `uppercase` 类样式引起的 DTS 路径大写显示问题，将括号路径用 `normal-case` 包裹以保持原装的小写路径显示。

## [未发布] - 2026-06-08

### 变更

- **时钟关系流程图重构与修复**：
  - 修复了因为前端在初始化时钟树时传递了空的配置参数 `{}`，导致 Rust 端的 `compute_clock_tree` 接口未填充主要的锁相环与子锁相环（PLL & SubPLL）默认初始数据，致使前端的“锁相环”卡片和“子锁相环”卡片内部一片空白、无法渲染子项时钟及参数配置输入框的 Bug。现在在 Rust 端加入了对缺失时钟配置的自动补全补丁，与 C++ 源码默认配置一致，确保即使传入空配置也能正确渲染。
  - 修复了 `ClockPage` 连线配置数据中，部分连线将 `color` 字段误写为 `toItem: "#ff8c00"` 导致 React 渲染在 `color.replace` 时抛出空指针 TypeError 崩溃引发白屏的 Bug。
  - 修复了由于 `ContentArea` 的 `overflow-auto` 导致外部纵向滚动条被撑开，致使横向滚动条被埋在 4300px 画布底部而无法在视口内展示的布局 Bug。通过把 `main` 容器和 `ContentArea` 更改为 `overflow-hidden` 与列 Flex 布局，把画布高度完全约束在当前的视口中，使 `ClockPage` 实现了横向与纵向局部滚动条的完备呈现。
  - 实现了基于卡片顶部标题栏原生鼠标按下、移动与释放事件的卡片位置拖拽交互，且移动时级联的 SVG 连线及箭头伴随卡片坐标的更新而实时、流畅地重画。
  - 实现了拖拽结束后的卡片坐标自动存盘机制，即松开鼠标时自动调用 Tauri 的 `saveModulePositions` 命令更新至后端的 JSON 配置文件中。
  - 在卡片搜索匹配时，运用了淡化其它非匹配卡片与连线的高亮过滤机制。
- **数据同步**：根据 `src/pinfunction.cpp` 中的引脚功能定义，完整同步并更新了 `src-tauri/pin_data.json`。
  - 将引脚定义总数从 110 个补全至 175 个，修复了 65 个 BGA/QFN 引脚缺失的问题。
  - 对更新后的引脚列表进行了排序（QFN 按引脚号数字排序，BGA 按字母行和数字列排序）。
- **单元测试**：更新了 `src-tauri/src/pin_data.rs` 中的单元测试断言，确保引脚总数变更为 175 后测试全部通过。
- **界面优化**：
  - 修复了 `PinoutPage` 和 `ChipCanvas` 在视口高度不足时，底部 R 行引脚被遮挡的问题。通过调整 Flexbox 和 `overflow-auto` 属性，使得引脚网格可以在其容器内进行内部滚动。
  - 修复了右键/点击引脚弹出的功能选择菜单在底部引脚触发时会被屏幕底端遮挡的问题。在 `PinButton.tsx` 中引入了 `useLayoutEffect` 动态调整菜单的定位，防止其超出视口边缘。
- **搜索优化**：
  - 扩展了 `PinoutPage` 中的引脚搜索逻辑，支持在引脚的 `supported_functions`（即候选/备选复用功能）中进行模糊匹配。这使得在搜索“SPI1”等外设名时，能准确高亮支持该功能的引脚（例如 A6 与 A7，其备选功能包含 `MUX_SPI1_MOSI` / `MUX_SPI1_CS`），符合用户的直觉配置习惯。
- **单元测试**：
  - 在前端 `chipStore.test.ts` 中新增了 `searchPin` 备用功能搜索的测试用例，并在本地运行通过。
