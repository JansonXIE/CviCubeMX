import React, { useState } from "react";
import { useChipStore } from "../stores/chipStore";
import ChipCanvas from "../components/ChipCanvas";
import { Search, Save, Cpu } from "lucide-react";
import { invoke } from "@tauri-apps/api/core";
import { useSdkStore } from "../stores/sdkStore";

export default function PinoutPage() {
  const { searchPin, isLoading, pins } = useChipStore();
  const { sdkPath, chipType, setIsOnboardingOpen } = useSdkStore();
  const [searchValue, setSearchValue] = useState("");
  const [genResult, setGenResult] = useState<string | null>(null);

  const handleSearchChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const val = e.target.value;
    setSearchValue(val);
    searchPin(val);
  };

  const handleGenerateCode = async () => {
    if (!sdkPath || !chipType) {
      setGenResult("请先配置全局 SDK 源码路径和芯片型号");
      return;
    }
    setGenResult(null);
    try {
      // 过滤出 user_configured = true 的引脚配置供合并
      const pinConfigs = Array.from(pins.values())
        .filter((pin) => pin.user_configured)
        .map((pin) => ({
          pin_name: pin.pin_name,
          function: pin.current_function,
          user_configured: pin.user_configured,
          state: pin.current_state ?? null,
        }));

      // 触发 Rust 端的代码生成/合并命令
      const result = await invoke<string>("generate_board_init_code", {
        sdkPath,
        chipType,
        pinConfigs,
      });

      setGenResult(`代码生成成功！已写入/更新: build/boards/cv184x/${chipType}/u-boot/cvi_board_init.c`);
      setTimeout(() => setGenResult(null), 4000);
    } catch (e) {
      setGenResult(`代码生成失败: ${e}`);
      setTimeout(() => setGenResult(null), 5000);
    }
  };

  return (
    <div className="flex flex-col h-full space-y-5">
      {/* 头部配置栏 */}
      <div className="flex flex-wrap items-center justify-between gap-4 bg-slate-900/40 border border-slate-800 p-4 rounded-2xl relative overflow-hidden">
        {/* 背景微光 */}
        <div className="absolute inset-0 bg-gradient-to-r from-indigo-500/5 to-transparent pointer-events-none" />

        <div className="flex items-center gap-3 relative z-10">
          <div className="bg-indigo-500/10 p-2.5 rounded-xl text-indigo-400 border border-indigo-500/20">
            <Cpu size={22} className="animate-pulse" />
          </div>
          <div>
            <h2 className="text-base font-bold text-slate-100">引脚复用配置 (Pinout & Configuration)</h2>
            <p className="text-xs text-slate-500 font-medium">选型芯片、配置各引脚在 DTS 和底层驱动中的复用功能</p>
          </div>
        </div>

        {/* 选型、搜索与控制入口 */}
        <div className="flex flex-wrap items-center gap-3 relative z-10">
          {/* 搜索框 */}
          <div className="relative">
            <Search size={14} className="absolute left-3 top-1/2 -translate-y-1/2 text-slate-500" />
            <input
              type="text"
              value={searchValue}
              onChange={handleSearchChange}
              placeholder="搜索引脚编号 / 功能..."
              className="bg-slate-950/80 border border-slate-800 focus:border-indigo-500 focus:ring-0 rounded-xl pl-9 pr-4 py-2 text-xs text-slate-200 w-52 placeholder-slate-600 transition-all focus:outline-none"
            />
          </div>

          {/* 芯片型号展示 */}
          <div className="flex items-center gap-2 bg-slate-950/80 border border-slate-800 rounded-xl px-4 py-2 text-xs text-slate-300 font-mono">
            <span>芯片: {chipType || "未选型"}</span>
          </div>

          <button
            type="button"
            onClick={() => setIsOnboardingOpen(true)}
            className="px-3 py-2 bg-slate-800 hover:bg-slate-700 active:scale-95 text-slate-200 rounded-xl text-xs font-semibold border border-slate-700 transition-all focus:outline-none focus:ring-0"
          >
            更改配置
          </button>

          {/* 生成代码按钮 */}
          <button
            type="button"
            onClick={handleGenerateCode}
            disabled={isLoading}
            className="bg-indigo-600 hover:bg-indigo-500 active:scale-95 text-white px-5 py-2 rounded-xl text-xs font-bold shadow-lg shadow-indigo-500/20 flex items-center gap-1.5 transition-all focus:outline-none focus:ring-0 disabled:opacity-50"
          >
            <Save size={14} />
            生成驱动代码
          </button>
        </div>
      </div>

      {genResult && (
        <div className="flex items-center gap-2 bg-indigo-500/10 border border-indigo-500/30 text-indigo-400 p-4 rounded-xl text-xs font-semibold animate-fade-in">
          <span>{genResult}</span>
        </div>
      )}

      {/* 主面板布局：引脚画布满宽 */}
      <div className="flex-1 w-full min-h-0 overflow-hidden">
        <ChipCanvas />
      </div>
    </div>
  );
}