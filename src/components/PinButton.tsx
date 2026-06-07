import React, { useState, useEffect, useRef } from "react";
import { PinInfo } from "../stores/chipStore";

interface PinButtonProps {
  pin: PinInfo;
  isHighlighted: boolean;
  onSelectFunction: (func: string) => Promise<void>;
}

function getPinColor(func: string): string {
  const f = func.toUpperCase();
  if (f.startsWith("GPIO") || f.startsWith("XGPIO")) return "bg-slate-500 text-slate-100 hover:bg-slate-400";
  if (f.includes("ADC")) return "bg-rose-500 text-white hover:bg-rose-400";
  if (f.includes("I2C") || f.includes("IIC")) return "bg-sky-500 text-white hover:bg-sky-400";
  if (f.includes("UART")) return "bg-emerald-500 text-white hover:bg-emerald-400";
  if (f.includes("SPI")) return "bg-amber-500 text-white hover:bg-amber-400";
  if (f.includes("PWM")) return "bg-purple-500 text-white hover:bg-purple-400";
  return "bg-indigo-500 text-white hover:bg-indigo-400";
}

export default function PinButton({ pin, isHighlighted, onSelectFunction }: PinButtonProps) {
  const [menuOpen, setMenuOpen] = useState(false);
  const [menuPos, setMenuPos] = useState({ x: 0, y: 0 });
  const menuRef = useRef<HTMLDivElement>(null);

  // 右键菜单拦截
  const handleContextMenu = (e: React.MouseEvent) => {
    e.preventDefault();
    setMenuPos({ x: e.clientX, y: e.clientY });
    setMenuOpen(true);
  };

  // 点击外部关闭菜单
  useEffect(() => {
    const handleClickOutside = (event: MouseEvent) => {
      if (menuRef.current && !menuRef.current.contains(event.target as Node)) {
        setMenuOpen(false);
      }
    };
    if (menuOpen) {
      document.addEventListener("mousedown", handleClickOutside);
    }
    return () => {
      document.removeEventListener("mousedown", handleClickOutside);
    };
  }, [menuOpen]);

  const selectFunc = async (func: string) => {
    setMenuOpen(false);
    await onSelectFunction(func);
  };

  const colorClass = getPinColor(pin.current_function);

  return (
    <div className="relative inline-block">
      {/* 引脚按钮 */}
      <button
        type="button"
        onContextMenu={handleContextMenu}
        className={`w-10 h-10 rounded-lg font-bold text-xs flex flex-col items-center justify-center transition-all duration-300 border border-slate-700/50 shadow-md ${colorClass} ${
          isHighlighted ? "animate-pulse ring-4 ring-indigo-500/80 shadow-[0_0_15px_rgba(99,102,241,0.6)] scale-110" : ""
        } focus:outline-none focus:ring-0 active:scale-95`}
        title={`引脚: ${pin.pin_num}\n名称: ${pin.pin_name}\n当前功能: ${pin.current_function}`}
      >
        <span className="opacity-80 scale-75">{pin.pin_num}</span>
        <span className="truncate max-w-full scale-75 leading-none">{pin.current_function.split("_")[0]}</span>
      </button>

      {/* 右键功能选择菜单 */}
      {menuOpen && (
        <div
          ref={menuRef}
          className="fixed z-50 w-56 bg-slate-900/90 backdrop-blur-xl border border-slate-700/50 rounded-xl shadow-2xl p-2 animate-fade-in"
          style={{ top: menuPos.y, left: menuPos.x }}
        >
          <div className="px-3 py-1.5 border-b border-slate-800 text-[10px] uppercase tracking-wider text-slate-400 font-bold">
            选择引脚功能 ({pin.pin_num})
          </div>
          <div className="mt-1 max-h-60 overflow-y-auto space-y-0.5 custom-scrollbar">
            {pin.supported_functions.map((fn) => (
              <button
                key={fn}
                type="button"
                onClick={() => selectFunc(fn)}
                className={`w-full text-left px-3 py-2 text-xs rounded-lg transition-all duration-200 focus:outline-none focus:ring-0 ${
                  pin.current_function === fn
                    ? "bg-indigo-600/30 text-indigo-400 font-semibold"
                    : "text-slate-300 hover:bg-slate-800/80 hover:text-white"
                }`}
              >
                {fn}
              </button>
            ))}
          </div>
        </div>
      )}
    </div>
  );
}
