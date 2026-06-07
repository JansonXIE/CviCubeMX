import React, { useEffect, useState } from "react";
import { useClockStore, PllConfig } from "../stores/clockStore";
import { Search, Save, Clock, HelpCircle, RefreshCw } from "lucide-react";

export default function ClockPage() {
  const {
    pllConfigs,
    outputs,
    subNodes,
    searchText,
    isLoading,
    error,
    computeClockTree,
    searchClock,
    exportClockDefconfig,
  } = useClockStore();

  const [localPlls, setLocalPlls] = useState<Record<string, PllConfig>>({});
  const [localSearch, setLocalSearch] = useState("");
  const [exportMsg, setExportMsg] = useState<string | null>(null);

  // 初始化时钟树
  useEffect(() => {
    // 初始空配置，触发首次计算
    computeClockTree({});
  }, []);

  // 当 store 里的 pllConfigs 变化时同步到本地状态以编辑
  useEffect(() => {
    if (Object.keys(pllConfigs).length > 0) {
      setLocalPlls(JSON.parse(JSON.stringify(pllConfigs)));
    }
  }, [pllConfigs]);

  const handlePllChange = async (name: string, field: keyof PllConfig, value: any) => {
    const updated = {
      ...localPlls,
      [name]: {
        ...localPlls[name],
        [field]: value,
      },
    };
    setLocalPlls(updated);
    // 触发联动重算
    await computeClockTree(updated);
  };

  const handleSearchChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const val = e.target.value;
    setLocalSearch(val);
    searchClock(val);
  };

  const handleExport = async () => {
    setExportMsg(null);
    try {
      await exportClockDefconfig("boards_pinout/cv1842hp/defconfig", "cv1842hp", localPlls);
      setExportMsg("时钟配置导出成功！");
      setTimeout(() => setExportMsg(null), 3000);
    } catch (e) {
      setExportMsg(`时钟配置导出失败: ${e}`);
      setTimeout(() => setExportMsg(null), 5000);
    }
  };

  // 过滤显示
  const filterNodes = (name: string) => {
    if (!searchText) return true;
    return name.toLowerCase().includes(searchText.toLowerCase());
  };

  return (
    <div className="flex flex-col h-full space-y-5">
      {/* 头部配置栏 */}
      <div className="flex flex-wrap items-center justify-between gap-4 bg-slate-900/40 border border-slate-800 p-4 rounded-2xl relative overflow-hidden">
        <div className="absolute inset-0 bg-gradient-to-r from-indigo-500/5 to-transparent pointer-events-none" />

        <div className="flex items-center gap-3 relative z-10">
          <div className="bg-indigo-500/10 p-2.5 rounded-xl text-indigo-400 border border-indigo-500/20">
            <Clock size={22} className="animate-pulse" />
          </div>
          <div>
            <h2 className="text-base font-bold text-slate-100">时钟树频率计算 (Clock Tree Configuration)</h2>
            <p className="text-xs text-slate-500 font-medium">配置晶振、各主 PLL 乘除分频因子以动态生成系统总线及时钟输出频率</p>
          </div>
        </div>

        <div className="flex items-center gap-3 relative z-10">
          {/* 搜索 */}
          <div className="relative">
            <Search size={14} className="absolute left-3 top-1/2 -translate-y-1/2 text-slate-500" />
            <input
              type="text"
              value={localSearch}
              onChange={handleSearchChange}
              placeholder="搜索时钟节点..."
              className="bg-slate-950/80 border border-slate-800 focus:border-indigo-500 focus:ring-0 rounded-xl pl-9 pr-4 py-2 text-xs text-slate-200 w-48 placeholder-slate-600 transition-all"
            />
          </div>

          <button
            type="button"
            onClick={handleExport}
            className="bg-indigo-600 hover:bg-indigo-500 active:scale-95 text-white px-5 py-2 rounded-xl text-xs font-bold shadow-lg shadow-indigo-500/20 flex items-center gap-1.5 transition-all focus:outline-none focus:ring-0"
          >
            <Save size={14} />
            保存时钟树
          </button>
        </div>
      </div>

      {exportMsg && (
        <div className="flex items-center gap-2 bg-indigo-500/10 border border-indigo-500/30 text-indigo-400 p-4 rounded-xl text-xs font-semibold animate-fade-in">
          <span>{exportMsg}</span>
        </div>
      )}

      {/* 主画布拓扑 */}
      <div className="flex-1 grid grid-cols-1 xl:grid-cols-4 gap-6 overflow-y-auto custom-scrollbar">
        {/* 晶振基础时钟源 */}
        <div className="xl:col-span-1 space-y-4">
          <h3 className="text-sm font-bold text-slate-400 uppercase tracking-wider">晶振时钟源 (Oscillators)</h3>
          <div className="bg-slate-900/60 border border-slate-800 rounded-2xl p-5 space-y-4 shadow-xl">
            {/* OSC */}
            <div className="flex items-center justify-between border-b border-slate-800 pb-3">
              <div>
                <span className="font-bold text-xs text-slate-300 font-mono">OSC_XTAL</span>
                <p className="text-[10px] text-slate-500">主外部高速无源晶振</p>
              </div>
              <span className="font-mono font-bold text-emerald-400 text-sm">25.0 MHz</span>
            </div>
            {/* RTC */}
            <div className="flex items-center justify-between">
              <div>
                <span className="font-bold text-xs text-slate-300 font-mono">RTC_CLK</span>
                <p className="text-[10px] text-slate-500">低速外部实时时钟源</p>
              </div>
              <span className="font-mono font-bold text-emerald-400 text-sm">32.768 kHz</span>
            </div>
          </div>

          <div className="bg-indigo-950/20 border border-indigo-500/10 rounded-2xl p-4 text-xs text-slate-400 leading-relaxed flex gap-2">
            <HelpCircle size={18} className="text-indigo-400 flex-shrink-0 mt-0.5" />
            <div>
              <p className="font-semibold text-slate-300">联动重算说明:</p>
              <p className="text-[11px] mt-1">改变 PLL 的 Multiplier (倍频系数) 或 Divider (分频系数)，系统将自动通过后端级联重算其子时钟树的实际频率，确保拓扑正确性。</p>
            </div>
          </div>
        </div>

        {/* PLL 节点配置 */}
        <div className="xl:col-span-2 space-y-4">
          <h3 className="text-sm font-bold text-slate-400 uppercase tracking-wider">主锁相环配置 (PLL Nodes)</h3>
          {isLoading && Object.keys(localPlls).length === 0 ? (
            <div className="text-center py-12 text-slate-500">
              <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-indigo-400 mx-auto mb-2"></div>
              正在计算时钟频率...
            </div>
          ) : (
            <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
              {Object.entries(localPlls)
                .filter(([name]) => filterNodes(name))
                .map(([name, pll]) => (
                  <div
                    key={name}
                    className="bg-slate-900/60 border border-slate-800/80 rounded-2xl p-5 flex flex-col justify-between shadow-xl relative overflow-hidden group hover:border-slate-700 transition-all"
                  >
                    <div className="absolute top-0 left-0 w-1.5 h-full bg-indigo-500" />
                    <div>
                      <div className="flex items-center justify-between">
                        <span className="font-bold text-sm text-slate-200 font-mono">{pll.name}</span>
                        <span className="font-mono font-bold text-xs text-slate-400 bg-slate-950/40 px-2 py-0.5 rounded border border-slate-800">
                          源: {pll.source}
                        </span>
                      </div>
                      <p className="text-[10px] text-slate-500 mt-0.5">级联输入频率: {pll.inputFreq.toFixed(1)} MHz</p>
                    </div>

                    {/* 配置项 */}
                    <div className="mt-4 space-y-3 bg-slate-950/40 p-3 rounded-xl border border-slate-900">
                      <div className="flex items-center justify-between text-xs">
                        <span className="text-slate-400">倍频系数 (Multiplier)</span>
                        <input
                          type="number"
                          value={pll.multiplier}
                          onChange={(e) => handlePllChange(name, "multiplier", Number(e.target.value))}
                          className="bg-slate-900 border border-slate-700/80 rounded px-2 py-0.5 text-xs text-slate-200 w-16 text-center font-mono focus:outline-none focus:border-indigo-500 focus:ring-0"
                        />
                      </div>
                      <div className="flex items-center justify-between text-xs">
                        <span className="text-slate-400">分频系数 (Divider)</span>
                        <input
                          type="number"
                          step="0.1"
                          value={pll.divider}
                          onChange={(e) => handlePllChange(name, "divider", Number(e.target.value))}
                          className="bg-slate-900 border border-slate-700/80 rounded px-2 py-0.5 text-xs text-slate-200 w-16 text-center font-mono focus:outline-none focus:border-indigo-500 focus:ring-0"
                        />
                      </div>
                    </div>

                    <div className="mt-4 flex items-center justify-between pt-3 border-t border-slate-850">
                      <span className="text-xs text-slate-400 font-semibold">生成频率</span>
                      <span className="font-mono font-bold text-indigo-400 text-base">{pll.outputFreq.toFixed(2)} MHz</span>
                    </div>
                  </div>
                ))}
            </div>
          )}
        </div>

        {/* 时钟树输出 / 叶子节点 */}
        <div className="xl:col-span-1 space-y-4">
          <h3 className="text-sm font-bold text-slate-400 uppercase tracking-wider">外设时钟总线 (Clock Outputs)</h3>
          <div className="bg-slate-900/60 border border-slate-800 rounded-2xl p-5 space-y-3 max-h-[70vh] overflow-y-auto custom-scrollbar shadow-xl">
            {Object.keys(outputs).length === 0 ? (
              <span className="text-xs text-slate-500 block text-center py-6">无可用时钟输出节点</span>
            ) : (
              Object.entries(outputs)
                .filter(([name]) => filterNodes(name))
                .map(([name, out]) => (
                  <div key={name} className="bg-slate-950/40 border border-slate-850 p-3 rounded-xl flex items-center justify-between">
                    <div>
                      <span className="font-bold text-xs text-slate-300 font-mono">{out.name}</span>
                      <p className="text-[9px] text-slate-500 mt-0.5">源: {out.source} / 分频: {out.divider}</p>
                    </div>
                    <span className="font-mono font-bold text-indigo-400 text-xs">{out.frequency.toFixed(2)} MHz</span>
                  </div>
                ))
            )}
          </div>
        </div>
      </div>
    </div>
  );
}