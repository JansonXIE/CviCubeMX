import React, { useState } from "react";
import { usePeripheralStore, PeripheralInfo } from "../stores/peripheralStore";
import { Settings, ChevronDown, ChevronRight, ToggleLeft, ToggleRight, Radio } from "lucide-react";
import ConfigDialog from "./ConfigDialog";

export default function PeripheralTree() {
  const { peripherals, setPeripheralStatus } = usePeripheralStore();
  const [expandedCategories, setExpandedCategories] = useState<Record<string, boolean>>({
    I2C: true,
    UART: true,
    PWM: true,
    SPI: true,
    DMA: true,
    OTHERS: false,
  });
  const [activePeripheral, setActivePeripheral] = useState<PeripheralInfo | null>(null);

  // 分类策略
  const categorize = (name: string): string => {
    const n = name.toLowerCase();
    if (n.includes("i2c") || n.includes("iic")) return "I2C";
    if (n.includes("uart")) return "UART";
    if (n.includes("pwm")) return "PWM";
    if (n.includes("spi")) return "SPI";
    if (n.includes("dma")) return "DMA";
    return "OTHERS";
  };

  // 分类汇总
  const groups: Record<string, PeripheralInfo[]> = {
    I2C: [],
    UART: [],
    PWM: [],
    SPI: [],
    DMA: [],
    OTHERS: [],
  };

  peripherals.forEach((p) => {
    const cat = categorize(p.name);
    if (!groups[cat]) {
      groups[cat] = [];
    }
    groups[cat].push(p);
  });

  const toggleCategory = (cat: string) => {
    setExpandedCategories((prev) => ({
      ...prev,
      [cat]: !prev[cat],
    }));
  };

  const handleToggleStatus = async (p: PeripheralInfo) => {
    const nextStatus = p.status === "okay" ? "disabled" : "okay";
    await setPeripheralStatus(p.name, nextStatus);
  };

  return (
    <div className="bg-slate-900/60 backdrop-blur-xl border border-slate-700/50 rounded-2xl p-5 shadow-2xl h-full flex flex-col overflow-hidden relative">
      <div className="absolute inset-0 bg-gradient-to-br from-indigo-500/5 to-cyan-500/5 pointer-events-none" />

      <h3 className="font-bold text-slate-300 mb-4 flex items-center gap-2 relative z-10">
        <Radio size={18} className="text-indigo-400" />
        设备树外设资源
      </h3>

      <div className="flex-1 overflow-y-auto space-y-2 pr-1 custom-scrollbar relative z-10">
        {Object.entries(groups).map(([cat, items]) => {
          if (items.length === 0) return null;
          const isExpanded = expandedCategories[cat];

          return (
            <div key={cat} className="border border-slate-800 rounded-xl bg-slate-950/20 overflow-hidden">
              {/* 分组头部 */}
              <button
                type="button"
                onClick={() => toggleCategory(cat)}
                className="w-full flex items-center justify-between px-4 py-3 bg-slate-900/40 text-slate-300 hover:bg-slate-900/60 transition-all focus:outline-none focus:ring-0"
              >
                <span className="font-semibold text-xs tracking-wider uppercase text-slate-400 flex items-center gap-1">
                  {cat} ({items.length})
                </span>
                {isExpanded ? <ChevronDown size={16} /> : <ChevronRight size={16} />}
              </button>

              {/* 外设项列表 */}
              {isExpanded && (
                <div className="p-2 space-y-1.5 divide-y divide-slate-900/30">
                  {items.map((p) => {
                    const isOkay = p.status === "okay";
                    return (
                      <div
                        key={p.name}
                        className="flex items-center justify-between px-3 py-2 text-xs text-slate-300 hover:bg-slate-800/40 rounded-lg group transition-all"
                      >
                        <div className="flex items-center gap-2">
                          <span className={`w-2 h-2 rounded-full ${isOkay ? "bg-emerald-500 animate-pulse" : "bg-slate-600"}`} />
                          <span className="font-mono">{p.name}</span>
                        </div>

                        <div className="flex items-center gap-2 opacity-80 group-hover:opacity-100 transition-opacity">
                          {/* 快速开关 */}
                          <button
                            type="button"
                            onClick={() => handleToggleStatus(p)}
                            className="text-slate-400 hover:text-white transition focus:outline-none focus:ring-0"
                            title={isOkay ? "禁用外设" : "启用外设"}
                          >
                            {isOkay ? (
                              <ToggleRight size={20} className="text-emerald-500" />
                            ) : (
                              <ToggleLeft size={20} className="text-slate-500" />
                            )}
                          </button>

                          {/* 配置表单 */}
                          <button
                            type="button"
                            onClick={() => setActivePeripheral(p)}
                            className="text-slate-400 hover:text-indigo-400 p-1 rounded hover:bg-slate-800 transition focus:outline-none focus:ring-0"
                            title="配置参数"
                          >
                            <Settings size={14} />
                          </button>
                        </div>
                      </div>
                    );
                  })}
                </div>
              )}
            </div>
          );
        })}
      </div>

      {/* 外设配置对话弹窗 */}
      {activePeripheral && (
        <ConfigDialog
          peripheral={activePeripheral}
          isOpen={!!activePeripheral}
          onClose={() => setActivePeripheral(null)}
        />
      )}
    </div>
  );
}
