import { create } from 'zustand';
import { invoke } from '@tauri-apps/api/core';
import { listen, UnlistenFn } from '@tauri-apps/api/event';
import { isMarkdownContent } from '../utils/markdownDetect';

/** AI API 配置 */
export interface AiApiConfig {
 api_key: string;
 base_url: string;
 model: string;
}

/** 聊天消息 */
export interface ChatMessage {
 id: string;
 role: 'user' | 'assistant';
 content: string;
 timestamp: number;
 isMarkdown: boolean;
}

/** 聊天会话 */
export interface ChatSession {
 id: string;
 title: string;
 messages: ChatMessage[];
 createdAt: string;
 updatedAt: string;
}

/** 聊天会话列表元数据 */
export interface ChatSessionMeta {
 id: string;
 title: string;
 messageCount: number;
 createdAt: string;
 updatedAt: string;
}

/** AI chunk 推送 payload (来自 Rust端 Tauri Event) */
interface AiChunkPayload {
 content: string;
 done: boolean;
}

/** ChatStore 状态 */
interface ChatState {
 sessions: ChatSessionMeta[];
 currentSessionId: string | null;
 messages: ChatMessage[];
 aiConfig: AiApiConfig;
 isStreaming: boolean;
 currentAIResponse: string;
 error: string | null;

 // Actions
 loadSessions: () => Promise<void>;
 createSession: () => Promise<ChatSession>;
 switchSession: (id: string) => Promise<void>;
 deleteSession: (id: string) => Promise<void>;
 renameSession: (id: string, title: string) => Promise<void>;
 sendMessage: (message: string) => Promise<void>;
 saveConfig: (config: AiApiConfig) => Promise<void>;
 loadConfig: () => Promise<void>;
 clearMessages: () => void;
 clearError: () => void;
 initListener: () => Promise<UnlistenFn>;
}

// 缓存 listen() 返回的 Promise 而非解析后的 UnlistenFn：作为同步守卫，
// 保证监听器只注册一次（详见 initListener 内注释）。
let listenerPromise: Promise<UnlistenFn> | null = null;

const DEFAULT_SESSION_TITLE = '新对话';

const makeSessionTitle = (message: string) => {
 const title = message.trim().replace(/\s+/g, ' ').slice(0,32);
 return title || DEFAULT_SESSION_TITLE;
};

const updateSessionMeta = (sessions: ChatSessionMeta[], session: ChatSession): ChatSessionMeta[] => {
 const meta: ChatSessionMeta = {
 id: session.id,
 title: session.title,
 messageCount: session.messages.length,
 createdAt: session.createdAt,
 updatedAt: session.updatedAt,
 };
 const filtered = sessions.filter((item) => item.id !== session.id);
 return [meta, ...filtered].sort((a, b) => b.updatedAt.localeCompare(a.updatedAt));
};

