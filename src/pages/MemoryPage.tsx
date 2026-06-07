import React from "react";
import MemoryTable from "../components/MemoryTable";
import { Cpu, ShieldCheck } from "lucide-react";
import { useMemoryStore } from "../stores/memoryStore";

// 十六进制格式化
function formatHex(val: number): string {
  return "0x" + val.toString(16).toUpperCase().padStart(8, "0");
}

export default function MemoryPage() {
  const { regions } = useMemoryStore();

  // 渲染一个高级的内存段分布概念图
  // 基准范围 0x80000000 到 0x90000000 (256MB)
  const baseAddr = 0x80000000;
  const totalSpan = 0x10000000; // 256MB

  return (
    <div className="flex flex-col h-full space-y-5">
      {/* 头部配置栏 */}
      <div className="flex flex-wrap items-center justify-between gap-4 bg-slate-900/40 border border-slate-800 p-4 rounded-2xl relative overflow-hidden">
        <div className="absolute inset-0 bg-gradient-to-r from-indigo-500/5 to-transparent pointer-events-none" />

        <div className="flex items-center gap-3 relative z-10">
          <div className="bg-indigo-500/10 p-2.5 rounded-xl text-indigo-400 border border-indigo-500/20">
            <Cpu size={22} className="animate-pulse" />
          </div>
          <div>
            <h2 className="text-base font-bold text-slate-100">物理内存映射配置 (System Memory Allocation)</h2>
            <p className="text-xs text-slate-500 font-medium">配置 Linux 内核、RTOS、ION 内存等多核共享内存段的起始物理地址与范围大小</p>
          </div>
        </div>

        <div className="flex items-center gap-2 relative z-10 text-xs text-slate-400 font-semibold bg-slate-950/40 px-3 py-1.5 rounded-xl border border-slate-800">
          <ShieldCheck size={14} className="text-indigo-400" />
          系统物理基地址: 0x80000000
        </div>
      </div>

      {/* 内存段直观分布条形图 */}
      <div className="bg-slate-900/60 border border-slate-800 rounded-2xl p-5 shadow-xl flex flex-col gap-3">
        <span className="text-xs font-semibold text-slate-400">系统 256MB 物理内存段分布模型 (Concept Layout)</span>
        
        <div className="w-full bg-slate-950 h-8 rounded-xl overflow-hidden border border-slate-800 p-1 flex relative">
          {regions.length === 0 ? (
            <div className="w-full h-full flex items-center justify-center text-[10px] text-slate-600">未分配内存段</div>
          ) : (
            regions.map((r, idx) => {
              // 计算占比
              const startOffset = Math.max(0, r.start_address - baseAddr);
              const width = Math.min(r.size, totalSpan);
              const pct = (width / totalSpan) * 100;
              const leftPct = (startOffset / totalSpan) * 100;

              // 渐变及配色
              const colors = [
                "from-indigo-600/60 to-indigo-500/80 border-indigo-500/30",
                "from-cyan-600/60 to-cyan-500/80 border-cyan-500/30",
                "from-emerald-600/60 to-emerald-500/80 border-emerald-500/30",
                "from-amber-600/60 to-amber-500/80 border-amber-500/30",
                "from-purple-600/60 to-purple-500/80 border-purple-500/30",
              ];
              const color = colors[idx % colors.length];

              return (
                <div
                  key={r.name}
                  className={`h-full border rounded-lg bg-gradient-to-r ${color} flex items-center justify-center text-[9px] font-bold text-white/90 truncate px-1 shadow-md hover:scale-[1.01] hover:z-10 transition-all cursor-help`}
                  style={{
                    width: `${pct}%`,
                    marginLeft: idx === 0 ? `${leftPct}%` : "0.25%",
                  }}
                  title={`${r.name}\n范围: ${formatHex(r.start_address)} - ${formatHex(r.end_address)}\n大小: ${r.size_string}`}
                >
                  {r.name}
                </div>
              );
            })
          )}
        </div>
        <div className="flex justify-between text-[9px] text-slate-500 font-mono">
          <span>0x80000000</span>
          <span>0x88000000</span>
          <span>0x90000000 (256MB)</span>
        </div>
      </div>

      {/* 内存表格 */}
      <div className="flex-1 bg-slate-900/60 border border-slate-800 rounded-2xl p-5 shadow-xl">
        <MemoryTable />
      </div>
    </div>
  );
}