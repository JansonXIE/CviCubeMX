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

/** AI chunk 推送 payload (来自 Rust 端 Tauri Event) */
interface AiChunkPayload {
  content: string;
  done: boolean;
}

/** ChatStore 状态 */
interface ChatState {
  messages: ChatMessage[];
  aiConfig: AiApiConfig;
  isStreaming: boolean;
  currentAIResponse: string;
  error: string | null;

  // Actions
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

export const useChatStore = create<ChatState>((set, get) => ({
  messages: [],
  // 敏感信息不硬编码在前端。启动时通过 loadConfig() 从 Rust 端加载
  // (Rust 端的值来自编译时注入的环境变量 AI_API_KEY / AI_BASE_URL / AI_MODEL)。
  aiConfig: {
    api_key: '',
    base_url: 'https://www.sophnet.com/api/open-apis/v1',
    model: '',
  },
  isStreaming: false,
  currentAIResponse: '',
  error: null,

  /** 初始化 Tauri Event listener，监听 ai-chunk 事件 */
  initListener: async () => {
    // 同步缓存 listen() 的 Promise：React.StrictMode 在开发环境会双调用挂载 effect，
    // 若守卫用的是 await 之后才赋值的 unlistenFn，两次调用会都看到 null 而各自注册
    // 一个 'ai-chunk' 监听器，导致同一 chunk 被追加两次（输出重复交错）。
    // 在任何 await 之前就把 Promise 存入 listenerPromise，可保证只注册一次。
    if (listenerPromise) {
      return listenerPromise;
    }

    listenerPromise = listen<AiChunkPayload>('ai-chunk', (event) => {
      const payload = event.payload;

      if (payload.done) {
        // 流式响应完成
        const { currentAIResponse, messages } = get();
        const finalContent = currentAIResponse;
        if (finalContent) {
          // 更新最后一条 AI 消息为完整内容
          const lastMsgIdx = messages.length - 1;
          if (lastMsgIdx >= 0 && messages[lastMsgIdx].role === 'assistant') {
            set({
              messages: messages.map((msg, i) =>
                i === lastMsgIdx
                  ? {
                      ...msg,
                      content: finalContent,
                      isMarkdown: isMarkdownContent(finalContent),
                    }
                  : msg
              ),
              currentAIResponse: '',
              isStreaming: false,
            });
          }
        } else {
          set({ isStreaming: false, currentAIResponse: '' });
        }
      } else {
        // 流式追加内容
        const newContent = get().currentAIResponse + payload.content;
        const { messages } = get();
        const lastMsgIdx = messages.length - 1;

        if (lastMsgIdx >= 0 && messages[lastMsgIdx].role === 'assistant') {
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

  /** 发送消息到 AI */
  sendMessage: async (message: string) => {
    const state = get();
    if (state.isStreaming) return;

    // 添加用户消息
    const userMsg: ChatMessage = {
      id: `user-${Date.now()}`,
      role: 'user',
      content: message,
      timestamp: Date.now(),
      isMarkdown: isMarkdownContent(message),
    };

    // 添加 AI 消息占位 (正在思考中...)
    const aiMsg: ChatMessage = {
      id: `ai-${Date.now()}`,
      role: 'assistant',
      content: '',
      timestamp: Date.now(),
      isMarkdown: false,
    };

    set({
      messages: [...state.messages, userMsg, aiMsg],
      isStreaming: true,
      currentAIResponse: '',
      error: null,
    });

    try {
      // 初始化 listener（如果尚未初始化）
      await get().initListener();

      // 调用 Rust 端的 send_ai_message 命令
      await invoke('send_ai_message', {
        message: message,
        config: state.aiConfig,
      });
    } catch (err) {
      // 错误处理
      const errorMsg = String(err);
      set({
        isStreaming: false,
        currentAIResponse: '',
        error: errorMsg,
        messages: get().messages.map((msg) =>
          msg.id === aiMsg.id
            ? { ...msg, content: `错误: ${errorMsg}` }
            : msg
        ),
      });
    }
  },

  /** 保存 AI 配置 */
  saveConfig: async (config: AiApiConfig) => {
    try {
      await invoke('save_ai_config', { config });
      set({ aiConfig: config });
    } catch (err) {
      set({ error: String(err) });
    }
  },

  /** 加载 AI 配置 */
  loadConfig: async () => {
    try {
      const config = await invoke<AiApiConfig>('load_ai_config');
      set({ aiConfig: config });
    } catch (err) {
      set({ error: String(err) });
    }
  },

  /** 清空聊天消息 */
  clearMessages: () => {
    set({ messages: [], currentAIResponse: '', error: null });
  },

  /** 清除错误 */
  clearError: () => {
    set({ error: null });
  },
}));