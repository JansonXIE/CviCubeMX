import React from "react";
import PartitionTable from "../components/PartitionTable";
import { Zap, HelpCircle } from "lucide-react";

export default function FlashPage() {
  return (
    <div className="flex flex-col h-full space-y-5">
      {/* 头部配置栏 */}
      <div className="flex flex-wrap items-center justify-between gap-4 bg-slate-900/40 border border-slate-800 p-4 rounded-2xl relative overflow-hidden">
        <div className="absolute inset-0 bg-gradient-to-r from-indigo-500/5 to-transparent pointer-events-none" />

        <div className="flex items-center gap-3 relative z-10">
          <div className="bg-indigo-500/10 p-2.5 rounded-xl text-indigo-400 border border-indigo-500/20">
            <Zap size={22} className="animate-pulse" />
          </div>
          <div>
            <h2 className="text-base font-bold text-slate-100">闪存分区表配置 (Flash Partition Layout)</h2>
            <p className="text-xs text-slate-500 font-medium">规划 SPI Nor/Nand Flash 上的分区边界、打包镜像文件挂载点及对应的文件系统类型</p>
          </div>
        </div>

        <div className="flex items-center gap-2 relative z-10 text-xs text-slate-400 font-semibold bg-slate-950/40 px-3 py-1.5 rounded-xl border border-slate-800 font-mono">
          FLASH: 32 MB SPI Nor
        </div>
      </div>

      <div className="flex-1 grid grid-cols-1 xl:grid-cols-4 gap-6 overflow-hidden">
        {/* 表格区 (3/4) */}
        <div className="xl:col-span-3 bg-slate-900/60 border border-slate-800 rounded-2xl p-5 shadow-xl flex flex-col h-full overflow-hidden">
          <PartitionTable />
        </div>

        {/* 侧边说明卡片 (1/4) */}
        <div className="xl:col-span-1 space-y-4">
          <h3 className="text-sm font-bold text-slate-400 uppercase tracking-wider">分区指导规范</h3>
          
          <div className="bg-slate-900/60 border border-slate-800 rounded-2xl p-5 shadow-xl space-y-4 text-xs text-slate-300">
            <div className="flex gap-2">
              <HelpCircle size={16} className="text-indigo-400 flex-shrink-0 mt-0.5" />
              <div>
                <p className="font-bold">分区边界与对其对齐</p>
                <p className="text-slate-400 text-[11px] mt-1">每个分区起始位置和大小应尽量为 Flash 擦除扇区大小 (通常为 64KB) 的整数倍，避免跨扇区写入降低寿命。</p>
              </div>
            </div>

            <div className="flex gap-2 border-t border-slate-800 pt-3">
              <HelpCircle size={16} className="text-indigo-400 flex-shrink-0 mt-0.5" />
              <div>
                <p className="font-bold">只读与读写区分</p>
                <p className="text-slate-400 text-[11px] mt-1">建议将根文件系统 (ROOTFS) 设置为 squashfs 等只读文件系统，而将配置和临时日志段 (/data) 设置为 jffs2 等可写系统。</p>
              </div>
            </div>

            <div className="flex gap-2 border-t border-slate-800 pt-3">
              <HelpCircle size={16} className="text-indigo-400 flex-shrink-0 mt-0.5" />
              <div>
                <p className="font-bold">镜像打包自动化</p>
                <p className="text-slate-400 text-[11px] mt-1">保存分区配置后，底层 SDK 脚本将读取此表以自动生成打包的 Flash 烧录镜像 binary 文件。</p>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}