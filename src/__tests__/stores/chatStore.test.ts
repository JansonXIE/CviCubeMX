// ============================================================
// CviCubeMX - AI Chat 智能问答功能验证测试 (chatStore)
// ============================================================
// 在纯 vitest/jsdom 环境下验证前端聊天状态机逻辑，Tauri 的
// invoke / listen 被 mock，通过捕获的 'ai-chunk' 回调模拟 Rust 端
// 的 SSE 流式推送。覆盖:
//   - sendMessage 基本流程 (追加消息 / 调用 send_ai_message)
//   - 流式 chunk 追加与 currentAIResponse 累积
//   - 流式过程中 isMarkdown 实时生效 (标题/粗体边流边渲染)
//   - done 事件定稿
//   - 监听器只注册一次 (React.StrictMode 双调用去重回归)
//   - isStreaming 并发保护
//   - invoke 抛错处理 (含 HTTP 400 缺少授权参数场景)
//   - saveConfig / loadConfig / clearMessages / clearError

import { describe, it, expect, vi, beforeEach } from 'vitest';
import type { AiApiConfig, ChatMessage } from '../../stores/chatStore';

// ---- Tauri API mock (用 vi.hoisted 保证在 vi.mock 工厂前初始化) ----
const { mockInvoke, mockListen, mockUnlisten } = vi.hoisted(() => ({
  mockInvoke: vi.fn(),
  mockListen: vi.fn(),
  mockUnlisten: vi.fn(),
}));

vi.mock('@tauri-apps/api/core', () => ({
  invoke: (...args: unknown[]) => mockInvoke(...args),
}));

vi.mock('@tauri-apps/api/event', () => ({
  listen: (...args: unknown[]) => mockListen(...args),
}));

// 收集所有注册到 'ai-chunk' 的回调。真实 Tauri 事件会广播给每个监听器，
// 所以若 chatStore 误注册两次，emitChunk 会触发两次追加 → 内容重复 (回归可捕获)。
type ChunkEvent = { payload: { content: string; done: boolean } };
let chunkListeners: Array<(event: ChunkEvent) => void> = [];

/** 重新加载 chatStore 模块，重置模块级 listenerPromise 守卫与 store 单例状态 */
async function loadFreshStore() {
  vi.resetModules();
  chunkListeners = [];
  const mod = await import('../../stores/chatStore');
  return mod.useChatStore;
}

/** 模拟 Rust 端推送一个 ai-chunk 事件，广播给所有已注册监听器 */
function emitChunk(content: string, done = false) {
  if (chunkListeners.length === 0) {
    throw new Error('尚无监听器注册，请先调用 sendMessage / initListener');
  }
  for (const cb of chunkListeners) {
    cb({ payload: { content, done } });
  }
}

/** 取最后一条消息 */
const lastMsg = (msgs: ChatMessage[]) => msgs[msgs.length - 1];

beforeEach(() => {
  mockInvoke.mockReset();
  mockListen.mockReset();
  mockUnlisten.mockReset();
  chunkListeners = [];

  // listen: 捕获回调并返回 unlisten
  mockListen.mockImplementation(async (_event: string, cb: (e: ChunkEvent) => void) => {
    chunkListeners.push(cb);
    return mockUnlisten;
  });
  // invoke: 默认成功 (返回 undefined，模拟 send_ai_message 完成)
  mockInvoke.mockResolvedValue(undefined);
});

