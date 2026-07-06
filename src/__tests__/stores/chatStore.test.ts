// ============================================================
// CviCubeMX - AI Chat 智能问答功能验证测试 (chatStore)
// ============================================================
// 在纯 vitest/jsdom 环境下验证前端聊天状态机逻辑，Tauri 的
// invoke / listen 被 mock，通过捕获的 'ai-chunk' 回调模拟 Rust端
// 的 SSE 流式推送。覆盖消息流式状态机、会话生命周期、持久化与配置读写。

import { describe, it, expect, vi, beforeEach } from 'vitest';
import type { AiApiConfig, ChatMessage, ChatSession, ChatSessionMeta } from '../../stores/chatStore';

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

// 收集所有注册到 'ai-chunk' 的回调。真实 Tauri事件会广播给每个监听器，
// 所以若 chatStore误注册两次，emitChunk 会触发两次追加 → 内容重复 (回归可捕获)。
type ChunkEvent = { payload: { content: string; done: boolean } };
let chunkListeners: Array<(event: ChunkEvent) => void> = [];
let sessionCounter =0;
let sessionsDb: ChatSession[] = [];
let commandRejects = new Map<string, unknown>();

/**重新加载 chatStore 模块，重置模块级 listenerPromise 守卫与 store 单例状态 */
async function loadFreshStore() {
 vi.resetModules();
 chunkListeners = [];
 const mod = await import('../../stores/chatStore');
 return mod.useChatStore;
}

/** 模拟 Rust端推送一个 ai-chunk事件，广播给所有已注册监听器 */
function emitChunk(content: string, done = false) {
 if (chunkListeners.length ===0) {
 throw new Error('尚无监听器注册，请先调用 sendMessage / initListener');
 }
 for (const cb of chunkListeners) {
 cb({ payload: { content, done } });
 }
}

/**取最后一条消息 */
const lastMsg = (msgs: ChatMessage[]) => msgs[msgs.length -1];
const flushPromises = async () => {
 await Promise.resolve();
 await Promise.resolve();
};

const makeSession = (overrides: Partial<ChatSession> = {}): ChatSession => ({
 id: `session-${++sessionCounter}`,
 title: '新对话',
 messages: [],
 createdAt: '2026-07-06T00:00:00.000Z',
 updatedAt: '2026-07-06T00:00:00.000Z',
 ...overrides,
});

const toMeta = (session: ChatSession): ChatSessionMeta => ({
 id: session.id,
 title: session.title,
 messageCount: session.messages.length,
 createdAt: session.createdAt,
 updatedAt: session.updatedAt,
});

function rejectCommand(command: string, error: unknown) {
 commandRejects.set(command, error);
}

beforeEach(() => {
 mockInvoke.mockReset();
 mockListen.mockReset();
 mockUnlisten.mockReset();
 chunkListeners = [];
 sessionCounter =0;
 sessionsDb = [];
 commandRejects = new Map();

 // listen: 捕获回调并返回 unlisten
 mockListen.mockImplementation(async (_event: string, cb: (e: ChunkEvent) => void) => {
 chunkListeners.push(cb);
 return mockUnlisten;
 });

 // invoke: 按命令路由，模拟 Rust端会话 JSON 持久化命令
 mockInvoke.mockImplementation(async (command: string, args?: Record<string, unknown>) => {
 if (commandRejects.has(command)) {
 throw commandRejects.get(command);
 }

 switch (command) {
 case 'list_chat_sessions':
 return sessionsDb.map(toMeta).sort((a, b) => b.updatedAt.localeCompare(a.updatedAt));
 case 'create_chat_session': {
 const session = makeSession();
 sessionsDb.unshift(session);
 return session;
 }
 case 'get_chat_session': {
 const id = args?.sessionId as string;
 const session = sessionsDb.find((item) => item.id === id);
 if (!session) throw `会话不存在: ${id}`;
 return session;
 }
 case 'save_chat_session': {
 const session = args?.session as ChatSession;
 sessionsDb = [session, ...sessionsDb.filter((item) => item.id !== session.id)];
 return undefined;
 }
 case 'delete_chat_session': {
 const id = args?.sessionId as string;
 sessionsDb = sessionsDb.filter((item) => item.id !== id);
 return undefined;
 }
 case 'send_ai_message':
 case 'save_ai_config':
 return undefined;
 case 'load_ai_config':
 return { api_key: '', base_url: 'https://www.sophnet.com/api/open-apis/v1', model: '' };
 default:
 return undefined;
 }
 });
});

