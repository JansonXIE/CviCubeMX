import React, { useEffect, useState } from "react";
import { useFlashStore, FlashPartition, formatFlashKb } from "../stores/flashStore";
import { Plus, Trash2, ShieldAlert, CheckCircle, RefreshCw, Save } from "lucide-react";
import { useSdkStore } from "../stores/sdkStore";

/**
 * 单个分区大小的内联编辑单元格。
 * - 输入框留空 => size 0 => 自动分配（编译期把 Flash 剩余空间全部划给该分区，如 DATA）。
 * - 其余字段（标签/文件/挂载点/类型）只读，照 defconfig 读取值展示。
 */
function SizeCell({
  partition,
  remainingKb,
  onCommit,
}: {
  partition: FlashPartition;
  remainingKb: number;
  onCommit: (sizeKb: number) => void;
}) {
  const isAuto = partition.size === 0;
  const [val, setVal] = useState<string>(isAuto ? "" : String(partition.size));

  // 外部变化（重置 / 重新读取板卡）时同步显示
  useEffect(() => {
    setVal(partition.size > 0 ? String(partition.size) : "");
  }, [partition.size]);

  const commit = (s: string) => {
    const n = parseInt(s, 10);
    onCommit(Number.isNaN(n) || n < 0 ? 0 : n);
  };

  return (
    <div className="flex items-center gap-2">
      <div className="relative">
        <input
          type="number"
          min={0}
          value={val}
          placeholder="自动"
          onChange={(e) => {
            setVal(e.target.value);
            commit(e.target.value);
          }}
          className="w-24 bg-slate-950 border border-slate-800 rounded-lg px-2 py-1 text-xs text-slate-200 font-mono focus:outline-none focus:border-indigo-500 focus:ring-0"
          title="分区大小 (KB)，留空表示自动分配剩余空间"
        />
        <span className="absolute right-2 top-1/2 -translate-y-1/2 text-[9px] text-slate-600 font-bold pointer-events-none">
          KB
        </span>
      </div>
      {isAuto ? (
        <span className="text-[11px] text-amber-400 font-semibold whitespace-nowrap">
          自动分配 · 剩余 ~{formatFlashKb(remainingKb)}
        </span>
      ) : (
        <span className="text-[11px] text-slate-500 font-mono whitespace-nowrap">
          ({formatFlashKb(partition.size)})
        </span>
      )}
    </div>
  );
}