export const useChatStore = create<ChatState>((set, get) => ({
 sessions: [],
 currentSessionId: null,
 messages: [],
 // 敏感信息不硬编码在前端。启动时通过 loadConfig() 从 Rust端加载
 // (Rust端的值来自编译时注入的环境变量 AI_API_KEY / AI_BASE_URL / AI_MODEL)。
 aiConfig: {
 api_key: '',
 base_url: 'https://www.sophnet.com/api/open-apis/v1',
 model: '',
 },
 isStreaming: false,
 currentAIResponse: '',
 error: null,

 /** 初始化 Tauri Event listener，监听 ai-chunk事件 */
 initListener: async () => {
 // 同步缓存 listen() 的 Promise：React.StrictMode 在开发环境会双调用挂载 effect，
 // 若守卫用的是 await之后才赋值的 unlistenFn，两次调用会都看到 null 而各自注册
 // 一个 'ai-chunk'监听器，导致同一 chunk 被追加两次（输出重复交错）。
 // 在任何 await之前就把 Promise 存入 listenerPromise，可保证只注册一次。
 if (listenerPromise) {
 return listenerPromise;
 }

 listenerPromise = listen<AiChunkPayload>('ai-chunk', (event) => {
 const payload = event.payload;

 if (payload.done) {
 // 流式响应完成
 const { currentAIResponse, messages, currentSessionId, sessions } = get();
 const finalContent = currentAIResponse;
 const lastMsgIdx = messages.length -1;
 let finalMessages = messages;

 if (finalContent && lastMsgIdx >=0 && messages[lastMsgIdx].role === 'assistant') {
 // 更新最后一条 AI 消息为完整内容
 finalMessages = messages.map((msg, i) =>
 i === lastMsgIdx
 ? {
 ...msg,
 content: finalContent,
 isMarkdown: isMarkdownContent(finalContent),
 }
 : msg
 );
 }

 set({
 messages: finalMessages,
 currentAIResponse: '',
 isStreaming: false,
 });

 if (currentSessionId) {
 const existing = sessions.find((session) => session.id === currentSessionId);
 const session: ChatSession = {
 id: currentSessionId,
 title: existing?.title || DEFAULT_SESSION_TITLE,
 messages: finalMessages,
 createdAt: existing?.createdAt || new Date().toISOString(),
 updatedAt: new Date().toISOString(),
 };

 void invoke('save_chat_session', { session })
 .then(() => get().loadSessions())
 .catch((err) => set({ error: String(err) }));
 }
 } else {
 // 流式追加内容
 const newContent = get().currentAIResponse + payload.content;
 const { messages } = get();
 const lastMsgIdx = messages.length -1;

 if (lastMsgIdx >=0 && messages[lastMsgIdx].role === 'assistant') {
 // 更新最后一条 AI 消息的显示内容。
 // 每个 chunk 都实时重算 isMarkdown，使标题/列表等在流式过程中即渲染为
 // markdown（ReactMarkdown 可安全渲染未闭合的片段），无需等到 done 才刷新。
 set({
 currentAIResponse: newContent,
 messages: messages.map((msg, i) =>
 i === lastMsgIdx
 ? { ...msg, content: newContent, isMarkdown: isMarkdownContent(newContent) }
 : msg
 ),
 });
 }
 }
 });

 return listenerPromise;
 },

 /** 加载聊天会话列表 */
 loadSessions: async () => {
 try {
 const sessions = await invoke<ChatSessionMeta[]>('list_chat_sessions');
 set({ sessions });

 if (!get().currentSessionId && sessions.length >0) {
 await get().switchSession(sessions[0].id);
 }
 } catch (err) {
 set({ error: String(err) });
 }
 },

 /** 新建聊天会话 */
 createSession: async () => {
 try {
 const session = await invoke<ChatSession>('create_chat_session');
 set((state) => ({
 sessions: updateSessionMeta(state.sessions, session),
 currentSessionId: session.id,
 messages: session.messages,
 currentAIResponse: '',
 error: null,
 }));
 void get().loadSessions();
 return session;
 } catch (err) {
 set({ error: String(err) });
 throw err;
 }
 },

 /** 切换聊天会话 */
 switchSession: async (id: string) => {
 if (get().isStreaming) {
 set({ error: '正在生成回复，请稍后再切换会话' });
 return;
 }

 try {
 const session = await invoke<ChatSession>('get_chat_session', { sessionId: id });
 set({
 currentSessionId: session.id,
 messages: session.messages,
 currentAIResponse: '',
 error: null,
 });
 } catch (err) {
 set({ error: String(err) });
 }
 },

 /** 删除聊天会话 */
 deleteSession: async (id: string) => {
 if (get().isStreaming) {
 set({ error: '正在生成回复，请稍后再删除会话' });
 return;
 }

 try {
 await invoke('delete_chat_session', { sessionId: id });
 const remaining = get().sessions.filter((session) => session.id !== id);
 set({ sessions: remaining });

 if (get().currentSessionId === id) {
 if (remaining.length >0) {
 await get().switchSession(remaining[0].id);
 } else {
 set({ currentSessionId: null, messages: [], currentAIResponse: '', error: null });
 }
 }

 void get().loadSessions();
 } catch (err) {
 set({ error: String(err) });
 }
 },

 /** 重命名聊天会话 */
 renameSession: async (id: string, title: string) => {
 try {
 const currentSession =
 get().currentSessionId === id
 ? {
 id,
 title,
 messages: get().messages,
 createdAt: get().sessions.find((session) => session.id === id)?.createdAt || new Date().toISOString(),
 updatedAt: new Date().toISOString(),
 }
 : await invoke<ChatSession>('get_chat_session', { sessionId: id });

 const session = { ...currentSession, title };
 await invoke('save_chat_session', { session });
 await get().loadSessions();
 } catch (err) {
 set({ error: String(err) });
 }
 },

 /**发送消息到 AI */
 sendMessage: async (message: string) => {
 const state = get();
 if (state.isStreaming) return;

 let sessionId = state.currentSessionId;
 let sessions = state.sessions;
 let baseMessages = state.messages;

 try {
 if (!sessionId) {
 const session = await invoke<ChatSession>('create_chat_session');
 const titledSession = { ...session, title: makeSessionTitle(message) };
 await invoke('save_chat_session', { session: titledSession });
 sessionId = titledSession.id;
 sessions = updateSessionMeta(sessions, titledSession);
 baseMessages = titledSession.messages;
 set({
 sessions,
 currentSessionId: sessionId,
 messages: baseMessages,
 currentAIResponse: '',
 error: null,
 });
 } else {
 const currentMeta = sessions.find((session) => session.id === sessionId);
 if (currentMeta && currentMeta.title === DEFAULT_SESSION_TITLE && baseMessages.length ===0) {
 const titledSession: ChatSession = {
 id: sessionId,
 title: makeSessionTitle(message),
 messages: baseMessages,
 createdAt: currentMeta.createdAt,
 updatedAt: new Date().toISOString(),
 };
 await invoke('save_chat_session', { session: titledSession });
 sessions = updateSessionMeta(sessions, titledSession);
 set({ sessions });
 }
 }
 } catch (err) {
 set({ error: String(err) });
 return;
 }

 // 添加用户消息
 const now = Date.now();
 const userMsg: ChatMessage = {
 id: `user-${now}`,
 role: 'user',
 content: message,
 timestamp: now,
 isMarkdown: isMarkdownContent(message),
 };

 // 添加 AI 消息占位 (正在思考中...)
 const aiMsg: ChatMessage = {
 id: `ai-${now}`,
 role: 'assistant',
 content: '',
 timestamp: now,
 isMarkdown: false,
 };

 set({
 messages: [...baseMessages, userMsg, aiMsg],
 isStreaming: true,
 currentAIResponse: '',
 error: null,
 });

 try {
 // 初始化 listener（如果尚未初始化）
 await get().initListener();

 // 调用 Rust端的 send_ai_message 命令
 await invoke('send_ai_message', {
 message: message,
 config: get().aiConfig,
 });
 } catch (err) {
 // 错误处理
 const errorMsg = String(err);
 const erroredMessages = get().messages.map((msg) =>
 msg.id === aiMsg.id ? { ...msg, content: `错误: ${errorMsg}` } : msg
 );
 set({
 isStreaming: false,
 currentAIResponse: '',
 error: errorMsg,
 messages: erroredMessages,
 });

 if (sessionId) {
 const meta = get().sessions.find((session) => session.id === sessionId);
 const session: ChatSession = {
 id: sessionId,
 title: meta?.title || DEFAULT_SESSION_TITLE,
 messages: erroredMessages,
 createdAt: meta?.createdAt || new Date().toISOString(),
 updatedAt: new Date().toISOString(),
 };
 void invoke('save_chat_session', { session })
 .then(() => get().loadSessions())
 .catch((saveErr) => set({ error: `${errorMsg}; 保存会话失败: ${String(saveErr)}` }));
 }
 }
 },

 /** 保存 AI 配置 */
 saveConfig: async (config: AiApiConfig) => {
 try {
 await invoke('save_ai_config', { config });
 set({ aiConfig: { ...config, api_key: '' } });
 } catch (err) {
 set({ error: String(err) });
 }
 },

 /** 加载 AI 配置 */
 loadConfig: async () => {
 try {
 const config = await invoke<AiApiConfig>('load_ai_config');
 set({ aiConfig: { ...config, api_key: '' } });
 } catch (err) {
 set({ error: String(err) });
 }
 },

 /** 清空当前聊天会话消息 */
 clearMessages: () => {
 const { currentSessionId, sessions } = get();
 set({ messages: [], currentAIResponse: '', error: null });

 if (currentSessionId) {
 const meta = sessions.find((session) => session.id === currentSessionId);
 const session: ChatSession = {
 id: currentSessionId,
 title: meta?.title || DEFAULT_SESSION_TITLE,
 messages: [],
 createdAt: meta?.createdAt || new Date().toISOString(),
 updatedAt: new Date().toISOString(),
 };
 void invoke('save_chat_session', { session })
 .then(() => get().loadSessions())
 .catch((err) => set({ error: String(err) }));
 }
 },

 /** 清除错误 */
 clearError: () => {
 set({ error: null });
 },
}));