describe('AI Chat 智能问答 (chatStore)', () => {
 // === AC-T1: sendMessage 基本流程 ===
 describe('AC-T1: sendMessage 基本流程', () => {
 it('应自动创建会话、追加用户消息与 AI 占位消息，并进入流式状态', async () => {
 const useChatStore = await loadFreshStore();
 await useChatStore.getState().sendMessage('你好');

 const state = useChatStore.getState();
 expect(state.currentSessionId).toBeTruthy();
 expect(state.sessions[0]).toMatchObject({ title: '你好', messageCount:0 });
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

 // === AC-T2: 流式 chunk追加 ===
 describe('AC-T2: 流式 chunk追加', () => {
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

 // === AC-T3:监听器去重 (StrictMode 双调用回归) ===
 describe('AC-T3:监听器只注册一次', () => {
 it('并发多次 initListener只应调用 listen 一次', async () => {
 const useChatStore = await loadFreshStore();
 const { initListener } = useChatStore.getState();

 await Promise.all([initListener(), initListener(), initListener()]);

 expect(mockListen).toHaveBeenCalledTimes(1);
 expect(chunkListeners).toHaveLength(1);
 });

 it('重复初始化后单个 chunk 不应被重复追加 (无 “你好你好”交错)', async () => {
 const useChatStore = await loadFreshStore();
 const store = useChatStore.getState();
 await store.initListener();
 await store.initListener();
 await store.sendMessage('hi');

 emitChunk('你好');

 //只注册了一个监听器 → 内容恰为 '你好' 而非 '你好你好'
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
 expect(state.isStreaming).toBe(true); //仍在流式中即已切换为 markdown
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

 // === AC-T5: done事件定稿 ===
 describe('AC-T5: done事件定稿', () => {
 it('done 应结束流式、清空 currentAIResponse、保存会话并按最终内容重算 isMarkdown', async () => {
 const useChatStore = await loadFreshStore();
 await useChatStore.getState().sendMessage('hi');

 emitChunk('## 标题\n正文');
 emitChunk('', true); // done
 await flushPromises();

 const state = useChatStore.getState();
 expect(state.isStreaming).toBe(false);
 expect(state.currentAIResponse).toBe('');
 const aiMsg = lastMsg(state.messages);
 expect(aiMsg.content).toBe('## 标题\n正文');
 expect(aiMsg.isMarkdown).toBe(true);
 expect(mockInvoke).toHaveBeenCalledWith(
 'save_chat_session',
 expect.objectContaining({
 session: expect.objectContaining({ messages: state.messages }),
 })
 );
 expect(mockInvoke).toHaveBeenCalledWith('list_chat_sessions');
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

 // === AC-T7: 错误处理 (含 HTTP400 缺少授权参数) ===
 describe('AC-T7: 错误处理', () => {
 it('send_ai_message 抛错应结束流式、设置 error 并写入最后一条 AI 消息', async () => {
 const useChatStore = await loadFreshStore();
 const apiErr = 'API 错误 (HTTP400): 缺少授权参数';
 rejectCommand('send_ai_message', apiErr);

 await useChatStore.getState().sendMessage('你好');
 await flushPromises();

 const state = useChatStore.getState();
 expect(state.isStreaming).toBe(false);
 expect(state.currentAIResponse).toBe('');
 expect(state.error).toBe(apiErr);
 expect(lastMsg(state.messages).content).toBe(`错误: ${apiErr}`);
 expect(mockInvoke).toHaveBeenCalledWith(
 'save_chat_session',
 expect.objectContaining({ session: expect.objectContaining({ messages: state.messages }) })
 );
 });
 });

 // === AC-T8: 配置读写 ===
 describe('AC-T8: 配置保存与加载', () => {
 it('saveConfig 应调用 save_ai_config 并更新 aiConfig', async () => {
 const useChatStore = await loadFreshStore();
 const cfg: AiApiConfig = { api_key: 'k-123', base_url: 'https://x', model: 'DeepSeek-V4-Pro' };

 await useChatStore.getState().saveConfig(cfg);

 expect(mockInvoke).toHaveBeenCalledWith('save_ai_config', { config: cfg });
 expect(useChatStore.getState().aiConfig).toEqual({ ...cfg, api_key: '' });
 });

 it('loadConfig 应从 load_ai_config读取并写入 aiConfig', async () => {
 const useChatStore = await loadFreshStore();
 const cfg: AiApiConfig = { api_key: 'loaded', base_url: 'https://y', model: 'm' };
 mockInvoke.mockImplementationOnce(async () => cfg);

 await useChatStore.getState().loadConfig();

 expect(mockInvoke).toHaveBeenCalledWith('load_ai_config');
 expect(useChatStore.getState().aiConfig).toEqual({ ...cfg, api_key: '' });
 });

 it('loadConfig失败时应设置 error', async () => {
 const useChatStore = await loadFreshStore();
 rejectCommand('load_ai_config', '读取配置文件失败');

 await useChatStore.getState().loadConfig();

 expect(useChatStore.getState().error).toBe('读取配置文件失败');
 });
 });

 // === AC-T9: 清理动作 ===
 describe('AC-T9: clearMessages / clearError', () => {
 it('clearMessages 应清空消息、currentAIResponse 与 error，并保存当前会话', async () => {
 const useChatStore = await loadFreshStore();
 await useChatStore.getState().sendMessage('hi');
 emitChunk('部分内容');
 emitChunk('', true);
 await flushPromises();
 mockInvoke.mockClear();

 useChatStore.getState().clearMessages();
 await flushPromises();

 const state = useChatStore.getState();
 expect(state.messages).toHaveLength(0);
 expect(state.currentAIResponse).toBe('');
 expect(state.error).toBeNull();
 expect(mockInvoke).toHaveBeenCalledWith(
 'save_chat_session',
 expect.objectContaining({ session: expect.objectContaining({ messages: [] }) })
 );
 });

 it('clearError 应仅清除 error', async () => {
 const useChatStore = await loadFreshStore();
 rejectCommand('send_ai_message', 'boom');
 await useChatStore.getState().sendMessage('hi');
 expect(useChatStore.getState().error).toBe('boom');

 useChatStore.getState().clearError();

 expect(useChatStore.getState().error).toBeNull();
 });
 });

 // === AC-T10: 会话生命周期 ===
 describe('AC-T10: 会话生命周期', () => {
 it('loadSessions 应加载列表，并在无当前会话时切换到最新会话', async () => {
 const useChatStore = await loadFreshStore();
 const session = makeSession({ id: 'saved-1', title: '历史会话', messages: [{ id: 'm1', role: 'user', content: '旧消息', timestamp:1, isMarkdown: false }] });
 sessionsDb = [session];

 await useChatStore.getState().loadSessions();

 expect(useChatStore.getState().sessions).toEqual([toMeta(session)]);
 expect(useChatStore.getState().currentSessionId).toBe('saved-1');
 expect(useChatStore.getState().messages).toEqual(session.messages);
 });

 it('createSession 应创建并切换到新会话', async () => {
 const useChatStore = await loadFreshStore();

 const session = await useChatStore.getState().createSession();

 expect(mockInvoke).toHaveBeenCalledWith('create_chat_session');
 expect(useChatStore.getState().currentSessionId).toBe(session.id);
 expect(useChatStore.getState().messages).toEqual([]);
 expect(useChatStore.getState().sessions[0]).toMatchObject({ id: session.id });
 });

 it('switchSession 应加载会话详情并替换当前消息', async () => {
 const useChatStore = await loadFreshStore();
 const session = makeSession({ id: 's1', messages: [{ id: 'u1', role: 'user', content: 'hello', timestamp:1, isMarkdown: false }] });
 sessionsDb = [session];

 await useChatStore.getState().switchSession('s1');

 expect(mockInvoke).toHaveBeenCalledWith('get_chat_session', { sessionId: 's1' });
 expect(useChatStore.getState().currentSessionId).toBe('s1');
 expect(useChatStore.getState().messages).toEqual(session.messages);
 });

 it('deleteSession 删除当前会话后应清空状态', async () => {
 const useChatStore = await loadFreshStore();
 const session = await useChatStore.getState().createSession();
 await flushPromises();

 await useChatStore.getState().deleteSession(session.id);

 expect(mockInvoke).toHaveBeenCalledWith('delete_chat_session', { sessionId: session.id });
 expect(useChatStore.getState().currentSessionId).toBeNull();
 expect(useChatStore.getState().messages).toEqual([]);
 expect(useChatStore.getState().sessions).toEqual([]);
 });
 });
});