export default function PartitionTable() {
  const { sdkPath, chipType } = useSdkStore();
  const {
    partitions,
    flashSize,
    flashSizeKb,
    isLoading,
    error,
    loadBoardInfo,
    loadPartitions,
    addPartition,
    updatePartitionSize,
    removePartition,
    resetToBaseline,
    validatePartitions,
    exportDefconfig,
  } = useFlashStore();

  const [showAddForm, setShowAddForm] = useState(false);
  const [newLabel, setNewLabel] = useState("");
  const [newSizeKb, setNewSizeKb] = useState("");
  const [newFile, setNewFile] = useState("");
  const [newMount, setNewMount] = useState("");
  const [newType, setNewType] = useState("");

  const [validationSuccess, setValidationSuccess] = useState(false);
  const [exportSuccess, setExportSuccess] = useState(false);

  // 从 SDK 对应板卡的 defconfig 读取真实分区信息（无 SDK 时回退内置默认）
  const reloadFromBoard = () => {
    if (sdkPath && chipType) {
      loadBoardInfo(sdkPath, chipType);
    } else {
      loadPartitions();
    }
  };

  // 打开页面 / 切换板卡 => 重新读取
  useEffect(() => {
    reloadFromBoard();
  }, [sdkPath, chipType]);

  // 已分配容量（size 为 0 的自动分区不计入）
  const allocatedKb = partitions
    .filter((p) => p.size > 0)
    .reduce((sum, p) => sum + p.size, 0);

  const remainingKb = Math.max(flashSizeKb - allocatedKb, 0);
  const allocatedPercent = flashSizeKb > 0 ? Math.min((allocatedKb / flashSizeKb) * 100, 100) : 0;
  const isOverflow = allocatedKb > flashSizeKb;

  const handleAddSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    const label = newLabel.trim();
    const size = parseInt(newSizeKb, 10);

    // 新增分区只强制要求 LABEL 与 SIZE
    if (!label) {
      alert("请填写分区标签 (LABEL)");
      return;
    }
    if (isNaN(size) || size <= 0) {
      alert("请输入有效的分区容量 (KB)");
      return;
    }

    // 自动分配下一个可用物理分区号
    const nextNum = partitions.length
      ? Math.max(...partitions.map((p) => p.partition_number)) + 1
      : 2;

    const newPart: FlashPartition = {
      partition_number: nextNum,
      label: label.toUpperCase(),
      size,
      size_string: formatFlashKb(size),
      file: newFile.trim(),
      mountpoint: newMount.trim(),
      type_field: newType.trim(),
      enabled: true,
    };

    addPartition(newPart);
    setShowAddForm(false);
    // 重置表单
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
          <span>分区错误: {error}</span>
        </div>
      )}

      {isOverflow && (
        <div className="flex items-center gap-2 bg-amber-500/10 border border-amber-500/30 text-amber-400 p-4 rounded-xl text-xs font-semibold">
          <ShieldAlert size={16} />
          <span>空间溢出警告：当前已分配分区总容量 ({formatFlashKb(allocatedKb)}) 超过了 Flash 总物理容量 ({formatFlashKb(flashSizeKb)})！</span>
        </div>
      )}

      {validationSuccess && (
        <div className="flex items-center gap-2 bg-emerald-500/10 border border-emerald-500/30 text-emerald-400 p-4 rounded-xl text-xs font-semibold animate-fade-in">
          <CheckCircle size={16} />
          <span>分区布局校验成功，所有分区分配均在合理空间范围内！</span>
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
            {formatFlashKb(allocatedKb)} / {formatFlashKb(flashSizeKb)} ({allocatedPercent.toFixed(1)}%)
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
            onClick={() => resetToBaseline()}
            className="flex items-center gap-1 bg-slate-800 hover:bg-slate-700 text-slate-200 px-4 py-2 rounded-xl text-xs font-bold border border-slate-700/50 transition focus:outline-none focus:ring-0 active:scale-95"
            title="撤回本次修改，恢复到载入当前板卡时的分区配置（恢复后需再次保存才会写回 defconfig）"
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
            <h4 className="font-bold text-slate-100 text-sm mb-1">新增 Flash 分区</h4>
            <p className="text-[11px] text-slate-500 mb-4">
              分区号自动分配（当前下一个为 <span className="font-mono text-indigo-400">{partitions.length ? Math.max(...partitions.map((p) => p.partition_number)) + 1 : 2}</span>）。
              标签与大小为必填，文件 / 挂载点 / 类型为选填。
            </p>
            <form onSubmit={handleAddSubmit} className="space-y-3">
              <div className="grid grid-cols-2 gap-3">
                <div>
                  <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">分区标签 (Label) *</label>
                  <input
                    type="text"
                    required
                    value={newLabel}
                    onChange={(e) => setNewLabel(e.target.value)}
                    placeholder="如: ROOTFS"
                    className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0"
                  />
                </div>
                <div>
                  <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">大小 (KB) *</label>
                  <input
                    type="number"
                    required
                    min={1}
                    value={newSizeKb}
                    onChange={(e) => setNewSizeKb(e.target.value)}
                    placeholder="如: 16384 (16M)"
                    className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0 font-mono"
                  />
                </div>
              </div>
              <div className="grid grid-cols-2 gap-3">
                <div>
                  <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">打包镜像名 (可选)</label>
                  <input
                    type="text"
                    value={newFile}
                    onChange={(e) => setNewFile(e.target.value)}
                    placeholder="如: rootfs.emmc"
                    className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0"
                  />
                </div>
                <div>
                  <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">挂载点 (可选)</label>
                  <input
                    type="text"
                    value={newMount}
                    onChange={(e) => setNewMount(e.target.value)}
                    placeholder="如: /mnt/data"
                    className="bg-slate-950 border border-slate-800 rounded-xl px-3 py-2 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 focus:ring-0"
                  />
                </div>
              </div>
              <div>
                <label className="block text-[10px] uppercase font-bold text-slate-500 tracking-wider">文件系统类型 (可选)</label>
                <input
                  type="text"
                  value={newType}
                  onChange={(e) => setNewType(e.target.value)}
                  placeholder="如: ext4 / jffs2 / squashfs"
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

      {/* 分区表格 */}
      <div className="flex-1 overflow-auto bg-slate-950/20 border border-slate-850 rounded-2xl shadow-inner max-h-[50vh] custom-scrollbar">
        <table className="w-full text-left border-collapse text-xs">
          <thead>
            <tr className="bg-slate-900/40 text-slate-400 font-bold uppercase tracking-wider border-b border-slate-800">
              <th className="px-5 py-3.5 w-16 text-center">物理编号</th>
              <th className="px-5 py-3.5">分区标签 (Label)</th>
              <th className="px-5 py-3.5">大小 (可编辑)</th>
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
                  <td className="px-5 py-3">
                    <SizeCell
                      partition={p}
                      remainingKb={remainingKb}
                      onCommit={(sizeKb) => updatePartitionSize(p.partition_number, sizeKb)}
                    />
                  </td>
                  <td className="px-5 py-3 font-mono text-slate-400">{p.file || "—"}</td>
                  <td className="px-5 py-3 text-slate-400">{p.mountpoint || "—"}</td>
                  <td className="px-5 py-3 font-mono text-slate-400">{p.type_field || "—"}</td>
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

      {/* 底部提示 */}
      <p className="text-[11px] text-slate-500">
        当前板卡 Flash 容量 <span className="font-mono text-slate-300">{flashSize}</span>。
        某分区大小留空（自动）时，编译期会把剩余空间全部划分给该分区（通常为 DATA）。
      </p>
    </div>
  );
}
