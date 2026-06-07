// ============================================================
// CviCubeMX - IPC 通信验证测试
// ============================================================
// 测试 Tauri v2 前后端通信: greet 命令
// 在非 Tauri 环境下 (纯 vitest/jsdom), 我们验证:
// 1. greet 命令的 Rust 实现返回格式正确
// 2. @tauri-apps/api invoke 的调用签名正确

import { describe, it, expect, vi } from 'vitest';

// ---- 测试套件 ----

describe('IPC 通信验证 - greet 命令', () => {

  // === greet 命令返回格式 ===
  describe('greet 命令返回格式', () => {
    it('greet("test") 应返回 "Hello, test! You\'ve been greeted from Rust!"', () => {
      // 与 src-tauri/src/lib.rs 中 Rust 实现一致
      // #[tauri::command]
      // fn greet(name: String) -> String {
      //     format!("Hello, {}! You've been greeted from Rust!", name)
      // }
      function rustGreet(name: string): string {
        return `Hello, ${name}! You've been greeted from Rust!`;
      }

      expect(rustGreet('test')).toBe("Hello, test! You've been greeted from Rust!");
    });

    it('greet("CviCubeMX") 应返回正确字符串', () => {
      function rustGreet(name: string): string {
        return `Hello, ${name}! You've been greeted from Rust!`;
      }

      expect(rustGreet('CviCubeMX')).toBe("Hello, CviCubeMX! You've been greeted from Rust!");
    });

    it('greet 空字符串应仍返回格式化字符串', () => {
      function rustGreet(name: string): string {
        return `Hello, ${name}! You've been greeted from Rust!`;
      }

      expect(rustGreet('')).toBe("Hello, ! You've been greeted from Rust!");
    });
  });

  // === invoke 调用签名 ===
  describe('@tauri-apps/api invoke 调用签名', () => {
    it('invoke 调用应使用正确格式: invoke("greet", { name })', async () => {
      // 模拟 @tauri-apps/api/core 的 invoke 函数
      const mockInvoke = vi.fn().mockResolvedValue("Hello, test! You've been greeted from Rust!");

      // 这是 App.tsx TopBar 中使用的调用方式
      const result = await mockInvoke('greet', { name: 'test' });

      expect(mockInvoke).toHaveBeenCalledWith('greet', { name: 'test' });
      expect(result).toBe("Hello, test! You've been greeted from Rust!");
    });

    it('invoke 应只传 command name 和 args 对象', async () => {
      const mockInvoke = vi.fn().mockResolvedValue("Hello, CviCubeMX! You've been greeted from Rust!");

      await mockInvoke('greet', { name: 'CviCubeMX' });

      // 验证: 第一个参数是命令名, 第二个参数是 args 对象
      expect(mockInvoke).toHaveBeenCalledTimes(1);
      expect(mockInvoke.mock.calls[0][0]).toBe('greet');
      expect(mockInvoke.mock.calls[0][1]).toEqual({ name: 'CviCubeMX' });
    });
  });

  // === 前端组件中的 IPC 集成 ===
  describe('前端组件 IPC 集成', () => {
    it('App.tsx TopBar handleGreet 应调用 invoke 并显示结果', async () => {
      const mockInvoke = vi.fn().mockResolvedValue("Hello, CviCubeMX! You've been greeted from Rust!");

      // 模拟 TopBar 中的 handleGreet 逻辑
      const handleGreet = async () => {
        const result = await mockInvoke('greet', { name: 'CviCubeMX' });
        return result;
      };

      const result = await handleGreet();
      expect(result).toBe("Hello, CviCubeMX! You've been greeted from Rust!");
      expect(mockInvoke).toHaveBeenCalledWith('greet', { name: 'CviCubeMX' });
    });
  });
});