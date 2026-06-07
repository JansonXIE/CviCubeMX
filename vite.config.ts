// @ts-nocheck
import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

// https://tauri.app/start/frontend/vite/
const host = process.env.TAURI_DEV_HOST;

export default defineConfig({
  plugins: [react()],
  // Vite 默认不清除屏幕，Tauri 开发时也保持这个行为
  clearScreen: false,
  server: {
    // Tauri 开发时使用固定端口，除非通过环境变量指定
    port: 1420,
    strictPort: true,
    host: host || false,
    watch: {
      // 忽略 Rust 后端变更，避免不必要的重启
      ignored: ["**/src-tauri/**"],
    },
  },
  // 环境变量前缀，只有 VITE_ 和 TAURI_ENV_* 开头的变量会暴露给前端
  envPrefix: ["VITE_", "TAURI_ENV_*"],
  build: {
    // Tauri 在 Windows 上使用 Chrome，在其他平台上使用 Safari
    target: process.env.TAURI_ENV_PLATFORM === "windows"
      ? "chrome105"
      : "safari13",
    // 开发模式不压缩，生产模式压缩
    minify: !process.env.TAURI_ENV_DEBUG,
    sourcemap: !!process.env.TAURI_ENV_DEBUG,
    outDir: "dist",
  },
  resolve: {
    alias: {
      // 路径别名，与 tsconfig.json 和 vitest.config.ts 保持一致
      "@": "/src",
      "@stores": "/src/stores",
      "@components": "/src/components",
      "@utils": "/src/utils",
    },
  },
});