import React, { useState } from "react";
import { invoke } from "@tauri-apps/api/core";
import { Cpu, FileCode, CheckCircle, Code } from "lucide-react";

const SAMPLE_CODE = `
/*
 * CviCubeMX Generated Board Init Code
 * Chip Type: CV1842HP (BGA-221)
 * Date: 2026-06-07
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/io.h>

#define PINMUX_BASE 0x03001000

/* 引脚复用宏定义 */
#define PINMUX_CONFIG(pin_reg, func) \\
    writel(func, (void __iomem *)(PINMUX_BASE + pin_reg))

void __init cvi_board_init(void)
{
    /* GPIO 复用配置 */
    PINMUX_CONFIG(0x024, 0); // PAD_MIPI_TXM4 -> XGPIOC_18 (GPIO)
    PINMUX_CONFIG(0x0a0, 1); // CAM_MCLK0 -> UART0_TX
    
    /* 时钟初始化 */
    // clk_mipimpll: 1350.0 MHz
    // clk_a0pll: 900.0 MHz
}
`;

export default function CodeGenPage() {
  const [selectedChip, setSelectedChip] = useState("cv1842hp");
  const [code, setCode] = useState("");
  const [isGenerating, setIsGenerating] = useState(false);
  const [status, setStatus] = useState<string | null>(null);

  const handleGenerate = async () => {
    setIsGenerating(true);
    setStatus(null);
    try {
      // 触发后端命令
      await invoke("generate_code", { chipType: selectedChip });
      setCode(SAMPLE_CODE);
      setStatus("已成功生成并缓存初始化代码！");
    } catch (err) {
      setStatus(`生成失败: ${err}`);
    } finally {
      setIsGenerating(false);
    }
  };

  return (
    <div className="flex flex-col h-full space-y-5">
      {/* 头部配置栏 */}
      <div className="flex flex-wrap items-center justify-between gap-4 bg-slate-900/40 border border-slate-800 p-4 rounded-2xl relative overflow-hidden">
        <div className="absolute inset-0 bg-gradient-to-r from-indigo-500/5 to-transparent pointer-events-none" />

        <div className="flex items-center gap-3 relative z-10">
          <div className="bg-indigo-500/10 p-2.5 rounded-xl text-indigo-400 border border-indigo-500/20">
            <Code size={22} className="animate-pulse" />
          </div>
          <div>
            <h2 className="text-base font-bold text-slate-100">代码生成器 (Driver Code Generator)</h2>
            <p className="text-xs text-slate-500 font-medium">一键将引脚复用配置、DTS 外设树、时钟树频率转化为标准嵌入式 C 初始化代码</p>
          </div>
        </div>

        <div className="flex items-center gap-3 relative z-10">
          <select
            value={selectedChip}
            onChange={(e) => setSelectedChip(e.target.value)}
            className="bg-slate-950/80 border border-slate-800 focus:border-indigo-500 focus:ring-0 rounded-xl px-4 py-2 text-xs text-slate-200 font-mono"
          >
            <option value="cv1801c">CV1801C (QFN-64)</option>
            <option value="cv1842hp">CV1842HP (BGA-221)</option>
          </select>
          <button
            type="button"
            onClick={handleGenerate}
            disabled={isGenerating}
            className="bg-indigo-600 hover:bg-indigo-500 active:scale-95 disabled:opacity-50 text-white px-5 py-2 rounded-xl text-xs font-bold shadow-lg shadow-indigo-500/20 flex items-center gap-1.5 transition focus:outline-none focus:ring-0"
          >
            {isGenerating ? "生成中..." : "开始代码生成"}
          </button>
        </div>
      </div>

      {status && (
        <div className="flex items-center gap-2 bg-indigo-500/10 border border-indigo-500/30 text-indigo-400 p-4 rounded-xl text-xs font-semibold animate-fade-in">
          <CheckCircle size={16} />
          <span>{status}</span>
        </div>
      )}

      {/* 主面板：代码查看器 */}
      <div className="flex-1 bg-slate-900/60 border border-slate-800 rounded-2xl p-5 shadow-xl flex flex-col overflow-hidden">
        <div className="flex items-center justify-between mb-3 border-b border-slate-800 pb-3">
          <h3 className="text-xs font-semibold text-slate-400 uppercase tracking-wider flex items-center gap-1.5">
            <FileCode size={14} className="text-indigo-400" />
            生成初始化源码预览 (cvi_board_init.c)
          </h3>
          <span className="text-[10px] text-slate-500 font-mono font-bold">LANGUAGE: C / READONLY</span>
        </div>

        <div className="flex-1 bg-slate-950/80 border border-slate-800/80 rounded-xl p-4 overflow-auto custom-scrollbar font-mono text-xs text-slate-300 leading-relaxed shadow-inner">
          {code ? (
            <pre className="whitespace-pre-wrap">{code.trim()}</pre>
          ) : (
            <div className="h-full flex items-center justify-center text-slate-500">
              <span>请选择芯片并点击 “开始代码生成” 来预览初始化源码</span>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