describe('AI Chat 智能问答 (chatStore)', () => {
  // === AC-T1: sendMessage 基本流程 ===
  describe('AC-T1: sendMessage 基本流程', () => {
    it('应追加用户消息与 AI 占位消息，并进入流式状态', async () => {
      const useChatStore = await loadFreshStore();
      await useChatStore.getState().sendMessage('你好');

      const state = useChatStore.getState();
      expect(state.messages).toHaveLength(2);
      expect(state.messages[0]).toMatchObject({ role: 'user', content: '你好' });
      expect(state.messages[1]).toMatchObject({ role: 'assistant', content: '' });
      expect(state.isStreaming).toBe(true);
      expect(state.error).toBeNull();
    });

    it('应以 message + config 调用 send_ai_message 命令', async () => {
      const useChatStore = await loadFreshStore();
      await useChatStore.getState().sendMessage('你好');

      expect(mockInvoke).toHaveBeenCalledWith('send_ai_message', {
        message: '你好',
        config: useChatStore.getState().aiConfig,
      });
    });
  });

  // === AC-T2: 流式 chunk 追加 ===
  describe('AC-T2: 流式 chunk 追加', () => {
    it('多个 chunk 应按顺序追加到最后一条 AI 消息', async () => {
      const useChatStore = await loadFreshStore();
      await useChatStore.getState().sendMessage('hi');

      emitChunk('你好');
      emitChunk('，');
      emitChunk('世界');

      const state = useChatStore.getState();
      expect(lastMsg(state.messages).content).toBe('你好，世界');
      expect(state.currentAIResponse).toBe('你好，世界');
      expect(state.isStreaming).toBe(true); // 尚未 done
    });
  });

  // === AC-T3: 监听器去重 (StrictMode 双调用回归) ===
  describe('AC-T3: 监听器只注册一次', () => {
    it('并发多次 initListener 只应调用 listen 一次', async () => {
      const useChatStore = await loadFreshStore();
      const { initListener } = useChatStore.getState();

      await Promise.all([initListener(), initListener(), initListener()]);

      expect(mockListen).toHaveBeenCalledTimes(1);
      expect(chunkListeners).toHaveLength(1);
    });

    it('重复初始化后单个 chunk 不应被重复追加 (无 “你好你好” 交错)', async () => {
      const useChatStore = await loadFreshStore();
      const store = useChatStore.getState();
      await store.initListener();
      await store.initListener();
      await store.sendMessage('hi');

      emitChunk('你好');

      // 只注册了一个监听器 → 内容恰为 '你好' 而非 '你好你好'
      expect(lastMsg(useChatStore.getState().messages).content).toBe('你好');
    });
  });

  // === AC-T4: 流式 markdown 实时渲染 ===
  describe('AC-T4: 流式过程中 isMarkdown 实时生效', () => {
    it('检测到 markdown 标记前为 false，标记出现后立即为 true (无需等 done)', async () => {
      const useChatStore = await loadFreshStore();
      await useChatStore.getState().sendMessage('讲一下');

      emitChunk('这是');
      emitChunk('回答');
      expect(lastMsg(useChatStore.getState().messages).isMarkdown).toBe(false);

      // 出现粗体标记 **加粗**
      emitChunk('，重点是 **加粗**');
      const state = useChatStore.getState();
      expect(lastMsg(state.messages).isMarkdown).toBe(true);
      expect(state.isStreaming).toBe(true); // 仍在流式中即已切换为 markdown
    });

    it('以标题开头流式输出时，标题标记到达即刻置 isMarkdown 为 true', async () => {
      const useChatStore = await loadFreshStore();
      await useChatStore.getState().sendMessage('讲个标题');

      emitChunk('# ');
      expect(lastMsg(useChatStore.getState().messages).isMarkdown).toBe(true);

      emitChunk('大标题');
      const aiMsg = lastMsg(useChatStore.getState().messages);
      expect(aiMsg.content).toBe('# 大标题');
      expect(aiMsg.isMarkdown).toBe(true);
    });
  });

  // === AC-T5: done 事件定稿 ===
  describe('AC-T5: done 事件定稿', () => {
    it('done 应结束流式、清空 currentAIResponse 并按最终内容重算 isMarkdown', async () => {
      const useChatStore = await loadFreshStore();
      await useChatStore.getState().sendMessage('hi');

      emitChunk('## 标题\n正文');
      emitChunk('', true); // done

      const state = useChatStore.getState();
      expect(state.isStreaming).toBe(false);
      expect(state.currentAIResponse).toBe('');
      const aiMsg = lastMsg(state.messages);
      expect(aiMsg.content).toBe('## 标题\n正文');
      expect(aiMsg.isMarkdown).toBe(true);
    });
  });

  // === AC-T6: isStreaming 并发保护 ===
  describe('AC-T6: 流式期间的并发保护', () => {
    it('流式进行中再次 sendMessage 应被忽略', async () => {
      const useChatStore = await loadFreshStore();
      await useChatStore.getState().sendMessage('第一条');
      expect(useChatStore.getState().isStreaming).toBe(true);

      mockInvoke.mockClear();
      await useChatStore.getState().sendMessage('第二条');

      expect(mockInvoke).not.toHaveBeenCalled();
      expect(useChatStore.getState().messages).toHaveLength(2);
    });
  });

  // === AC-T7: 错误处理 (含 HTTP 400 缺少授权参数) ===
  describe('AC-T7: 错误处理', () => {
    it('send_ai_message 抛错应结束流式、设置 error 并写入最后一条 AI 消息', async () => {
      const useChatStore = await loadFreshStore();
      const apiErr = 'API 错误 (HTTP 400): 缺少授权参数';
      mockInvoke.mockRejectedValueOnce(apiErr);

      await useChatStore.getState().sendMessage('你好');

      const state = useChatStore.getState();
      expect(state.isStreaming).toBe(false);
      expect(state.currentAIResponse).toBe('');
      expect(state.error).toBe(apiErr);
      expect(lastMsg(state.messages).content).toBe(`错误: ${apiErr}`);
    });
  });

  // === AC-T8: 配置读写 ===
  describe('AC-T8: 配置保存与加载', () => {
    it('saveConfig 应调用 save_ai_config 并更新 aiConfig', async () => {
      const useChatStore = await loadFreshStore();
      const cfg: AiApiConfig = { api_key: 'k-123', base_url: 'https://x', model: 'DeepSeek-V4-Pro' };

      await useChatStore.getState().saveConfig(cfg);

      expect(mockInvoke).toHaveBeenCalledWith('save_ai_config', { config: cfg });
      expect(useChatStore.getState().aiConfig).toEqual(cfg);
    });

    it('loadConfig 应从 load_ai_config 读取并写入 aiConfig', async () => {
      const useChatStore = await loadFreshStore();
      const cfg: AiApiConfig = { api_key: 'loaded', base_url: 'https://y', model: 'm' };
      mockInvoke.mockResolvedValueOnce(cfg);

      await useChatStore.getState().loadConfig();

      expect(mockInvoke).toHaveBeenCalledWith('load_ai_config');
      expect(useChatStore.getState().aiConfig).toEqual(cfg);
    });

    it('loadConfig 失败时应设置 error', async () => {
      const useChatStore = await loadFreshStore();
      mockInvoke.mockRejectedValueOnce('读取配置文件失败');

      await useChatStore.getState().loadConfig();

      expect(useChatStore.getState().error).toBe('读取配置文件失败');
    });
  });

  // === AC-T9: 清理动作 ===
  describe('AC-T9: clearMessages / clearError', () => {
    it('clearMessages 应清空消息、currentAIResponse 与 error', async () => {
      const useChatStore = await loadFreshStore();
      await useChatStore.getState().sendMessage('hi');
      emitChunk('部分内容');

      useChatStore.getState().clearMessages();

      const state = useChatStore.getState();
      expect(state.messages).toHaveLength(0);
      expect(state.currentAIResponse).toBe('');
      expect(state.error).toBeNull();
    });

    it('clearError 应仅清除 error', async () => {
      const useChatStore = await loadFreshStore();
      mockInvoke.mockRejectedValueOnce('boom');
      await useChatStore.getState().sendMessage('hi');
      expect(useChatStore.getState().error).toBe('boom');

      useChatStore.getState().clearError();

      expect(useChatStore.getState().error).toBeNull();
    });
  });
});
