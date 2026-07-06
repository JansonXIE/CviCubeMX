import React, { useEffect } from 'react';
import { MessageSquarePlus, Trash2 } from 'lucide-react';
import { useChatStore } from '../stores/chatStore';

export default function ChatSidebar() {
 const {
 sessions,
 currentSessionId,
 isStreaming,
 loadSessions,
 createSession,
 switchSession,
 deleteSession,
 } = useChatStore();

 useEffect(() => {
 let isCancelled = false;

 const load = async () => {
 if (!isCancelled) {
 await loadSessions();
 }
 };

 void load();

 return () => {
 isCancelled = true;
 };
 }, [loadSessions]);

 const handleCreate = () => {
 if (isStreaming) return;
 void createSession();
 };

 const handleDelete = (e: React.MouseEvent, id: string, title: string) => {
 e.stopPropagation();
 if (isStreaming) return;
 if (window.confirm(`确定删除会话「${title}」吗？`)) {
 void deleteSession(id);
 }
 };

 return (
 <aside className="w-72 flex-shrink-0 bg-slate-900/60 backdrop-blur-xl border-r border-slate-700/50 flex flex-col">
 <div className="p-4 border-b border-slate-800/80">
 <button
 className="w-full flex items-center justify-center gap-2 px-3 py-2.5 rounded-xl text-sm font-bold text-white bg-indigo-600 hover:bg-indigo-500 disabled:opacity-50 disabled:cursor-not-allowed shadow-lg shadow-indigo-500/20 transition-all active:scale-95"
 disabled={isStreaming}
 onClick={handleCreate}
 >
 <MessageSquarePlus size={16} />
 新建对话
 </button>
 </div>

 <div className="flex-1 overflow-y-auto p-3 space-y-2 custom-scrollbar">
 {sessions.length ===0 ? (
 <div className="px-3 py-8 text-center text-sm text-slate-500">暂无对话</div>
 ) : (
 sessions.map((session) => {
 const active = session.id === currentSessionId;
 return (
 <button
 key={session.id}
 className={`group relative w-full overflow-hidden rounded-xl px-3 py-3 text-left transition-all ${
 active
 ? 'bg-indigo-500/10 text-indigo-300 shadow-[inset_4px_0_0_0_rgba(99,102,241,1)]'
 : 'text-slate-400 hover:bg-slate-800/50 hover:text-slate-200'
 }`}
 onClick={() => void switchSession(session.id)}
 disabled={isStreaming && !active}
 >
 <div className="absolute inset-0 bg-gradient-to-r from-transparent via-white/[0.05] to-transparent -translate-x-full group-hover:animate-shimmer pointer-events-none" />
 <div className="relative z-10 flex items-start gap-2">
 <div className="min-w-0 flex-1">
 <div className="truncate text-sm font-semibold">{session.title}</div>
 <div className="mt-1 text-xs text-slate-500">{session.messageCount} 条消息</div>
 </div>
 <span
 role="button"
 tabIndex={0}
 className="opacity-0 group-hover:opacity-100 text-slate-500 hover:text-rose-400 transition-opacity p-1 rounded-lg hover:bg-rose-500/10"
 onClick={(e) => handleDelete(e, session.id, session.title)}
 onKeyDown={(e) => {
 if (e.key === 'Enter' || e.key === ' ') {
 handleDelete(e as unknown as React.MouseEvent, session.id, session.title);
 }
 }}
 aria-label="删除会话"
 >
 <Trash2 size={14} />
 </span>
 </div>
 </button>
 );
 })
 )}
 </div>
 </aside>
 );
}
