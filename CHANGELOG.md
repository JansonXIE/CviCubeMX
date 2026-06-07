# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Changed
- **现代化 UI 升级**: 
  - 全面重构主应用程序界面 (`App.tsx`)，引入了更具质感的深色模式（Dark Mode）与毛玻璃（Glassmorphism）特效，提升了用户的视觉体验。
  - 使用 `lucide-react` 替换了原有的 Emoji 图标，使图标在侧边栏和其他界面组件中更加现代化与专业。
  - **按钮交互优化**: 严格移除了所有的默认黑色边框（`outline: none`），同时新增了平滑的渐变背景与微动效（如悬停微光 `shimmer`、点击下沉动画）。
  - **字体与全局样式调整**: 在 `index.html` 中引入了 `Inter` 字体，优化了全局的 `::-webkit-scrollbar` 滚动条样式。
