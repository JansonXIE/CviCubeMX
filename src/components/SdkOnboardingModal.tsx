import React, { useState, useEffect } from "react";
import { invoke } from "@tauri-apps/api/core";
import { getCurrentWindow } from "@tauri-apps/api/window";
import { FolderOpen, Cpu, CheckCircle2, AlertTriangle, ArrowRight, X } from "lucide-react";
import { open } from "@tauri-apps/plugin-dialog";

interface SdkOnboardingModalProps {
  isOpen: boolean;
  onClose: () => void;
  onComplete: (sdkPath: string, chipType: string) => void;
  isClosable?: boolean;
}

const CHIP_LIST = [
  "cv1840cp_wevb_0015a_spinor",
  "cv1841cp_wevb_0015a_emmc",
  "cv1841cp_wevb_0015a_spinand",
  "cv1841cp_wevb_0015a_spinor",
  "cv1842cp_wevb_0015a_spinand",
  "cv1842cp_wevb_0015a_spinor",
  "cv1842hp_wevb_0014a_emmc",
  "cv1842hp_wevb_0014a_spinand",
  "cv1842hp_wevb_0014a_spinor",
  "cv1843hp_wevb_0014a_emmc",
  "cv1843hp_wevb_0014a_spinand",
  "cv1843hp_wevb_0014a_spinor",
];

