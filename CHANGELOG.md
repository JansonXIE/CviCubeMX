# 更新日志 (CHANGELOG)

## [未发布] - 2026-06-08

### 变更

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
