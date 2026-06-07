import React, { useEffect, useState } from "react";
import { useChipStore } from "../stores/chipStore";
import { usePeripheralStore } from "../stores/peripheralStore";
import PeripheralTree from "../components/PeripheralTree";
import ChipCanvas from "../components/ChipCanvas";
import { Search, Save, Cpu } from "lucide-react";
import { invoke } from "@tauri-apps/api/core";

const CHIP_OPTIONS = [
  { value: "cv1801c", label: "CV1801C (QFN-64)" },
  { value: "cv1801h", label: "CV1801H (BGA-60)" },
  { value: "cv1811c", label: "CV1811C (QFN-88)" },
  { value: "cv1811h", label: "CV1811H (BGA-84)" },
  { value: "cv1842cp", label: "CV1842CP (QFN-88)" },
  { value: "cv1842hp", label: "CV1842HP (BGA-221)" },
];

export default function PinoutPage() {
  const { selectChip, searchPin, chipType, isLoading } = useChipStore();
  const { loadPeripherals } = usePeripheralStore();
  const [searchValue, setSearchValue] = useState("");
  const [selectedChipType, setSelectedChipType] = useState("cv1842hp");
  const [genResult, setGenResult] = useState<string | null>(null);

  // 初始化选择默认芯片
  useEffect(() => {
    handleChipSelect(selectedChipType);
  }, []);

  const handleChipSelect = async (type: string) => {
    setSelectedChipType(type);
    await selectChip(type);
    
    // 假设 DTS 路径位于 boards_pinout/芯片/dts
    const mockDtsPath = `boards_pinout/${type}/dts`;
    await loadPeripherals(mockDtsPath);
  };

  const handleSearchChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const val = e.target.value;
    setSearchValue(val);
    searchPin(val);
  };

  const handleGenerateCode = async () => {
    setGenResult(null);
    try {
      // 触发 Rust 端的代码生成命令
      const result = await invoke<string>("generate_code", { chipType: selectedChipType });
      setGenResult("代码生成成功！");
      setTimeout(() => setGenResult(null), 3000);
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
              className="bg-slate-950/80 border border-slate-800 focus:border-indigo-500 focus:ring-0 rounded-xl pl-9 pr-4 py-2 text-xs text-slate-200 w-52 placeholder-slate-600 transition-all"
            />
          </div>

          {/* 芯片下拉选型 */}
          <select
            value={selectedChipType}
            onChange={(e) => handleChipSelect(e.target.value)}
            disabled={isLoading}
            className="bg-slate-950/80 border border-slate-800 focus:border-indigo-500 focus:ring-0 rounded-xl px-4 py-2 text-xs text-slate-200 transition-all font-mono"
          >
            {CHIP_OPTIONS.map((opt) => (
              <option key={opt.value} value={opt.value}>
                {opt.label}
              </option>
            ))}
          </select>

          {/* 生成代码按钮 */}
          <button
            type="button"
            onClick={handleGenerateCode}
            className="bg-indigo-600 hover:bg-indigo-500 active:scale-95 text-white px-5 py-2 rounded-xl text-xs font-bold shadow-lg shadow-indigo-500/20 flex items-center gap-1.5 transition-all focus:outline-none focus:ring-0"
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

      {/* 主面板布局：左侧外设树，右侧引脚画布 */}
      <div className="flex-1 grid grid-cols-1 lg:grid-cols-3 gap-6 overflow-hidden">
        {/* 左侧外设资源 (1/3) */}
        <div className="lg:col-span-1 h-full min-h-[400px] overflow-hidden">
          <PeripheralTree />
        </div>

        {/* 右侧引脚画布 (2/3) */}
        <div className="lg:col-span-2 h-full overflow-hidden">
          <ChipCanvas />
        </div>
      </div>
    </div>
  );
}