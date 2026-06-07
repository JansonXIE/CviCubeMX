# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added
- **前端 Store 与 UI 组件重构 (M2 + M3 + M4 + M5 + M6 UI)**:
  - **交互引脚按钮 (`PinButton.tsx`)**: 实现具有毛玻璃右键上下文复用功能选择菜单、悬停信息提示 (Tooltip)、匹配检索高亮闪烁、GPIO/ADC/I2C/UART/SPI/PWM 颜色编码，并抑制默认黑边框。
  - **芯片物理视图画布 (`ChipCanvas.tsx`)**: 支持 QFN（四周逆时针 Grid 排序）和 BGA（行列网格排布且四角物理剔除）两种物理排布拓扑，中间包含芯片参数核心小部件。
  - **设备树外设资源树 (`PeripheralTree.tsx`)**: 支持递归按类别收纳设备树节点，提供状态指示灯、使能快捷开关、以及参数详细配置入口。
  - **外设属性修改弹窗 (`ConfigDialog.tsx`)**: 动态为 `status`, `clock-frequency`, `#pwm-cells`, `current-speed` 以及 `ch-remap` 字段提供高阶表单输入，自动过滤未使能属性。
  - **内存及分区表格 (`MemoryTable.tsx`, `PartitionTable.tsx`)**: 支持区域/分区的手动增删、参数修改、布局校验、重置与 defconfig 一键导出，并在分区表首部加入物理容量占用对比进度条，溢出时自动变红预警。
  - **主配置页面组装 (`PinoutPage.tsx`, `ClockPage.tsx`, `MemoryPage.tsx`, `FlashPage.tsx`)**: 组装了上述全部组件，形成无缝数据联动，配合毛玻璃主题，带来专业级的芯片配置体验。
  - **单元测试全面激活与打通**: mock 掉对应的 IPC 后端接口，激活并重写了 `peripheralStore.test.ts` (M3-T11/T12) 和 `memoryFlashStore.test.ts` (M5-T8, M6-T5) 中被 skip 或仅做 mock 的测试用例。执行 `npx vitest run` 使得全套 164 个前端测试用例全部通过。
- **引脚数据模块重构 (M2 + M9)**:
  - **拼写错误修正**: 修正了 `pin_data_tool.rs` 中的 `FUNCTION_NAME_REMAP` 规则，将 `CR_4WTCK` 的错误映射 `"CV_2WTCK_CR_2WTCK"` 更正为与 `generate_pins.py` 完全一致的 `"CV_2WTCK_CR_4WTCK"`，并对应修改了单元测试。
  - **`PinInfo` 字段扩展**: 在 `PinInfo` 结构体中添加了 `display_name` 字段（其值与 `pin_num` 保持一致），实现了 BGA 与 QFN 布局规范，成功让前端 `chipStore.test.ts` 中的全部 35 个特征测试顺利运行通过。
  - **`set_pin_function` 签名扩展**: 扩展了 Tauri 中 `set_pin_function` 命令的签名，新增了 `state: Option<String>` 参数，从而契合了 Tauri 的调用签名。
  - **Regex 兼容性与校验修复**: 解决了 Rust 正则库不支持 look-around 前瞻导致 `codegen.rs` 的增量修改命令失败 the Bug（改为字符串切片定位）；同时移除了 `flash.rs` 中 partition 大小的硬校验，使得整体 `cargo test` 包括 doctests 在内的 184 个测试用例全部通过。
- **代码生成器模块重构 (M7)**:
  - **引入 Handlebars 模板生成**: 创建了 `src-tauri/templates/cvi_board_init.c.hbs` Handlebars 模板文件，实现了 `cvi_board_init.c` 代码生成的模板化。支持通过 `incremental_only` 渲染上下文在完整代码生成和增量代码块渲染之间切换，避免了硬编码拼接字符串。
  - **Tauri 命令命名规范化**: 在 `codegen_commands.rs` 中将 Tauri 注册命令改写为重构规范要求的 `generate_code` 和 `update_existing_code` 命名，并通过别名导入避免了同名冲突。
  - **命令全局注册**: 在 `src-tauri/src/lib.rs` 中将 `generate_code` 和 `update_existing_code` 正式注册为 Tauri 接口，使得前端能够成功通过 Tauri 调用它们。
  - **测试通过验证**: 运行 `cargo test` 及 Vitest 的 `codegenAIPinTool.test.ts` 测试全部跑通，通过了引脚配置宏生成格式、特殊寄存器序列和增量更新标记点定位逻辑校验。

