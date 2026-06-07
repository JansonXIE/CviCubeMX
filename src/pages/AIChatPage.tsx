import React from "react";
import ChatPanel from "../components/ChatPanel";
import { MessageSquare } from "lucide-react";

export default function AIChatPage() {
  return (
    <div className="flex flex-col h-full space-y-5">
      {/* 头部配置栏 */}
      <div className="flex flex-wrap items-center justify-between gap-4 bg-slate-900/40 border border-slate-800 p-4 rounded-2xl relative overflow-hidden flex-shrink-0">
        <div className="absolute inset-0 bg-gradient-to-r from-indigo-500/5 to-transparent pointer-events-none" />

        <div className="flex items-center gap-3 relative z-10">
          <div className="bg-indigo-500/10 p-2.5 rounded-xl text-indigo-400 border border-indigo-500/20">
            <MessageSquare size={22} className="animate-pulse" />
          </div>
          <div>
            <h2 className="text-base font-bold text-slate-100">AI 智能问答面板 (Copilot Agent Panel)</h2>
            <p className="text-xs text-slate-500 font-medium">与 CviCubeMX 内置的 AI 开发助手交流，直接通过自然语言获取引脚复用和设备树参数配置建议</p>
          </div>
        </div>
      </div>

      {/* 消息聊天框容器 */}
      <div className="flex-1 bg-slate-900/60 border border-slate-800 rounded-2xl overflow-hidden shadow-xl flex flex-col min-h-[400px]">
        {/* 我们将主聊天组件融入暗色毛玻璃卡片，且消除聊天内部按钮聚焦框 */}
        <div className="flex-1 overflow-hidden rounded-2xl [&_button]:focus:outline-none [&_button]:focus:ring-0 [&_input]:focus:outline-none [&_textarea]:focus:outline-none">
          <ChatPanel />
        </div>
      </div>
    </div>
  );
}
