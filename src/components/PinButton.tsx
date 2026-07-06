import React, { useState, useEffect, useRef } from "react";
import { createPortal } from "react-dom";
import { PinInfo, MuxDef } from "../stores/chipStore";

interface PinButtonProps {
  pin: PinInfo;
  isHighlighted: boolean;
  onSelectFunction: (func: string) => Promise<void>;
  /** 当 pin.current_function 为二级 mux 时的定义 (否则 undefined) */
  muxOptions?: MuxDef;
  /** 选择二级功能的回调 */
  onSelectSubFunction?: (subFunc: string) => Promise<void>;
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

export default function PinButton({ pin, isHighlighted, onSelectFunction, muxOptions, onSelectSubFunction }: PinButtonProps) {
  const [menuOpen, setMenuOpen] = useState(false);
  const [menuPos, setMenuPos] = useState({ x: 0, y: 0 });
  const menuRef = useRef<HTMLDivElement>(null);
  const buttonRef = useRef<HTMLButtonElement>(null);

  // 动态调整菜单位置以防超出视口
  React.useLayoutEffect(() => {
    if (menuOpen && menuRef.current) {
      const rect = menuRef.current.getBoundingClientRect();
      const viewportHeight = window.innerHeight;
      const viewportWidth = window.innerWidth;

      let adjustedY = menuPos.y;
      let adjustedX = menuPos.x;

      // 如果菜单超出屏幕底部，向上调整位置
      if (menuPos.y + rect.height > viewportHeight) {
        adjustedY = Math.max(10, viewportHeight - rect.height - 10);
      }

      // 如果菜单超出屏幕右侧，向左调整位置
      if (menuPos.x + rect.width > viewportWidth) {
        adjustedX = Math.max(10, viewportWidth - rect.width - 10);
      }

      if (adjustedY !== menuPos.y || adjustedX !== menuPos.x) {
        setMenuPos({ x: adjustedX, y: adjustedY });
      }
    }
  }, [menuOpen, menuPos.x, menuPos.y]);

  // 左键点击事件处理器
  const handleClick = (e: React.MouseEvent) => {
    e.preventDefault();
    setMenuPos({ x: e.clientX, y: e.clientY });
    setMenuOpen((prev) => !prev);
  };

  // 右键菜单拦截
  const handleContextMenu = (e: React.MouseEvent) => {
    e.preventDefault();
    setMenuPos({ x: e.clientX, y: e.clientY });
    setMenuOpen(true);
  };

  // 点击外部关闭菜单
  useEffect(() => {
    const handleClickOutside = (event: MouseEvent) => {
      if (
        menuRef.current &&
        !menuRef.current.contains(event.target as Node) &&
        buttonRef.current &&
        !buttonRef.current.contains(event.target as Node)
      ) {
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

  // 选择二级 mux 功能后关闭菜单
  const selectSubFunc = async (subFunc: string) => {
    setMenuOpen(false);
    if (onSelectSubFunction) {
      await onSelectSubFunction(subFunc);
    }
  };

  const colorClass = getPinColor(pin.current_function);

  return (
    <div className="relative inline-block">
      {/* 引脚按钮 */}
      <button
        ref={buttonRef}
        type="button"
        onClick={handleClick}
        onContextMenu={handleContextMenu}
        className={`w-10 h-10 rounded-lg font-bold text-xs flex flex-col items-center justify-center transition-all duration-300 border border-slate-700/50 shadow-md outline-none focus:outline-none focus-visible:outline-none focus:ring-0 active:scale-95 ${colorClass} ${
          isHighlighted ? "animate-pulse ring-4 ring-indigo-500/80 shadow-[0_0_15px_rgba(99,102,241,0.6)] scale-110" : ""
        }`}
        style={{ outline: "none", WebkitTapHighlightColor: "transparent" }}
        title={`引脚: ${pin.pin_num}\n名称: ${pin.pin_name}\n当前功能: ${pin.current_function}${
          muxOptions && pin.current_state ? `\n二级功能: ${pin.current_state}` : ""
        }`}
      >
        <span className="opacity-80 scale-75">{pin.pin_num}</span>
        <span className="truncate max-w-full scale-75 leading-none">
          {(muxOptions && pin.current_state ? pin.current_state : pin.current_function).split("_")[0]}
        </span>
      </button>

      {/* 右键功能选择菜单 */}
      {menuOpen && createPortal(
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
                className={`w-full text-left px-3 py-2 text-xs rounded-lg transition-all duration-200 outline-none focus:outline-none focus-visible:outline-none focus:ring-0 ${
                  pin.current_function === fn
                    ? "bg-indigo-600/30 text-indigo-400 font-semibold"
                    : "text-slate-300 hover:bg-slate-800/80 hover:text-white"
                }`}
                style={{ outline: "none", WebkitTapHighlightColor: "transparent" }}
              >
                {fn}
              </button>
            ))}

            {/* 二级功能选择: 仅当当前一级功能是 SPI1 二级 mux 时显示 */}
            {muxOptions && (
              <div className="pt-1 mt-1 border-t border-slate-700/50">
                <div className="px-3 py-1.5 text-[10px] uppercase tracking-wider text-amber-400/80 font-bold flex items-center gap-1.5">
                  <svg xmlns="http://www.w3.org/2000/svg" width="11" height="11" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><path d="m6 9 6 6 6-6"/></svg>
                  二级功能 ({muxOptions.name})
                </div>
                {muxOptions.options.map((sub) => {
                  const active = (pin.current_state ?? muxOptions.default) === sub;
                  const isDefault = sub === muxOptions.default;
                  return (
                    <button
                      key={sub}
                      type="button"
                      onClick={() => selectSubFunc(sub)}
                      className={`w-full text-left px-3 py-2 text-xs rounded-lg transition-all duration-200 outline-none focus:outline-none focus-visible:outline-none focus:ring-0 flex items-center justify-between ${
                        active
                          ? "bg-amber-500/25 text-amber-300 font-semibold"
                          : "text-slate-300 hover:bg-slate-800/80 hover:text-white"
                      }`}
                      style={{ outline: "none", WebkitTapHighlightColor: "transparent" }}
                    >
                      <span>{sub}</span>
                      {isDefault && (
                        <span className="text-[9px] text-slate-500 uppercase tracking-wide">默认</span>
                      )}
                    </button>
                  );
                })}
              </div>
            )}

            {pin.current_function !== pin.default_function && (
              <div className="pt-1 mt-1 border-t border-slate-700/50">
                <button
                  type="button"
                  onClick={() => selectFunc(pin.default_function)}
                  className="w-full text-left px-3 py-2 text-xs rounded-lg transition-all duration-200 outline-none focus:outline-none focus-visible:outline-none focus:ring-0 text-amber-500/80 hover:bg-amber-500/20 hover:text-amber-400 flex items-center gap-1.5"
                  style={{ outline: "none", WebkitTapHighlightColor: "transparent" }}
                >
                  <svg xmlns="http://www.w3.org/2000/svg" width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><path d="M3 12a9 9 0 1 0 9-9 9.75 9.75 0 0 0-6.74 2.74L3 8"/><path d="M3 3v5h5"/></svg>
                  恢复默认 ({pin.default_function})
                </button>
              </div>
            )}
          </div>
        </div>,
        document.body
      )}
    </div>
  );
}
