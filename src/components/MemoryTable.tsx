import React, { useEffect, useState } from "react";
import { useMemoryStore, MemoryRegion } from "../stores/memoryStore";
import { Plus, Trash2, ShieldAlert, CheckCircle, RefreshCw, Save } from "lucide-react";

// 十六进制格式化
function formatHex(val: number): string {
  return "0x" + val.toString(16).toUpperCase().padStart(8, "0");
}

// 大小可读化格式
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

export default function MemoryTable() {
  const {
    regions,
    isLoading,
    error,
    loadMemoryRegions,
    addRegion,
    removeRegion,
    validateMemory,
    exportDefconfig,
  } = useMemoryStore();

  const [showAddForm, setShowAddForm] = useState(false);
  const [newName, setNewName] = useState("");
  const [newStart, setNewStart] = useState("");
  const [newSize, setNewSize] = useState("");
  const [newDesc, setNewDesc] = useState("");

  const [validationSuccess, setValidationSuccess] = useState(false);
  const [exportSuccess, setExportSuccess] = useState(false);

  useEffect(() => {
    loadMemoryRegions();
  }, []);

  const handleAddRegionSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    if (!newName.trim() || !newStart || !newSize) return;

    // 解析地址与大小
    const start = parseInt(newStart, 16);
    const size = parseInt(newSize, 16); // 支持输入 0x1000 或数字
    if (isNaN(start) || isNaN(size) || size <= 0) {
      alert("请输入有效的起始地址和大小数值 (支持十六进制前缀 0x)");
      return;
    }

    const end = start + size;
    const sizeStr = formatSizeReadable(size);

    const newReg: MemoryRegion = {
      name: newName.trim().toUpperCase(),
      start_address: start,
      end_address: end,
      size,
      size_string: sizeStr,
      is_editable: true,
      description: newDesc,
    };

    addRegion(newReg);
    setShowAddForm(false);
    // 重置表单
    setNewName("");
    setNewStart("");
    setNewSize("");
    setNewDesc("");
  };

  const handleValidate = async () => {
    setValidationSuccess(false);
    try {
      await validateMemory();
      setValidationSuccess(true);
      setTimeout(() => setValidationSuccess(false), 3000);
    } catch (err) {
      console.error(err);
    }
  };

  const handleExport = async () => {
    setExportSuccess(false);
    try {
      // 传递 mock 路径及默认芯片型号
      await exportDefconfig("boards_pinout/cv1842hp/defconfig", "cv1842hp");
      setExportSuccess(true);
      setTimeout(() => setExportSuccess(false), 3000);
    } catch (err) {
      console.error(err);
    }
  };

  return (
    <div className="flex flex-col h-full space-y-4">
      {/* 警示与状态面板 */}
      {error && (
        <div className="flex items-center gap-2 bg-rose-500/10 border border-rose-500/30 text-rose-400 p-4 rounded-xl text-xs font-semibold animate-shake">
          <ShieldAlert size={16} />
          <span>布局校验错误: {error}</span>
        </div>
      )}

      {validationSuccess && (
        <div className="flex items-center gap-2 bg-emerald-500/10 border border-emerald-500/30 text-emerald-400 p-4 rounded-xl text-xs font-semibold animate-fade-in">
          <CheckCircle size={16} />
          <span>内存布局校验成功，所有内存块分布合理且无重叠！</span>
        </div>
      )}

      {exportSuccess && (
        <div className="flex items-center gap-2 bg-indigo-500/10 border border-indigo-500/30 text-indigo-400 p-4 rounded-xl text-xs font-semibold animate-fade-in">
          <CheckCircle size={16} />
          <span>配置成功导出至 boards_pinout/cv1842hp/defconfig ！</span>
        </div>
      )}

      {/* 顶栏控制组 */}
      <div className="flex items-center justify-between">
        <h3 className="text-base font-bold text-slate-300">内存区域映射表</h3>
        <div className="flex items-center gap-2">
          <button
            type="button"
            onClick={() => loadMemoryRegions()}
            className="flex items-center gap-1 bg-slate-800 hover:bg-slate-700 text-slate-200 px-4 py-2 rounded-xl text-xs font-bold border border-slate-700/50 transition focus:outline-none focus:ring-0 active:scale-95"
          >
            <RefreshCw size={14} />
            重置
          </button>
          <button
            type="button"
            onClick={handleValidate}
            className="bg-emerald-600 hover:bg-emerald-500 text-white px-4 py-2 rounded-xl text-xs font-bold shadow-lg shadow-emerald-500/20 transition focus:outline-none focus:ring-0 active:scale-95"
          >
            校验内存
          </button>
          <button
            type="button"
            onClick={handleExport}
            className="flex items-center gap-1 bg-indigo-600 hover:bg-indigo-500 text-white px-4 py-2 rounded-xl text-xs font-bold shadow-lg shadow-indigo-500/20 transition focus:outline-none focus:ring-0 active:scale-95"
          >
            <Save size={14} />
            保存并导出
          </button>
          <button
            type="button"
            onClick={() => setShowAddForm(true)}
            className="flex items-center gap-1 bg-indigo-500 hover:bg-indigo-400 text-white px-4 py-2 rounded-xl text-xs font-bold shadow-lg shadow-indigo-500/20 transition focus:outline-none focus:ring-0 active:scale-95"
          >
            <Plus size={14} />
            新增区域
          </button>
        </div>
      </div>

      {/* 新增区域悬浮表单 */}
      {showAddForm && (
        <div className="fixed inset-0 z-50 flex items-center justify-center bg-slate-950/70 backdrop-blur-sm">
          <div className="bg-slate-900/95 border border-slate-700/50 rounded-2xl w-full max-w-md p-6 shadow-2xl relative">
            <h4 className="font-bold text-slate-100 text-sm mb-4">新增内存分配区域</h4>
            <form onSubmit={handleAddRegionSubmit} className="space-y-3">
              <div>
                <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">区域名称</label>
                <input
                  type="text"
                  required
                  value={newName}
                  onChange={(e) => setNewName(e.target.value)}
                  placeholder="如: ION_MEMORY"
                  className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0"
                />
              </div>
              <div className="grid grid-cols-2 gap-3">
                <div>
                  <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">起始地址 (Hex)</label>
                  <input
                    type="text"
                    required
                    value={newStart}
                    onChange={(e) => setNewStart(e.target.value)}
                    placeholder="如: 0x82000000"
                    className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0 font-mono"
                  />
                </div>
                <div>
                  <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">大小 (Hex / Dec)</label>
                  <input
                    type="text"
                    required
                    value={newSize}
                    onChange={(e) => setNewSize(e.target.value)}
                    placeholder="如: 0x1000000 (16M)"
                    className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0 font-mono"
                  />
                </div>
              </div>
              <div>
                <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">说明描述</label>
                <input
                  type="text"
                  value={newDesc}
                  onChange={(e) => setNewDesc(e.target.value)}
                  placeholder="选填说明"
                  className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0"
                />
              </div>
              <div className="flex justify-end gap-2 pt-3 border-t border-slate-800 mt-4">
                <button
                  type="button"
                  onClick={() => setShowAddForm(false)}
                  className="bg-slate-800 hover:bg-slate-700 text-slate-300 px-4 py-2 rounded-xl text-xs font-semibold focus:outline-none focus:ring-0"
                >
                  取消
                </button>
                <button
                  type="submit"
                  className="bg-indigo-600 hover:bg-indigo-500 text-white px-5 py-2 rounded-xl text-xs font-bold shadow-lg shadow-indigo-500/20 focus:outline-none focus:ring-0"
                >
                  确认添加
                </button>
              </div>
            </form>
          </div>
        </div>
      )}

      {/* 数据表格区域 */}
      <div className="flex-1 overflow-auto bg-slate-950/20 border border-slate-850 rounded-2xl shadow-inner max-h-[60vh] custom-scrollbar">
        <table className="w-full text-left border-collapse text-xs">
          <thead>
            <tr className="bg-slate-900/40 text-slate-400 font-bold uppercase tracking-wider border-b border-slate-800">
              <th className="px-5 py-3.5">区域名称</th>
              <th className="px-5 py-3.5">起始地址</th>
              <th className="px-5 py-3.5">结束地址</th>
              <th className="px-5 py-3.5">可读大小</th>
              <th className="px-5 py-3.5">描述</th>
              <th className="px-5 py-3.5 text-center">编辑性</th>
              <th className="px-5 py-3.5 text-right">操作</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-slate-800/40">
            {isLoading ? (
              <tr>
                <td colSpan={7} className="text-center py-8 text-slate-500">
                  <div className="animate-spin rounded-full h-6 w-6 border-b-2 border-indigo-400 mx-auto mb-2"></div>
                  数据加载中...
                </td>
              </tr>
            ) : regions.length === 0 ? (
              <tr>
                <td colSpan={7} className="text-center py-8 text-slate-500">
                  无可用内存区域数据
                </td>
              </tr>
            ) : (
              regions.map((r) => (
                <tr key={r.name} className="hover:bg-slate-900/10 text-slate-300 transition-colors">
                  <td className="px-5 py-3 font-mono font-bold text-slate-200">{r.name}</td>
                  <td className="px-5 py-3 font-mono text-slate-400">{formatHex(r.start_address)}</td>
                  <td className="px-5 py-3 font-mono text-slate-400">{formatHex(r.end_address)}</td>
                  <td className="px-5 py-3 font-mono text-slate-200">{r.size_string}</td>
                  <td className="px-5 py-3 text-slate-400">{r.description || "—"}</td>
                  <td className="px-5 py-3 text-center">
                    <span
                      className={`inline-block px-2 py-0.5 rounded-full text-[9px] font-bold ${
                        r.is_editable ? "bg-indigo-500/10 text-indigo-400" : "bg-slate-800 text-slate-500"
                      }`}
                    >
                      {r.is_editable ? "自定义" : "系统内置"}
                    </span>
                  </td>
                  <td className="px-5 py-3 text-right">
                    {r.is_editable ? (
                      <button
                        type="button"
                        onClick={() => removeRegion(r.name)}
                        className="text-rose-400 hover:text-rose-300 p-1.5 rounded-lg hover:bg-slate-800 transition focus:outline-none focus:ring-0"
                        title="删除此内存区域"
                      >
                        <Trash2 size={14} />
                      </button>
                    ) : (
                      <span className="text-slate-600 select-none text-[10px]">不可更改</span>
                    )}
                  </td>
                </tr>
              ))
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
}
