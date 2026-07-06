import React, { useState, useRef, useEffect } from 'react';
import ReactMarkdown from 'react-markdown';
import remarkGfm from 'remark-gfm';
import { useChatStore } from '../stores/chatStore';

/** ChatPanel - AI 聊天面板组件 */
export default function ChatPanel() {
 const {
 messages,
 isStreaming,
 error,
 sendMessage,
 loadConfig,
 clearMessages,
 clearError,
 initListener,
 } = useChatStore();

 const [inputValue, setInputValue] = useState('');
 const messageListRef = useRef<HTMLDivElement>(null);

 // 初始化 listener 和加载配置。配置不在页面展示，仍需从 Rust端加载供发送时使用。
 useEffect(() => {
 initListener();
 loadConfig();
 }, []);

 // 滚动到底部：仅滚动消息列表容器本身，避免 scrollIntoView 连带滚动
 // 外层 overflow-hidden祖先（overflow:hidden仍可被程序化滚动），
 // 否则进入本页时整页会被顶上去。
 useEffect(() => {
 const el = messageListRef.current;
 if (el) {
 el.scrollTop = el.scrollHeight;
 }
 }, [messages]);

 //发送消息
 const handleSend = () => {
 const msg = inputValue.trim();
 if (!msg || isStreaming) return;
 setInputValue('');
 sendMessage(msg);
 };

 //处理键盘事件: Enter发送, Shift+Enter 换行
 const handleKeyDown = (e: React.KeyboardEvent<HTMLTextAreaElement>) => {
 if (e.key === 'Enter' && !e.shiftKey) {
 e.preventDefault();
 handleSend();
 }
 };

 return (
 <div className="flex flex-col h-full bg-slate-950/30 text-slate-100">
 {/* 标题栏 */}
 <div className="flex items-center justify-between px-4 py-3 bg-slate-900/70 backdrop-blur-lg border-b border-slate-800 shadow-xl">
 <h2 className="text-lg font-semibold text-slate-100">AI 智能问答助手</h2>
 <button
 className="px-3 py-1.5 text-sm rounded-xl bg-slate-800 text-slate-200 hover:bg-slate-700 border border-slate-700 transition-all focus:outline-none focus:ring-0"
 onClick={clearMessages}
 >
 清空
 </button>
 </div>

 {/* 错误提示 */}
 {error && (
 <div className="px-4 py-2 bg-rose-950/40 border-b border-rose-900/40 text-rose-300 text-sm flex items-center justify-between">
 <span>{error}</span>
 <button
 className="text-rose-400 hover:text-rose-200 font-bold focus:outline-none focus:ring-0"
 onClick={clearError}
 >
 ×
 </button>
 </div>
 )}

 {/* 消息列表 */}
 <div ref={messageListRef} className="flex-1 overflow-y-auto px-4 py-3 space-y-3 custom-scrollbar">
 {messages.length ===0 && (
 <div className="text-center text-slate-500 py-8">
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
 className={`max-w-[70%] min-w-0 px-4 py-3 rounded-2xl shadow-sm overflow-hidden break-words ${
 msg.role === 'user'
 ? 'bg-indigo-600 text-white shadow-indigo-500/10'
 : 'bg-slate-800/70 text-slate-200 border border-slate-700/50'
 }`}
 >
 {msg.role === 'assistant' && msg.isMarkdown && msg.content ? (
 <div className="markdown-body markdown-body-dark">
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
 </div>

 {/* 输入区域 */}
 <div className="flex gap-2 px-4 py-3 bg-slate-950/50 border-t border-slate-800">
 <textarea
 className="flex-1 px-3 py-2 border border-slate-800 rounded-xl resize-none focus:border-indigo-500 focus:outline-none text-sm bg-slate-950/80 text-slate-200 placeholder-slate-600"
 placeholder="请输入您的问题... (Enter发送 | Shift+Enter换行)"
 rows={1}
 value={inputValue}
 onChange={(e) => setInputValue(e.target.value)}
 onKeyDown={handleKeyDown}
 disabled={isStreaming}
 style={{ minHeight: '40px', maxHeight: '120px' }}
 />
 <button
 className="px-6 py-2 rounded-xl font-semibold text-white disabled:opacity-50 disabled:cursor-not-allowed bg-indigo-600 hover:bg-indigo-500 shadow-lg shadow-indigo-500/20 transition-all focus:outline-none focus:ring-0"
 disabled={!inputValue.trim() || isStreaming}
 onClick={handleSend}
 >
 {isStreaming ? '等待...' : '发送'}
 </button>
 </div>
 </div>
 );
}
