import React from "react";
import PeripheralTree from "../components/PeripheralTree";
import { FileCode, Settings } from "lucide-react";
import { usePeripheralStore } from "../stores/peripheralStore";

export default function PeripheralPage() {
  const { dtsContent } = usePeripheralStore();

  return (
    <div className="flex flex-col h-full space-y-5">
      {/* 头部配置栏 */}
      <div className="flex flex-wrap items-center justify-between gap-4 bg-slate-900/40 border border-slate-800 p-4 rounded-2xl relative overflow-hidden">
        <div className="absolute inset-0 bg-gradient-to-r from-indigo-500/5 to-transparent pointer-events-none" />

        <div className="flex items-center gap-3 relative z-10">
          <div className="bg-indigo-500/10 p-2.5 rounded-xl text-indigo-400 border border-indigo-500/20">
            <Settings size={22} className="animate-pulse" />
          </div>
          <div>
            <h2 className="text-base font-bold text-slate-100">外设管理面板 (Peripheral Resources)</h2>
            <p className="text-xs text-slate-500 font-medium">查看和配置 DTS（设备树）中注册的外设寄存器资源和属性映射</p>
          </div>
        </div>
      </div>

      <div className="flex-1 grid grid-cols-1 lg:grid-cols-3 gap-6 overflow-hidden">
        {/* 左侧外设资源 (1/3) */}
        <div className="lg:col-span-1 h-full overflow-hidden">
          <PeripheralTree />
        </div>

        {/* 右侧 DTS 文件内容实时预览 (2/3) */}
        <div className="lg:col-span-2 bg-slate-900/60 border border-slate-800 rounded-2xl p-5 shadow-xl flex flex-col h-full overflow-hidden">
          <h3 className="text-xs font-semibold text-slate-400 uppercase tracking-wider mb-3 flex items-center gap-1.5">
            <FileCode size={14} className="text-indigo-400" />
            DTS 配置文件实时预览 <span className="normal-case text-slate-500 font-normal">({`build/boards/default/dts/cv184x/cv184x_base.dtsi`})</span>
          </h3>
          <div className="flex-1 bg-slate-950/80 border border-slate-800/80 rounded-xl p-4 overflow-auto custom-scrollbar font-mono text-xs text-slate-300 leading-relaxed shadow-inner">
            <pre className="whitespace-pre-wrap">{dtsContent ? dtsContent.trim() : "正在加载设备树配置文件..."}</pre>
          </div>
        </div>
      </div>
    </div>
  );
}