export default function SdkOnboardingModal({
  isOpen,
  onClose,
  onComplete,
  isClosable = false,
}: SdkOnboardingModalProps) {
  const [step, setStep] = useState(1);
  const [sdkPath, setSdkPath] = useState("");
  const [lastSdkPath, setLastSdkPath] = useState<string | null>(null);
  const [selectedChip, setSelectedChip] = useState("cv1842hp_wevb_0014a_emmc");
  
  const [isValidating, setIsValidating] = useState(false);
  const [validationError, setValidationError] = useState<string | null>(null);
  const [validationSuccess, setValidationSuccess] = useState(false);
  const [shake, setShake] = useState(false);

  const handleSelectDirectory = async () => {
    try {
      const selected = await open({
        directory: true,
        multiple: false,
        title: "选择 CviCubeMX 源码根目录 (SDK根目录)",
      });
      if (selected) {
        const path = Array.isArray(selected) ? selected[0] : selected;
        const normalizedPath = path.replace(/\\/g, "/");
        setSdkPath(normalizedPath);
        handleValidatePath(normalizedPath);
      }
    } catch (e) {
      setValidationError(`打开选择文件夹对话框失败: ${e}`);
      triggerShake();
    }
  };

  useEffect(() => {
    if (isOpen) {
      const last = localStorage.getItem("lastSdkPath");
      if (last) {
        setLastSdkPath(last);
      }
      const chip = localStorage.getItem("selectedChipType") || "cv1842hp_wevb_0014a_emmc";
      setSelectedChip(chip);
      setStep(1);
      setValidationError(null);
      setValidationSuccess(false);
    }
  }, [isOpen]);

  if (!isOpen) return null;

  const handleValidatePath = async (pathToCheck: string) => {
    if (!pathToCheck.trim()) {
      setValidationError("路径不能为空");
      triggerShake();
      return;
    }
    setIsValidating(true);
    setValidationError(null);
    setValidationSuccess(false);

    try {
      // 兼容 Windows 反斜杠
      const normalizedPath = pathToCheck.replace(/\\/g, "/");
      const isValid = await invoke<boolean>("validate_sdk_path", { path: normalizedPath });
      if (isValid) {
        setValidationSuccess(true);
        setSdkPath(normalizedPath);
        setTimeout(() => {
          setStep(2);
        }, 800);
      } else {
        setValidationError("路径验证失败：所选目录缺少必要的 build 结构或 boards/cv184x 关键目录");
        triggerShake();
      }
    } catch (err) {
      setValidationError(`验证出错: ${err}`);
      triggerShake();
    } finally {
      setIsValidating(false);
    }
  };

  const triggerShake = () => {
    setShake(true);
    setTimeout(() => setShake(false), 500);
  };

  const handleUseLastPath = () => {
    if (lastSdkPath) {
      handleValidatePath(lastSdkPath);
    }
  };

  const handleCancel = async () => {
    if (isClosable) {
      onClose();
    } else {
      // 在未初始化的启动状态，取消选择代表直接关闭应用
      try {
        await getCurrentWindow().close();
      } catch (e) {
        console.error("关闭窗口失败", e);
      }
    }
  };

  const handleConfirmChip = () => {
    localStorage.setItem("lastSdkPath", sdkPath);
    localStorage.setItem("selectedChipType", selectedChip);
    onComplete(sdkPath, selectedChip);
    onClose();
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-slate-950/40 backdrop-blur-md transition-all duration-300">
      <div 
        className={`w-full max-w-md overflow-hidden bg-white dark:bg-slate-900 border border-slate-200 dark:border-slate-800 rounded-2xl shadow-2xl transition-all duration-305 ${
          shake ? "animate-bounce" : ""
        }`}
      >
        {/* Modal 头部 */}
        <div className="flex items-center justify-between px-6 py-4 border-b border-slate-100 dark:border-slate-800/60 bg-slate-50/50 dark:bg-slate-900/50">
          <h2 className="text-base font-bold text-slate-800 dark:text-slate-100 flex items-center gap-2">
            {step === 1 ? (
              <>
                <FolderOpen className="w-5 h-5 text-blue-500" />
                <span>CviCubeMX - 源代码路径选择</span>
              </>
            ) : (
              <>
                <Cpu className="w-5 h-5 text-indigo-500" />
                <span>CviCubeMX - 芯片选型</span>
              </>
            )}
          </h2>
          {isClosable && (
            <button 
              onClick={onClose}
              className="p-1 rounded-lg text-slate-400 hover:text-slate-600 dark:hover:text-slate-200 hover:bg-slate-100 dark:hover:bg-slate-800 transition-colors focus:outline-none"
            >
              <X className="w-4 h-4" />
            </button>
          )}
        </div>

        {/* Modal 内容 */}
        <div className="p-6 space-y-6">
          {/* 进度指示 */}
          <div className="flex items-center justify-center gap-4 text-xs font-semibold">
            <div className={`flex items-center gap-1.5 pb-1 border-b-2 ${step === 1 ? "border-blue-500 text-blue-600 dark:text-blue-400" : "border-transparent text-slate-400"}`}>
              <span className="flex items-center justify-center w-5 h-5 rounded-full bg-blue-100 dark:bg-blue-900/30 text-[10px]">1</span>
              <span>配置源码路径</span>
            </div>
            <ArrowRight className="w-4 h-4 text-slate-300" />
            <div className={`flex items-center gap-1.5 pb-1 border-b-2 ${step === 2 ? "border-indigo-500 text-indigo-600 dark:text-indigo-400" : "border-transparent text-slate-400"}`}>
              <span className="flex items-center justify-center w-5 h-5 rounded-full bg-indigo-100 dark:bg-indigo-900/30 text-[10px]">2</span>
              <span>选择芯片型号</span>
            </div>
          </div>

          {step === 1 ? (
            /* 步骤 1 */
            <div className="space-y-4">
              <div className="space-y-2">
                <p className="text-sm font-medium text-slate-700 dark:text-slate-300">
                  欢迎使用 CviCubeMX！
                </p>
                <p className="text-xs text-slate-500 dark:text-slate-400 leading-relaxed">
                  请选择或输入包含设备树 dts 文件、defconfig 和 cvi_board_init.c 的源码根目录绝对路径。<br />
                  该根目录应当包含以下子结构：<br />
                  <code className="inline-block px-1.5 py-0.5 mt-1 font-mono text-blue-600 dark:text-blue-400 bg-blue-50 dark:bg-blue-950/30 rounded">- build/</code>
                </p>
              </div>

              {/* 输入与选择框 */}
              <div className="space-y-3">
                <div className="flex gap-2">
                  <input
                    type="text"
                    value={sdkPath}
                    onChange={(e) => {
                      setSdkPath(e.target.value);
                      setValidationError(null);
                      setValidationSuccess(false);
                    }}
                    onKeyDown={(e) => {
                      if (e.key === "Enter") {
                        handleValidatePath(sdkPath);
                      }
                    }}
                    placeholder="请选择或输入 SDK 源码目录绝对路径并回车..."
                    className="flex-1 px-3 py-2 text-xs bg-white dark:bg-slate-950 border border-slate-300 dark:border-slate-800 rounded-lg shadow-sm focus:outline-none focus:border-blue-500 dark:focus:border-blue-500 transition-colors"
                  />
                  <button
                    onClick={handleSelectDirectory}
                    className="px-3.5 py-2 text-xs font-semibold text-slate-700 dark:text-slate-200 bg-slate-100 dark:bg-slate-800 hover:bg-slate-200 dark:hover:bg-slate-750 active:scale-95 border border-slate-200 dark:border-slate-700 rounded-lg shadow-sm flex items-center gap-1.5 transition-all focus:outline-none focus:ring-0"
                  >
                    <FolderOpen className="w-4 h-4 text-blue-500" />
                    <span>选择目录</span>
                  </button>
                </div>

                {/* 上次路径填充卡片 */}
                {lastSdkPath && (
                  <div className="p-3 bg-slate-50 dark:bg-slate-950/40 border border-slate-100 dark:border-slate-850 rounded-xl flex items-center justify-between">
                    <div className="min-w-0 flex-1 pr-3">
                      <span className="block text-[10px] text-slate-400 font-bold uppercase tracking-wider">上次使用的路径</span>
                      <span className="block text-xs font-mono text-slate-600 dark:text-slate-300 truncate">{lastSdkPath}</span>
                    </div>
                    <button
                      onClick={handleUseLastPath}
                      className="px-2.5 py-1 text-xs font-bold text-blue-600 hover:text-white dark:text-blue-400 bg-white dark:bg-slate-900 border border-blue-200 dark:border-blue-900/60 hover:bg-blue-500 dark:hover:bg-blue-500 rounded-lg transition-colors focus:outline-none"
                    >
                      直接使用
                    </button>
                  </div>
                )}
              </div>

              {/* 验证反馈状态 */}
              {validationError && (
                <div className="p-3 bg-rose-50 dark:bg-rose-950/20 border border-rose-100 dark:border-rose-900/30 rounded-xl text-rose-600 dark:text-rose-400 text-xs flex gap-2 items-start">
                  <AlertTriangle className="w-4 h-4 mt-0.5 flex-shrink-0" />
                  <p>{validationError}</p>
                </div>
              )}

              {validationSuccess && (
                <div className="p-3 bg-emerald-50 dark:bg-emerald-950/20 border border-emerald-100 dark:border-emerald-900/30 rounded-xl text-emerald-600 dark:text-emerald-400 text-xs flex gap-2 items-start">
                  <CheckCircle2 className="w-4 h-4 mt-0.5 flex-shrink-0" />
                  <p>路径验证成功！即将跳转至下一步。</p>
                </div>
              )}
            </div>
          ) : (
            /* 步骤 2 */
            <div className="space-y-4">
              <div className="space-y-1 text-center">
                <p className="text-sm font-semibold text-slate-700 dark:text-slate-300">
                  请选择芯片型号
                </p>
                <p className="text-xs text-slate-500 dark:text-slate-400">
                  请从下方列表中选择您要配置的芯片型号
                </p>
              </div>

              {/* 下拉框 */}
              <div>
                <select
                  id="chip-select"
                  value={selectedChip}
                  onChange={(e) => setSelectedChip(e.target.value)}
                  className="w-full px-3 py-2 text-sm text-slate-900 dark:text-slate-100 bg-white dark:bg-slate-950 border border-slate-300 dark:border-slate-800 rounded-lg shadow-sm focus:outline-none focus:border-indigo-500 dark:focus:border-indigo-500 transition-colors"
                >
                  {CHIP_LIST.map((chip) => (
                    <option key={chip} value={chip} className="text-slate-900 dark:text-slate-100 bg-white dark:bg-slate-950">
                      {chip}
                    </option>
                  ))}
                </select>
              </div>
            </div>
          )}
        </div>

        {/* Modal 尾部 */}
        <div className="px-6 py-4 bg-slate-50 dark:bg-slate-900/50 border-t border-slate-100 dark:border-slate-800/60 flex justify-end gap-3">
          <button
            onClick={handleCancel}
            className="px-5 py-2 text-xs font-semibold text-slate-600 dark:text-slate-300 bg-white dark:bg-slate-900 border border-slate-200 dark:border-slate-800 rounded-lg hover:bg-slate-50 dark:hover:bg-slate-850 active:bg-slate-100 transition-colors focus:outline-none"
          >
            {isClosable ? "取消" : "取消并退出"}
          </button>
          
          {step === 2 && (
            <button
              onClick={handleConfirmChip}
              className="px-5 py-2 text-xs font-semibold text-white bg-indigo-500 hover:bg-indigo-600 active:bg-indigo-700 rounded-lg shadow-md transition-colors focus:outline-none"
            >
              确定
            </button>
          )}
        </div>
      </div>
    </div>
  );
}
