import React, { useEffect, useState } from "react";
import { useFlashStore, FlashPartition } from "../stores/flashStore";
import { Plus, Trash2, ShieldAlert, CheckCircle, RefreshCw, Save } from "lucide-react";
import { useSdkStore } from "../stores/sdkStore";

// Flash 基准容量: 32MB = 32768KB
const FLASH_TOTAL_CAPACITY_KB = 32768;

function formatKbToMb(kb: number): string {
  const mb = kb / 1024;
  return Number.isInteger(mb) ? `${mb}MB` : `${mb.toFixed(2)}MB`;
}

export default function PartitionTable() {
  const { sdkPath, chipType } = useSdkStore();
  const {
    partitions,
    isLoading,
    error,
    loadPartitions,
    addPartition,
    removePartition,
    validatePartitions,
    exportDefconfig,
  } = useFlashStore();

  const [showAddForm, setShowAddForm] = useState(false);
  const [newNum, setNewNum] = useState("");
  const [newLabel, setNewLabel] = useState("");
  const [newSizeKb, setNewSizeKb] = useState("");
  const [newFile, setNewFile] = useState("");
  const [newMount, setNewMount] = useState("");
  const [newType, setNewType] = useState("");

  const [validationSuccess, setValidationSuccess] = useState(false);
  const [exportSuccess, setExportSuccess] = useState(false);

  useEffect(() => {
    loadPartitions();
  }, []);

  // 计算已启用的分区总容量
  const allocatedKb = partitions
    .filter((p) => p.enabled)
    .reduce((sum, p) => sum + p.size, 0);

  const allocatedPercent = Math.min((allocatedKb / FLASH_TOTAL_CAPACITY_KB) * 100, 100);
  const isOverflow = allocatedKb > FLASH_TOTAL_CAPACITY_KB;

  const handleAddSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    const num = parseInt(newNum, 10);
    const size = parseInt(newSizeKb, 10);

    if (isNaN(num) || num < 0) {
      alert("请输入有效的物理分区编号");
      return;
    }
    if (isNaN(size) || size <= 0) {
      alert("请输入有效的分区容量 (KB)");
      return;
    }

    if (partitions.some((p) => p.partition_number === num)) {
      alert(`物理分区号 ${num} 已存在，请使用其它编号`);
      return;
    }

    const newPart: FlashPartition = {
      partition_number: num,
      label: newLabel.trim().toUpperCase() || `PART_${num}`,
      size,
      size_string: `${size}K`,
      file: newFile.trim(),
      mountpoint: newMount.trim(),
      type_field: newType.trim() || "squashfs",
      enabled: true,
    };

    addPartition(newPart);
    setShowAddForm(false);
    // 重置表单
    setNewNum("");
    setNewLabel("");
    setNewSizeKb("");
    setNewFile("");
    setNewMount("");
    setNewType("");
  };

  const handleValidate = async () => {
    setValidationSuccess(false);
    try {
      await validatePartitions();
      setValidationSuccess(true);
      setTimeout(() => setValidationSuccess(false), 3000);
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
      {/* 警告面板 */}
      {error && (
        <div className="flex items-center gap-2 bg-rose-500/10 border border-rose-500/30 text-rose-400 p-4 rounded-xl text-xs font-semibold animate-shake">
          <ShieldAlert size={16} />
          <span>分区校验错误: {error}</span>
        </div>
      )}

      {isOverflow && (
        <div className="flex items-center gap-2 bg-amber-500/10 border border-amber-500/30 text-amber-400 p-4 rounded-xl text-xs font-semibold">
          <ShieldAlert size={16} />
          <span>空间溢出警告：当前已分配分区总容量 ({formatKbToMb(allocatedKb)}) 超过了 Flash 硬件总物理容量 ({formatKbToMb(FLASH_TOTAL_CAPACITY_KB)})！</span>
        </div>
      )}

      {validationSuccess && (
        <div className="flex items-center gap-2 bg-emerald-500/10 border border-emerald-500/30 text-emerald-400 p-4 rounded-xl text-xs font-semibold animate-fade-in">
          <CheckCircle size={16} />
          <span>分区布局校验成功，所有启用分区分配均在合理空间范围内！</span>
        </div>
      )}

      {exportSuccess && (
        <div className="flex items-center gap-2 bg-indigo-500/10 border border-indigo-500/30 text-indigo-400 p-4 rounded-xl text-xs font-semibold animate-fade-in">
          <CheckCircle size={16} />
          <span>配置成功同步导出至 SDK 中的 defconfig ！</span>
        </div>
      )}

      {/* 容量分配指示器 */}
      <div className="bg-slate-900/40 border border-slate-800 rounded-2xl p-4 flex flex-col gap-2 relative overflow-hidden">
        <div className="flex items-center justify-between text-xs">
          <span className="font-semibold text-slate-400">Flash 容量分配状态</span>
          <span className="font-mono font-bold text-slate-200">
            {formatKbToMb(allocatedKb)} / {formatKbToMb(FLASH_TOTAL_CAPACITY_KB)} ({((allocatedKb / FLASH_TOTAL_CAPACITY_KB) * 100).toFixed(1)}%)
          </span>
        </div>
        <div className="w-full bg-slate-950 h-3 rounded-full overflow-hidden border border-slate-800 p-0.5">
          <div
            className={`h-full rounded-full transition-all duration-500 ${
              isOverflow ? "bg-rose-500 shadow-[0_0_10px_rgba(239,68,68,0.5)]" : "bg-indigo-500 shadow-[0_0_10px_rgba(99,102,241,0.5)]"
            }`}
            style={{ width: `${allocatedPercent}%` }}
          />
        </div>
      </div>

      {/* 顶栏控制组 */}
      <div className="flex items-center justify-between">
        <h3 className="text-base font-bold text-slate-300">Flash 分区表</h3>
        <div className="flex items-center gap-2">
          <button
            type="button"
            onClick={() => loadPartitions()}
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
            校验分区
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
            新增分区
          </button>
        </div>
      </div>

      {/* 新增分区表单 */}
      {showAddForm && (
        <div className="fixed inset-0 z-50 flex items-center justify-center bg-slate-950/70 backdrop-blur-sm">
          <div className="bg-slate-900/95 border border-slate-700/50 rounded-2xl w-full max-w-md p-6 shadow-2xl relative">
            <h4 className="font-bold text-slate-100 text-sm mb-4">新增 Flash 分区</h4>
            <form onSubmit={handleAddSubmit} className="space-y-3">
              <div className="grid grid-cols-2 gap-3">
                <div>
                  <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">分区编号</label>
                  <input
                    type="number"
                    required
                    value={newNum}
                    onChange={(e) => setNewNum(e.target.value)}
                    placeholder="如: 3"
                    className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0"
                  />
                </div>
                <div>
                  <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">分区标签 (Label)</label>
                  <input
                    type="text"
                    required
                    value={newLabel}
                    onChange={(e) => setNewLabel(e.target.value)}
                    placeholder="如: ROOTFS"
                    className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0"
                  />
                </div>
              </div>
              <div className="grid grid-cols-2 gap-3">
                <div>
                  <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">大小 (KB)</label>
                  <input
                    type="number"
                    required
                    value={newSizeKb}
                    onChange={(e) => setNewSizeKb(e.target.value)}
                    placeholder="如: 16384 (16M)"
                    className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0 font-mono"
                  />
                </div>
                <div>
                  <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">文件系统类型</label>
                  <input
                    type="text"
                    value={newType}
                    onChange={(e) => setNewType(e.target.value)}
                    placeholder="如: squashfs / ext4"
                    className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0"
                  />
                </div>
              </div>
              <div className="grid grid-cols-2 gap-3">
                <div>
                  <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">打包镜像名</label>
                  <input
                    type="text"
                    value={newFile}
                    onChange={(e) => setNewFile(e.target.value)}
                    placeholder="如: rootfs.sqsh (可选)"
                    className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0"
                  />
                </div>
                <div>
                  <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">挂载点 (Mountpoint)</label>
                  <input
                    type="text"
                    value={newMount}
                    onChange={(e) => setNewMount(e.target.value)}
                    placeholder="如: / (可选)"
                    className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0"
                  />
                </div>
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

      {/* 分区表格 */}
      <div className="flex-1 overflow-auto bg-slate-950/20 border border-slate-850 rounded-2xl shadow-inner max-h-[50vh] custom-scrollbar">
        <table className="w-full text-left border-collapse text-xs">
          <thead>
            <tr className="bg-slate-900/40 text-slate-400 font-bold uppercase tracking-wider border-b border-slate-800">
              <th className="px-5 py-3.5 w-16 text-center">物理编号</th>
              <th className="px-5 py-3.5">分区标签 (Label)</th>
              <th className="px-5 py-3.5">物理容量</th>
              <th className="px-5 py-3.5">镜像名</th>
              <th className="px-5 py-3.5">挂载点</th>
              <th className="px-5 py-3.5">类型</th>
              <th className="px-5 py-3.5 text-right">操作</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-slate-800/40">
            {isLoading ? (
              <tr>
                <td colSpan={7} className="text-center py-8 text-slate-500">
                  <div className="animate-spin rounded-full h-6 w-6 border-b-2 border-indigo-400 mx-auto mb-2"></div>
                  正在载入分区...
                </td>
              </tr>
            ) : partitions.length === 0 ? (
              <tr>
                <td colSpan={7} className="text-center py-8 text-slate-500">
                  无可用 Flash 分区数据
                </td>
              </tr>
            ) : (
              partitions.map((p) => (
                <tr key={p.partition_number} className="hover:bg-slate-900/10 text-slate-300 transition-colors">
                  <td className="px-5 py-3 text-center font-mono font-bold text-slate-400">{p.partition_number}</td>
                  <td className="px-5 py-3 font-mono font-bold text-indigo-400">{p.label}</td>
                  <td className="px-5 py-3 font-mono text-slate-200">
                    {p.size_string} ({formatKbToMb(p.size)})
                  </td>
                  <td className="px-5 py-3 font-mono text-slate-400">{p.file || "—"}</td>
                  <td className="px-5 py-3 text-slate-400">{p.mountpoint || "—"}</td>
                  <td className="px-5 py-3 font-mono text-slate-400">{p.type_field}</td>
                  <td className="px-5 py-3 text-right">
                    <button
                      type="button"
                      onClick={() => removePartition(p.partition_number)}
                      className="text-rose-400 hover:text-rose-300 p-1.5 rounded-lg hover:bg-slate-800 transition focus:outline-none focus:ring-0"
                      title="删除分区"
                    >
                      <Trash2 size={14} />
                    </button>
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