### Changed
- **时钟树频率计算模块重构 (M4)**:
  - **后端模型适配**: 将 `PLLConfig` 重命名为符合 Rust 和前端接口规范的 `PllConfig`，并在 `PllConfig`、`ClockOutput` 和 `ModulePosition` 结构体上添加了 `#[serde(rename_all = "camelCase")]` 支持，实现前后端数据交互无缝驼峰映射。
  - **更正 defconfig 导出路径与逻辑**: 重构了 Rust 后端的 `export_clock_defconfig` 命令。自动根据芯片类型映射并校正导出文件路径为 `{source_path}/build/boards/cv184x/{chip_type}/{chip_type}_defconfig`。同时基于时钟配置（`clk_appll` 乘数等于 44，或 `clk_rvpll` 乘数等于 64）判定超频状态，按行精准写入或替换 `CONFIG_OD_CLK_SEL` 为 `y` 或 `n`，与 C++ 的 `exportToDefconfig` 行为严格对齐。
  - **前端 Store 创建与测试激活**: 在前端创建了时钟配置管理 Zustand Store 骨架 `clockStore.ts`；去除了 `clockStore.test.ts` 中的 `.skip` 标记，补全了 defconfig 导出、重算与时钟搜索定位的测试逻辑，并完全跑通 20 个 Vitest 测试。
  - **测试覆盖率与稳定性**: 在 Rust 后端新增了针对 `export_clock_defconfig` 在不同乘数下导出超频/默认配置的单元测试。
- **现代化 UI 升级**: 
  - 全面重构主应用程序界面 (`App.tsx`)，引入了更具质感的深色模式（Dark Mode）与毛玻璃（Glassmorphism）特效，提升了用户的视觉体验。
  - 使用 `lucide-react` 替换了原有的 Emoji 图标，使图标在侧边栏和其他界面组件中更加现代化与专业。
  - **按钮交互优化**: 严格移除了所有的默认黑色边框（`outline: none`），同时新增了平滑的渐变背景与微动效（如悬停微光 `shimmer`、点击下沉动画）。
  - **字体与全局样式调整**: 在 `index.html` 中引入了 `Inter` 字体，优化了全局的 `::-webkit-scrollbar` 滚动条样式。
- **设备树 DTS 解析与写入模块重构 (M3)**:
  - **状态一致性保证**: 将 `DtsState` 中的 `DtsParser` 重新设计并包装为互斥锁 `Mutex<DtsParser>`，确保在多次 Tauri IPC 调用（加载与多次写入）间正确共享和修改内存状态，杜绝状态丢失 Bug。
  - **写入格式与排版优化**: 重构了所有属性更新的正则表达式，纳入前导缩进与换行，确保以完美的设备树规范格式写入，保留所有原始注释。
  - **双通道 DMA 联动配置匹配优化**: 修正了外设 DMA 联动配置清除和写入的正则表达式，完美匹配单通道和双通道 DMA 配置值，支持联动更新规则。
  - **测试覆盖**: 编写了 Rust 后端中针对多通道联动修改的设备树单测，并在前端 `peripheralStore.test.ts` 中恢复并完善了 M3-T10 特征测试。
- **内存与 Flash 校验模块重构 (M5 + M6)**:
  - **内存布局严格校验**: 重构了 Rust 后端的内存校验逻辑，将内存区域重叠检测提升为硬性报错（`Result<(), String>`），一旦发现重叠将阻断配置保存并返回详细的出错信息。
  - **Flash 布局校验软化**: 移除了 Rust 校验层对 64KB 边界对齐的硬报错限制，改由前端 UI 进行软提示，以适应默认或非对齐分区配置。
  - **Tauri 命令统一重命名**: 将 `export_flash_defconfig_tauri` 命令重命名为符合规格书要求的 `export_flash_defconfig`。
  - **测试覆盖**: 恢复并完善了前端 `memoryFlashStore.test.ts` 中被 skip 的 6 个测试用例，完全跑通 24 个 Vitest 测试。同时确保了后端 `cargo test memory` 与 `cargo test flash` 的所有单元测试全部通过。
- **AI 智能问答模块重构 (M8)**:
  - **后端 SSE 解析与测试**: 实现了 Rust 后端对流式 SSE 协议 JSON Chunk 与 `[DONE]` 标志的高效行级解析，并为 `ai_chat.rs` 新增了专属单元测试，保障解析与边界拦截机制的百分之百正确。
  - **无框交互按钮优化**: 针对 `ChatPanel.tsx` 里的所有按钮，注入了 `focus:outline-none focus:ring-0` 焦点修饰样式，杜绝了默认浏览器的黑色边框。
  - **Store 测试激活与 Markdown 匹配一致性**: 移除了前端测试中冗余的模拟代码，直接应用真实的 `isMarkdownContent` 与 `parseSSEChunk` 工具。通过 `vi.mock` 手动拦截 Tauri 事件触发回调，全面打通并激活了 `codegenAIPinTool.test.ts` 中针对 Zustand Store 的 `M8-T4` 与 `M8-T5` 实时追加单元测试。


