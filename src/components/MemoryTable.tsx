import React, { useEffect, useState } from "react";
import { useMemoryStore, MemoryRegion } from "../stores/memoryStore";
import { Plus, Trash2, ShieldAlert, CheckCircle, RefreshCw, Save, Edit2, AlertTriangle } from "lucide-react";
import { useSdkStore } from "../stores/sdkStore";

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

const BUILTIN_REGIONS = [
  "MONITOR", "KERNEL_MEMORY", "FSBL_C906L_START", "OPENSBI_FDT", "RTOS_LOG",
  "SHARE_MEM", "SHARE_PARAM", "PQBIN", "RTOS_LOGO", "CVI_UPDATE_HEADER",
  "FSBL_UNZIP", "UIMAG", "RTOS_COMPRESS_BIN", "H26X_BITSTREAM", "H26X_ENC_BUFF",
  "ION", "ISP_MEM_BASE", "BOOTLOGO", "RTOS_ION"
];

interface MemoryTableProps {
  hoveredRegion?: string | null;
  onHoverRegion?: (name: string | null) => void;
}

export default function MemoryTable({
  hoveredRegion = null,
  onHoverRegion = () => {},
}: MemoryTableProps) {
  const { sdkPath, chipType } = useSdkStore();
  const {
    regions,
    isLoading,
    error,
    warnings,
    loadMemoryRegions,
    addRegion,
    removeRegion,
    updateRegion,
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

  const [showEditForm, setShowEditForm] = useState(false);
  const [editTarget, setEditTarget] = useState<MemoryRegion | null>(null);
  const [editStart, setEditStart] = useState("");
  const [editSize, setEditSize] = useState("");
  const [editDesc, setEditDesc] = useState("");

  const handleEditClick = (r: MemoryRegion) => {
    setEditTarget(r);
    setEditStart(formatHex(r.start_address));
    setEditSize(formatHex(r.size));
    setEditDesc(r.description || "");
    setShowEditForm(true);
  };

  const handleEditSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    if (!editTarget) return;

    const start = editStart.toLowerCase().startsWith("0x")
      ? parseInt(editStart, 16)
      : parseInt(editStart, 10);

    const size = editSize.toLowerCase().startsWith("0x")
      ? parseInt(editSize, 16)
      : parseInt(editSize, 10);

    if (isNaN(start) || isNaN(size) || size < 0) {
      alert("请输入有效的物理地址与大小数值 (支持十六进制前缀 0x)");
      return;
    }

    const end = start + size;
    const sizeStr = formatSizeReadable(size);

    updateRegion(editTarget.name, {
      start_address: start,
      end_address: end,
      size,
      size_string: sizeStr,
      description: editDesc.trim(),
    });

    setShowEditForm(false);
    setEditTarget(null);
  };

  const handleValidate = async () => {
    setValidationSuccess(false);
    try {
      // 校验通过返回重叠警告列表；仅在“无重叠”时才显示绿色完全通过提示，
      // 有重叠时由下方黄色提示条展示（重叠为信息性，不阻断）
      const w = await validateMemory();
      if (!w || w.length === 0) {
        setValidationSuccess(true);
        setTimeout(() => setValidationSuccess(false), 3000);
      }
    } catch (err) {
      console.error(err);
    }
  };

  const handleExport = async () => {
    if (!sdkPath || !chipType) {
      alert("请先在顶部配置全局 SDK 源码路径和芯片型号！");
      return;
    }
    setExportSuccess(false);
    try {
      await exportDefconfig(sdkPath, chipType);
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

      {warnings.length > 0 && (
        <div className="bg-amber-500/10 border border-amber-500/30 text-amber-300 p-4 rounded-xl text-xs animate-fade-in">
          <div className="flex items-center gap-2 font-semibold mb-1.5">
            <AlertTriangle size={16} />
            <span>校验通过，但检测到 {warnings.length} 处区域重叠（提示性，不阻止导出）</span>
          </div>
          <ul className="list-disc list-inside space-y-0.5 text-amber-400/90 font-mono text-[11px] pl-1">
            {warnings.map((w, i) => (
              <li key={i}>{w}</li>
            ))}
          </ul>
          <p className="text-amber-400/70 mt-1.5 text-[10px] leading-relaxed">
            说明：KERNEL_MEMORY 覆盖整块 DDR、FSBL_UNZIP 与 UIMAG 同址分时复用、ION 子缓冲等属正常设计内的重叠；
            请确认上述并非真正的内存互踩后再导出。
          </p>
        </div>
      )}

      {exportSuccess && (
        <div className="flex items-center gap-2 bg-indigo-500/10 border border-indigo-500/30 text-indigo-400 p-4 rounded-xl text-xs font-semibold animate-fade-in">
          <CheckCircle size={16} />
          <span>配置成功同步导出至 SDK 中的 defconfig ！</span>
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
      {/* 编辑区域悬浮表单 */}
      {showEditForm && editTarget && (
        <div className="fixed inset-0 z-50 flex items-center justify-center bg-slate-950/70 backdrop-blur-sm animate-fade-in">
          <div className="bg-slate-900/95 border border-slate-700/50 rounded-2xl w-full max-w-md p-6 shadow-2xl relative">
            <h4 className="font-bold text-slate-100 text-sm mb-4">
              编辑内存分配区域: <span className="text-indigo-400 font-mono">{editTarget.name}</span>
            </h4>
            <form onSubmit={handleEditSubmit} className="space-y-3">
              <div>
                <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">区域名称 (不可修改)</label>
                <input
                  type="text"
                  disabled
                  value={editTarget.name}
                  className="bg-slate-950/50 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-500 w-full cursor-not-allowed font-mono"
                />
              </div>
              <div className="grid grid-cols-2 gap-3">
                <div>
                  <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">
                    起始地址 (Hex)
                    {["ION", "H26X_BITSTREAM", "H26X_ENC_BUFF", "ISP_MEM_BASE"].includes(editTarget.name) && (
                      <span className="text-indigo-400 font-semibold lowercase normal-case"> (只读/级联绑定)</span>
                    )}
                  </label>
                  <input
                    type="text"
                    required
                    disabled={["ION", "H26X_BITSTREAM", "H26X_ENC_BUFF", "ISP_MEM_BASE"].includes(editTarget.name)}
                    value={editStart}
                    onChange={(e) => setEditStart(e.target.value)}
                    placeholder="如: 0x82000000"
                    className={`bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs w-full focus:outline-none focus:border-indigo-500 focus:ring-0 font-mono ${
                      ["ION", "H26X_BITSTREAM", "H26X_ENC_BUFF", "ISP_MEM_BASE"].includes(editTarget.name)
                        ? "cursor-not-allowed opacity-50 text-slate-500 bg-slate-950/40"
                        : "text-slate-200"
                    }`}
                  />
                </div>
                <div>
                  <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">大小 (Hex / Dec)</label>
                  <input
                    type="text"
                    required
                    value={editSize}
                    onChange={(e) => setEditSize(e.target.value)}
                    placeholder="如: 0x1000000 (16M)"
                    className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0 font-mono"
                  />
                </div>
              </div>
              <div>
                <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">说明描述</label>
                <input
                  type="text"
                  value={editDesc}
                  onChange={(e) => setEditDesc(e.target.value)}
                  placeholder="选填说明"
                  className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0"
                />
              </div>
              <div className="flex justify-end gap-2 pt-3 border-t border-slate-800 mt-4">
                <button
                  type="button"
                  onClick={() => {
                    setShowEditForm(false);
                    setEditTarget(null);
                  }}
                  className="bg-slate-800 hover:bg-slate-700 text-slate-300 px-4 py-2 rounded-xl text-xs font-semibold focus:outline-none focus:ring-0 active:scale-95 transition"
                >
                  取消
                </button>
                <button
                  type="submit"
                  className="bg-indigo-600 hover:bg-indigo-500 text-white px-5 py-2 rounded-xl text-xs font-bold shadow-lg shadow-indigo-500/20 focus:outline-none focus:ring-0 active:scale-95 transition"
                >
                  确认修改
                </button>
              </div>
            </form>
          </div>
        </div>
      )}

      {/* 数据表格区域 */}
      <div className="flex-1 overflow-auto bg-slate-950/20 border border-slate-850 rounded-2xl shadow-inner custom-scrollbar min-h-0">
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
              regions.map((r) => {
                const isHovered = hoveredRegion === r.name;
                return (
                  <tr
                    key={r.name}
                    className={`transition-colors duration-150 cursor-pointer ${
                      isHovered ? "bg-indigo-500/10 text-slate-100" : "hover:bg-slate-900/10 text-slate-300"
                    }`}
                    onMouseEnter={() => onHoverRegion(r.name)}
                    onMouseLeave={() => onHoverRegion(null)}
                  >
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
                      <div className="flex justify-end items-center gap-1">
                        {r.is_editable && (
                          <button
                            type="button"
                            onClick={() => handleEditClick(r)}
                            className="text-indigo-400 hover:text-indigo-300 p-1.5 rounded-lg hover:bg-slate-800 transition focus:outline-none focus:ring-0 active:scale-95"
                            title="编辑此内存区域大小/起始物理地址"
                          >
                            <Edit2 size={14} />
                          </button>
                        )}
                        {r.is_editable && !BUILTIN_REGIONS.includes(r.name) ? (
                          <button
                            type="button"
                            onClick={() => removeRegion(r.name)}
                            className="text-rose-400 hover:text-rose-300 p-1.5 rounded-lg hover:bg-slate-800 transition focus:outline-none focus:ring-0 active:scale-95"
                            title="删除此内存区域"
                          >
                            <Trash2 size={14} />
                          </button>
                        ) : r.is_editable ? (
                          <span className="text-[10px] text-slate-500 select-none mr-2 font-medium">内置核心</span>
                        ) : (
                          <span className="text-slate-600 select-none text-[10px]">不可更改</span>
                        )}
                      </div>
                    </td>
                  </tr>
                );
              })
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
}
