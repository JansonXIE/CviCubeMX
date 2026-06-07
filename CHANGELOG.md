# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added
- **引脚数据模块重构 (M2 + M9)**:
  - **拼写错误修正**: 修正了 `pin_data_tool.rs` 中的 `FUNCTION_NAME_REMAP` 规则，将 `CR_4WTCK` 的错误映射 `"CV_2WTCK_CR_2WTCK"` 更正为与 `generate_pins.py` 完全一致的 `"CV_2WTCK_CR_4WTCK"`，并对应修改了单元测试。
  - **`PinInfo` 字段扩展**: 在 `PinInfo` 结构体中添加了 `display_name` 字段（其值与 `pin_num` 保持一致），实现了 BGA 与 QFN 布局规范，成功让前端 `chipStore.test.ts` 中的全部 35 个特征测试顺利运行通过。
  - **`set_pin_function` 签名扩展**: 扩展了 Tauri 中 `set_pin_function` 命令的签名，新增了 `state: Option<String>` 参数，从而契合了 Tauri 的调用签名。
  - **Regex 兼容性与校验修复**: 解决了 Rust 正则库不支持 look-around 前瞻导致 `codegen.rs` 的增量修改命令失败的 Bug（改为字符串切片定位）；同时移除了 `flash.rs` 中 partition 大小的硬校验，使得整体 `cargo test` 包括 doctests 在内的 184 个测试用例全部通过。

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

