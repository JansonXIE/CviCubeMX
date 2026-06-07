import React, { useState, useRef, useEffect } from 'react';
import ReactMarkdown from 'react-markdown';
import remarkGfm from 'remark-gfm';
import { useChatStore } from '../stores/chatStore';
import type { AiApiConfig } from '../stores/chatStore';

/** ChatPanel - AI 聊天面板组件 */
export default function ChatPanel() {
  const {
    messages,
    aiConfig,
    isStreaming,
    error,
    sendMessage,
    saveConfig,
    loadConfig,
    clearMessages,
    clearError,
    initListener,
  } = useChatStore();

  const [inputValue, setInputValue] = useState('');
  const [showConfig, setShowConfig] = useState(false);
  const [configForm, setConfigForm] = useState<AiApiConfig>(aiConfig);
  const chatEndRef = useRef<HTMLDivElement>(null);

  // 初始化 listener 和加载配置
  useEffect(() => {
    initListener();
    loadConfig();
  }, []);

  // 当 config 加载后更新表单
  useEffect(() => {
    setConfigForm(aiConfig);
  }, [aiConfig]);

  // 滚动到底部
  useEffect(() => {
    chatEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [messages]);

  // 发送消息
  const handleSend = () => {
    const msg = inputValue.trim();
    if (!msg || isStreaming) return;
    setInputValue('');
    sendMessage(msg);
  };

  // 处理键盘事件: Enter 发送, Shift+Enter 换行
  const handleKeyDown = (e: React.KeyboardEvent<HTMLTextAreaElement>) => {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      handleSend();
    }
  };

  // 保存配置
  const handleSaveConfig = () => {
    saveConfig(configForm);
    setShowConfig(false);
  };

  return (
    <div className="flex flex-col h-full bg-gray-50">
      {/* 标题栏 */}
      <div className="flex items-center justify-between px-4 py-3 bg-white border-b shadow-sm">
        <h2 className="text-lg font-semibold text-gray-800">AI 智能问答助手</h2>
        <div className="flex gap-2">
          <button
            className="px-3 py-1 text-sm rounded-lg bg-blue-500 text-white hover:bg-blue-600 focus:outline-none focus:ring-0"
            onClick={() => setShowConfig(!showConfig)}
          >
            配置
          </button>
          <button
            className="px-3 py-1 text-sm rounded-lg bg-gray-400 text-white hover:bg-gray-500 focus:outline-none focus:ring-0"
            onClick={clearMessages}
          >
            清空
          </button>
        </div>
      </div>

      {/* 配置面板 */}
      {showConfig && (
        <div className="p-4 bg-white border-b shadow-sm">
          <h3 className="text-sm font-medium text-gray-600 mb-2">API 配置</h3>
          <div className="grid grid-cols-1 gap-2">
            <input
              className="px-3 py-2 text-sm border rounded-lg focus:border-blue-400 focus:outline-none"
              placeholder="API Key"
              value={configForm.api_key}
              onChange={(e) =>
                setConfigForm({ ...configForm, api_key: e.target.value })
              }
            />
            <input
              className="px-3 py-2 text-sm border rounded-lg focus:border-blue-400 focus:outline-none"
              placeholder="Base URL"
              value={configForm.base_url}
              onChange={(e) =>
                setConfigForm({ ...configForm, base_url: e.target.value })
              }
            />
            <input
              className="px-3 py-2 text-sm border rounded-lg focus:border-blue-400 focus:outline-none"
              placeholder="Model"
              value={configForm.model}
              onChange={(e) =>
                setConfigForm({ ...configForm, model: e.target.value })
              }
            />
          </div>
          <div className="flex gap-2 mt-3">
            <button
              className="px-4 py-1 text-sm rounded-lg bg-green-500 text-white hover:bg-green-600 focus:outline-none focus:ring-0"
              onClick={handleSaveConfig}
            >
              保存
            </button>
            <button
              className="px-4 py-1 text-sm rounded-lg bg-gray-300 text-gray-700 hover:bg-gray-400 focus:outline-none focus:ring-0"
              onClick={() => setShowConfig(false)}
            >
              取消
            </button>
          </div>
        </div>
      )}

      {/* 错误提示 */}
      {error && (
        <div className="px-4 py-2 bg-red-100 text-red-700 text-sm flex items-center justify-between">
          <span>{error}</span>
          <button
            className="text-red-500 hover:text-red-700 font-bold focus:outline-none focus:ring-0"
            onClick={clearError}
          >
            ×
          </button>
        </div>
      )}

      {/* 消息列表 */}
      <div className="flex-1 overflow-y-auto px-4 py-2 space-y-3">
        {messages.length === 0 && (
          <div className="text-center text-gray-400 py-8">
            请输入您的问题，AI 助手将为您解答
          </div>
        )}

        {messages.map((msg) => (
          <div
            key={msg.id}
            className={`flex ${
              msg.role === 'user' ? 'justify-end' : 'justify-start'
            }`}
          >
            <div
              className={`max-w-[70%] px-4 py-3 rounded-2xl shadow-sm ${
                msg.role === 'user'
                  ? 'bg-blue-500 text-white'
                  : 'bg-white text-gray-800 border border-gray-200'
              }`}
            >
              {msg.role === 'assistant' && msg.isMarkdown && msg.content ? (
                <div className="markdown-body">
                  <ReactMarkdown remarkPlugins={[remarkGfm]}>
                    {msg.content}
                  </ReactMarkdown>
                </div>
              ) : (
                <div className="text-sm leading-relaxed whitespace-pre-wrap">
                  {msg.content || (isStreaming && msg.role === 'assistant' ? '正在思考中...' : '')}
                </div>
              )}
            </div>
          </div>
        ))}
        <div ref={chatEndRef} />
      </div>

      {/* 输入区域 */}
      <div className="flex gap-2 px-4 py-3 bg-white border-t">
        <textarea
          className="flex-1 px-3 py-2 border-2 rounded-lg resize-none focus:border-blue-400 focus:outline-none text-sm"
          placeholder="请输入您的问题... (Enter发送 | Shift+Enter换行)"
          rows={1}
          value={inputValue}
          onChange={(e) => setInputValue(e.target.value)}
          onKeyDown={handleKeyDown}
          disabled={isStreaming}
          style={{ minHeight: '40px', maxHeight: '120px' }}
        />
        <button
          className="px-6 py-2 rounded-lg font-semibold text-white disabled:bg-gray-300 bg-blue-500 hover:bg-blue-600 focus:outline-none focus:ring-0"
          disabled={!inputValue.trim() || isStreaming}
          onClick={handleSend}
        >
          {isStreaming ? '等待...' : '发送'}
        </button>
      </div>
    </div>
  );
}