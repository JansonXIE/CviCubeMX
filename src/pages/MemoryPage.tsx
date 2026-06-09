import React, { useState } from "react";
import MemoryTable from "../components/MemoryTable";
import { Cpu, ShieldCheck } from "lucide-react";
import { useMemoryStore } from "../stores/memoryStore";

// 十六进制格式化
function formatHex(val: number): string {
  return "0x" + val.toString(16).toUpperCase().padStart(8, "0");
}

// 格式化可读大小
function formatSizeReadable(sizeInBytes: number): string {
  if (sizeInBytes >= 1024 * 1024) {
    const mb = sizeInBytes / (1024 * 1024);
    return Number.isInteger(mb) ? `${mb}M` : `${mb.toFixed(1)}M`;
  } else if (sizeInBytes >= 1024) {
    const kb = sizeInBytes / 1024;
    return Number.isInteger(kb) ? `${kb}K` : `${kb.toFixed(1)}K`;
  }
  return `${sizeInBytes}B`;
}

interface MemorySegment {
  type: "used" | "free";
  name: string;
  start_address: number;
  end_address: number;
  size: number;
  size_string: string;
  description?: string;
  colorIndex?: number;
}

export default function MemoryPage() {
  const { regions } = useMemoryStore();
  const [hoveredRegion, setHoveredRegion] = useState<string | null>(null);

  // 渲染物理内存段分布概念图
  // 基准范围 0x80000000 到 0x90000000 (256MB)
  const baseAddr = 0x80000000;
  const totalSpan = 0x10000000; // 256MB
  const endAddr = baseAddr + totalSpan;

  // 过滤出 256M 范围内的区域并按起始物理地址升序排序
  const sortedRegions = [...regions]
    .filter((r) => r.start_address >= baseAddr && r.end_address <= endAddr)
    .sort((a, b) => a.start_address - b.start_address);

  // 补全所有未分配空间的段
  const segments: MemorySegment[] = [];
  let currentAddr = baseAddr;

  sortedRegions.forEach((r, idx) => {
    if (r.start_address > currentAddr) {
      const freeSize = r.start_address - currentAddr;
      segments.push({
        type: "free",
        name: "未分配空间",
        start_address: currentAddr,
        end_address: r.start_address,
        size: freeSize,
        size_string: formatSizeReadable(freeSize),
      });
    }
    segments.push({
      type: "used",
      name: r.name,
      start_address: r.start_address,
      end_address: r.end_address,
      size: r.size,
      size_string: r.size_string,
      description: r.description,
      colorIndex: idx,
    });
    currentAddr = Math.max(currentAddr, r.end_address);
  });

  if (currentAddr < endAddr) {
    const freeSize = endAddr - currentAddr;
    segments.push({
      type: "free",
      name: "未分配空间",
      start_address: currentAddr,
      end_address: endAddr,
      size: freeSize,
      size_string: formatSizeReadable(freeSize),
    });
  }

  return (
    <div className="flex flex-col h-full space-y-5 overflow-hidden">
      {/* 头部配置栏 */}
      <div className="flex flex-wrap items-center justify-between gap-4 bg-slate-900/40 border border-slate-800 p-4 rounded-2xl relative overflow-hidden shrink-0">
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

      {/* 左右分栏 */}
      <div className="flex flex-1 gap-5 min-h-0 overflow-hidden">
        {/* 左侧：内存区域映射表 */}
        <div className="flex-[7] bg-slate-900/60 border border-slate-800 rounded-2xl p-5 shadow-xl flex flex-col min-h-0 overflow-hidden">
          <MemoryTable hoveredRegion={hoveredRegion} onHoverRegion={setHoveredRegion} />
        </div>

        {/* 右侧：物理内存段分布模型 (Concept Layout) */}
        <div className="flex-[3] bg-slate-900/60 border border-slate-800 rounded-2xl p-5 shadow-xl flex flex-col min-h-0 overflow-hidden">
          <div className="flex flex-col gap-1 border-b border-slate-800/80 pb-3 shrink-0">
            <div className="flex items-center gap-2">
              <span className="text-xs font-bold text-slate-200">物理内存分布模型</span>
              <span className="text-[9px] bg-indigo-500/10 text-indigo-400 border border-indigo-500/20 px-1.5 py-0.5 rounded-md font-bold">Concept Layout</span>
            </div>
            <p className="text-[10px] text-slate-500 font-medium">256MB DDR 物理映射关系与未分配空间补齐展示</p>
          </div>

          {/* 垂直堆叠渲染区域 */}
          <div className="flex-1 overflow-y-auto pr-1 mt-4 space-y-2.5 custom-scrollbar min-h-0">
            <div className="flex justify-between items-center text-[9px] text-slate-500 font-mono pb-1 border-b border-slate-800/30">
              <span>起始 (LOW)</span>
              <span>0x80000000</span>
            </div>

            <div className="flex flex-col gap-2 relative mt-2">
              {segments.length === 0 ? (
                <div className="h-32 flex items-center justify-center text-xs text-slate-600 border border-dashed border-slate-800 rounded-xl">
                  暂无物理段配置
                </div>
              ) : (
                segments.map((seg, idx) => {
                  // 计算高度：已分配 42px 到 100px，未分配 34px 到 60px
                  const h = seg.type === "used"
                    ? Math.max(42, Math.min(100, (seg.size / totalSpan) * 320))
                    : Math.max(34, Math.min(60, (seg.size / totalSpan) * 160));

                  if (seg.type === "free") {
                    return (
                      <div
                        key={`free-${seg.start_address}-${idx}`}
                        style={{ height: `${h}px` }}
                        className="relative flex items-center justify-between px-3 py-2 rounded-xl border border-dashed border-slate-800/40 bg-slate-950/20 text-slate-500 select-none min-h-[34px] bg-[linear-gradient(45deg,rgba(255,255,255,0.015)_25%,transparent_25%,transparent_50%,rgba(255,255,255,0.015)_50%,rgba(255,255,255,0.015)_75%,transparent_75%,transparent)] bg-[size:16px_16px] transition-all hover:border-slate-700/60"
                      >
                        <div className="flex items-center gap-1.5 text-[10px] font-semibold text-slate-600">
                          <span className="w-1.5 h-1.5 rounded-full bg-slate-800" />
                          <span>未分配空间</span>
                        </div>
                        <div className="flex items-center gap-2 font-mono text-[9px] text-slate-600">
                          <span>{seg.size_string}</span>
                          <span className="opacity-60 text-[8px]">({formatHex(seg.start_address)})</span>
                        </div>
                      </div>
                    );
                  }

                  // 已分配物理段的颜色配色
                  const colors = [
                    "from-indigo-500/10 to-indigo-500/5 border-indigo-500/30 text-indigo-300 hover:from-indigo-500/15 hover:to-indigo-500/10",
                    "from-cyan-500/10 to-cyan-500/5 border-cyan-500/30 text-cyan-300 hover:from-cyan-500/15 hover:to-cyan-500/10",
                    "from-emerald-500/10 to-emerald-500/5 border-emerald-500/30 text-emerald-300 hover:from-emerald-500/15 hover:to-emerald-500/10",
                    "from-amber-500/10 to-amber-500/5 border-amber-500/30 text-amber-300 hover:from-amber-500/15 hover:to-amber-500/10",
                    "from-purple-500/10 to-purple-500/5 border-purple-500/30 text-purple-300 hover:from-purple-500/15 hover:to-purple-500/10",
                  ];
                  const color = colors[(seg.colorIndex || 0) % colors.length];
                  const isHovered = hoveredRegion === seg.name;

                  // 对应的左侧颜色标示线颜色
                  const indicatorColors = [
                    "bg-indigo-500/80",
                    "bg-cyan-500/80",
                    "bg-emerald-500/80",
                    "bg-amber-500/80",
                    "bg-purple-500/80",
                  ];
                  const indicatorColor = indicatorColors[(seg.colorIndex || 0) % indicatorColors.length];

                  return (
                    <div
                      key={seg.name}
                      style={{ height: `${h}px` }}
                      className={`relative flex flex-col justify-between p-3 rounded-xl border bg-gradient-to-r transition-all duration-200 cursor-pointer select-none min-h-[42px] ${color} ${
                        isHovered
                          ? "border-indigo-400/80 shadow-[0_0_12px_rgba(99,102,241,0.25)] scale-[1.015]"
                          : "border-slate-800/80"
                      }`}
                      onMouseEnter={() => setHoveredRegion(seg.name)}
                      onMouseLeave={() => setHoveredRegion(null)}
                      title={`${seg.name}\n范围: ${formatHex(seg.start_address)} - ${formatHex(seg.end_address)}\n大小: ${seg.size_string}`}
                    >
                      {/* 左侧区域特定色条，高亮时转为亮色 */}
                      <div
                        className={`absolute left-0 top-2.5 bottom-2.5 w-1 rounded-r transition-all duration-200 ${
                          isHovered ? "bg-indigo-400 w-1.5" : indicatorColor
                        }`}
                      />

                      <div className="flex justify-between items-start gap-2 pl-2">
                        <span className="font-mono font-bold text-xs truncate leading-none">{seg.name}</span>
                        <span className="font-mono text-[9px] opacity-80 leading-none shrink-0">{seg.size_string}</span>
                      </div>

                      <div className="flex justify-between items-center text-[9px] opacity-50 font-mono pl-2 leading-none mt-1">
                        <span>{formatHex(seg.start_address)}</span>
                        <span>→</span>
                        <span>{formatHex(seg.end_address)}</span>
                      </div>
                    </div>
                  );
                })
              )}
            </div>

            <div className="flex justify-between items-center text-[9px] text-slate-500 font-mono pt-1.5 border-t border-slate-800/30 mt-2">
              <span>结束 (HIGH)</span>
              <span>0x90000000 (256MB)</span>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}