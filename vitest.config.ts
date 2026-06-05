import { defineConfig } from 'vitest/config';
import react from '@vitejs/plugin-react';

export default defineConfig({
  plugins: [react()],
  test: {
    // 使用 jsdom 环境模拟浏览器 DOM
    environment: 'jsdom',

    // 全局设置
    globals: true,

    // 测试文件匹配模式
    include: ['src/__tests__/**/*.test.{ts,tsx}'],

    // 设置文件
    setupFiles: ['src/__tests__/setup.ts'],

    // 覆盖率配置
    coverage: {
      provider: 'v8',
      include: ['src/**/*.{ts,tsx}'],
      exclude: ['src/__tests__/**'],
    },
  },
  resolve: {
    alias: {
      // 路径别名，与 vite.config.ts 和 tsconfig.json 一致
      '@': '/src',
      '@stores': '/src/stores',
      '@components': '/src/components',
      '@utils': '/src/utils',
    },
  },
});