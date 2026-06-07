import React, { useState, useEffect } from "react";
import { usePeripheralStore, PeripheralInfo } from "../stores/peripheralStore";
import { X, Save } from "lucide-react";

interface ConfigDialogProps {
  peripheral: PeripheralInfo;
  isOpen: boolean;
  onClose: () => void;
}

export default function ConfigDialog({ peripheral, isOpen, onClose }: ConfigDialogProps) {
  const {
    setPeripheralStatus,
    setClockFrequency,
    setPwmCells,
    setCurrentSpeed,
    setSysdmaChannels,
  } = usePeripheralStore();

  const [status, setStatus] = useState(peripheral.status);
  const [clockFreq, setClockFreq] = useState(peripheral.clock_frequency);
  const [pwmCells, setPwmCellsVal] = useState(peripheral.pwm_cells);
  const [currentSpeed, setCurrentSpeedVal] = useState(peripheral.current_speed);
  const [sysdmaChs, setSysdmaChs] = useState<string[]>(
    peripheral.sysdma_channels.length === 8
      ? [...peripheral.sysdma_channels]
      : ["0", "0", "0", "0", "0", "0", "0", "0"]
  );

  const [isSaving, setIsSaving] = useState(false);

  useEffect(() => {
    setStatus(peripheral.status);
    setClockFreq(peripheral.clock_frequency);
    setPwmCellsVal(peripheral.pwm_cells);
    setCurrentSpeedVal(peripheral.current_speed);
    setSysdmaChs(
      peripheral.sysdma_channels.length === 8
        ? [...peripheral.sysdma_channels]
        : ["0", "0", "0", "0", "0", "0", "0", "0"]
    );
  }, [peripheral]);

  if (!isOpen) return null;

  const handleSave = async (e: React.FormEvent) => {
    e.preventDefault();
    setIsSaving(true);
    try {
      // 逐个保存有变更且符合条件的值
      if (peripheral.has_status && status !== peripheral.status) {
        await setPeripheralStatus(peripheral.name, status);
      }
      if (peripheral.has_clock_freq && clockFreq !== peripheral.clock_frequency) {
        await setClockFrequency(peripheral.name, clockFreq);
      }
      if (peripheral.has_pwm_cells && pwmCells !== peripheral.pwm_cells) {
        await setPwmCells(peripheral.name, pwmCells);
      }
      if (peripheral.has_current_speed && currentSpeed !== peripheral.current_speed) {
        await setCurrentSpeed(peripheral.name, currentSpeed);
      }
      if (peripheral.has_sysdma_channels) {
        await setSysdmaChannels(peripheral.name, sysdmaChs);
      }
      onClose();
    } catch (err) {
      console.error(err);
    } finally {
      setIsSaving(false);
    }
  };

  const handleDmaChChange = (index: number, val: string) => {
    const next = [...sysdmaChs];
    next[index] = val;
    setSysdmaChs(next);
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center bg-slate-950/70 backdrop-blur-sm animate-fade-in">
      <div className="bg-slate-900/95 border border-slate-700/50 rounded-2xl w-full max-w-lg p-6 shadow-2xl relative overflow-hidden flex flex-col">
        {/* 背景光亮 */}
        <div className="absolute top-[-20%] left-[-20%] w-[50%] h-[50%] rounded-full bg-indigo-500/10 blur-[100px] pointer-events-none" />

        {/* 头部 */}
        <div className="flex items-center justify-between border-b border-slate-800 pb-4 relative z-10">
          <h3 className="font-bold text-lg text-slate-100 flex items-center gap-2">
            配置外设: <span className="font-mono text-indigo-400 font-bold">{peripheral.name}</span>
          </h3>
          <button
            type="button"
            onClick={onClose}
            className="text-slate-400 hover:text-white p-1 rounded-lg hover:bg-slate-800/80 transition focus:outline-none focus:ring-0"
          >
            <X size={18} />
          </button>
        </div>

        {/* 表单内容 */}
        <form onSubmit={handleSave} className="mt-4 space-y-4 flex-1 relative z-10">
          {/* status 属性 */}
          {peripheral.has_status && (
            <div className="flex items-center justify-between bg-slate-950/30 p-3.5 rounded-xl border border-slate-800">
              <div>
                <label className="block text-xs font-bold uppercase text-slate-400 tracking-wider">
                  外设使能状态 (status)
                </label>
                <span className="text-[10px] text-slate-500 font-medium">控制 DTS 中此节点的 status 属性值</span>
              </div>
              <select
                value={status}
                onChange={(e) => setStatus(e.target.value)}
                className="bg-slate-900 border border-slate-700/80 rounded-lg px-3 py-1.5 text-xs text-slate-200 focus:outline-none focus:border-indigo-500 transition focus:ring-0"
              >
                <option value="okay">okay (启用)</option>
                <option value="disabled">disabled (禁用)</option>
              </select>
            </div>
          )}

          {/* clock-frequency 属性 */}
          {peripheral.has_clock_freq && (
            <div className="flex flex-col gap-1.5 bg-slate-950/30 p-3.5 rounded-xl border border-slate-800">
              <label className="block text-xs font-bold uppercase text-slate-400 tracking-wider">
                工作频率 (clock-frequency)
              </label>
              <div className="flex items-center gap-2">
                <input
                  type="number"
                  value={clockFreq}
                  onChange={(e) => setClockFreq(Number(e.target.value))}
                  className="bg-slate-900 border border-slate-700/80 rounded-lg px-3 py-1.5 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 transition focus:ring-0 font-mono"
                  placeholder="请输入时钟频率，如 100000"
                />
                <span className="text-xs text-slate-500 font-semibold font-mono">Hz</span>
              </div>
            </div>
          )}

          {/* #pwm-cells 属性 */}
          {peripheral.has_pwm_cells && (
            <div className="flex flex-col gap-1.5 bg-slate-950/30 p-3.5 rounded-xl border border-slate-800">
              <label className="block text-xs font-bold uppercase text-slate-400 tracking-wider">
                PWM 单元大小 (#pwm-cells)
              </label>
              <input
                type="number"
                value={pwmCells}
                onChange={(e) => setPwmCellsVal(Number(e.target.value))}
                className="bg-slate-900 border border-slate-700/80 rounded-lg px-3 py-1.5 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 transition focus:ring-0 font-mono"
                placeholder="请输入单元大小，一般为 2 或 3"
              />
            </div>
          )}

          {/* current-speed 属性 */}
          {peripheral.has_current_speed && (
            <div className="flex flex-col gap-1.5 bg-slate-950/30 p-3.5 rounded-xl border border-slate-800">
              <label className="block text-xs font-bold uppercase text-slate-400 tracking-wider">
                波特率 (current-speed)
              </label>
              <select
                value={currentSpeed}
                onChange={(e) => setCurrentSpeedVal(Number(e.target.value))}
                className="bg-slate-900 border border-slate-700/80 rounded-lg px-3 py-1.5 text-xs text-slate-200 w-full focus:outline-none focus:border-indigo-500 transition focus:ring-0 font-mono"
              >
                <option value={9600}>9600 bps</option>
                <option value={19200}>19200 bps</option>
                <option value={38400}>38400 bps</option>
                <option value={57600}>57600 bps</option>
                <option value={115200}>115200 bps</option>
                <option value={921600}>921600 bps</option>
              </select>
            </div>
          )}

          {/* sysdma_channels 属性 */}
          {peripheral.has_sysdma_channels && (
            <div className="flex flex-col gap-2 bg-slate-950/30 p-3.5 rounded-xl border border-slate-800">
              <div>
                <label className="block text-xs font-bold uppercase text-slate-400 tracking-wider">
                  DMA 通道重映射 (ch-remap)
                </label>
                <span className="text-[10px] text-slate-500 font-medium">配置 8 个通道各自对应的硬件映射通道 ID</span>
              </div>
              <div className="grid grid-cols-4 gap-2">
                {sysdmaChs.map((ch, idx) => (
                  <div key={`dma-ch-${idx}`} className="flex flex-col gap-1">
                    <span className="text-[10px] text-slate-500 font-mono font-bold">通道 {idx}</span>
                    <input
                      type="text"
                      value={ch}
                      onChange={(e) => handleDmaChChange(idx, e.target.value)}
                      className="bg-slate-900 border border-slate-700/80 rounded-lg px-2 py-1 text-xs text-slate-200 focus:outline-none focus:border-indigo-500 transition focus:ring-0 font-mono text-center"
                    />
                  </div>
                ))}
              </div>
            </div>
          )}

          {/* 底部操作按钮 */}
          <div className="flex items-center justify-end gap-3 border-t border-slate-800 pt-4 mt-6">
            <button
              type="button"
              onClick={onClose}
              className="bg-slate-800 hover:bg-slate-700 text-slate-300 px-4 py-2 rounded-lg text-xs font-semibold transition focus:outline-none focus:ring-0"
            >
              取消
            </button>
            <button
              type="submit"
              disabled={isSaving}
              className="bg-indigo-600 hover:bg-indigo-500 disabled:opacity-50 text-white px-5 py-2 rounded-lg text-xs font-bold flex items-center gap-1.5 shadow-lg shadow-indigo-500/20 transition focus:outline-none focus:ring-0"
            >
              <Save size={14} />
              {isSaving ? "保存中..." : "保存配置"}
            </button>
          </div>
        </form>
      </div>
    </div>
  );
}
