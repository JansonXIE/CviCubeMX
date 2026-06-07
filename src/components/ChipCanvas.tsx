import React from "react";
import { useChipStore } from "../stores/chipStore";
import { getQfnLayout, getBgaPins } from "../utils/pinLayout";
import PinButton from "./PinButton";

export default function ChipCanvas() {
  const { chipSpec, pins, highlightedPins, setPinFunction, isLoading } = useChipStore();

  if (isLoading) {
    return (
      <div className="flex flex-col items-center justify-center h-96 text-slate-400">
        <div className="animate-spin rounded-full h-10 w-10 border-b-2 border-indigo-400 mb-4"></div>
        <span>正在加载芯片视图...</span>
      </div>
    );
  }

  if (!chipSpec) {
    return (
      <div className="flex flex-col items-center justify-center h-96 border border-dashed border-slate-700/50 rounded-2xl bg-slate-900/20 text-slate-400">
        <span>请先选择芯片型号以渲染引脚视图</span>
      </div>
    );
  }

  // 寻找引脚信息
  const getPinInfoByNum = (pinNum: string) => {
    return Array.from(pins.values()).find((p) => p.pin_num === pinNum);
  };

  const isBga = chipSpec.package.toUpperCase() === "BGA";

  // BGA 布局渲染
  const renderBgaLayout = () => {
    const rowsStr = chipSpec.rows || "ABCDEFGHJKLMNOPQR";
    const cols = chipSpec.cols || 8;
    const rows = rowsStr.split("");

    const gridStyle = {
      gridTemplateColumns: `repeat(${cols + 1}, minmax(0, 1fr))`,
    };

    // 生成四角剔除名称
    const firstRow = rows[0];
    const lastRow = rows[rows.length - 1];
    const cornerExclusions = [
      `${firstRow}1`,
      `${firstRow}${cols}`,
      `${lastRow}1`,
      `${lastRow}${cols}`,
    ];

    return (
      <div className="overflow-auto max-w-full p-4 flex justify-center">
        <div className="grid gap-1.5" style={gridStyle}>
          {/* 左上角空白 */}
          <div className="w-10 h-10 flex items-center justify-center font-bold text-slate-500 text-xs"></div>
          {/* 顶栏数字 */}
          {Array.from({ length: cols }, (_, i) => i + 1).map((c) => (
            <div key={`col-${c}`} className="w-10 h-10 flex items-center justify-center font-bold text-slate-500 text-xs">
              {c}
            </div>
          ))}

          {rows.map((r) => (
            <React.Fragment key={`row-frag-${r}`}>
              {/* 左栏字母 */}
              <div className="w-10 h-10 flex items-center justify-center font-bold text-slate-500 text-xs">
                {r}
              </div>
              {/* 各引脚网格 */}
              {Array.from({ length: cols }, (_, i) => i + 1).map((c) => {
                const pinNum = `${r}${c}`;
                if (cornerExclusions.includes(pinNum)) {
                  // 四角缺失
                  return (
                    <div
                      key={`excluded-${pinNum}`}
                      className="w-10 h-10 border border-transparent flex items-center justify-center text-slate-800 text-[10px]"
                    >
                      {/* 留空 */}
                    </div>
                  );
                }

                const pin = getPinInfoByNum(pinNum);
                if (!pin) {
                  return (
                    <div
                      key={`empty-${pinNum}`}
                      className="relative w-10 h-10 bg-slate-800/20 rounded-full border border-slate-700/50 flex items-center justify-center text-slate-500 text-[10px] font-bold select-none opacity-70"
                      title={`${pinNum} - 未映射引脚（禁用）`}
                    >
                      {pinNum}
                      <div className="absolute inset-0 flex items-center justify-center pointer-events-none">
                        <svg className="w-6 h-6 text-red-500/80" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
                          <line x1="18" y1="6" x2="6" y2="18"></line>
                          <line x1="6" y1="6" x2="18" y2="18"></line>
                        </svg>
                      </div>
                    </div>
                  );
                }

                return (
                  <PinButton
                    key={pin.pin_name}
                    pin={pin}
                    isHighlighted={highlightedPins.has(pin.pin_name)}
                    onSelectFunction={(fn) => setPinFunction(pin.pin_name, fn)}
                  />
                );
              })}
            </React.Fragment>
          ))}
        </div>
      </div>
    );
  };

  // QFN 布局渲染
  const renderQfnLayout = () => {
    const pinCount = chipSpec.pin_count;
    const side = pinCount / 4;
    const layout = getQfnLayout(pinCount);

    // Grid 是 (side + 2) x (side + 2)
    const gridSize = side + 2;
    const gridStyle = {
      gridTemplateColumns: `repeat(${gridSize}, minmax(0, 1fr))`,
      gridTemplateRows: `repeat(${gridSize}, minmax(0, 1fr))`,
    };

    // 在网格中放置引脚
    const gridItems: React.ReactNode[] = [];

    for (let r = 1; r <= gridSize; r++) {
      for (let c = 1; c <= gridSize; c++) {
        const key = `cell-${r}-${c}`;

        // 判断是否是四角
        const isCorner =
          (r === 1 && c === 1) ||
          (r === 1 && c === gridSize) ||
          (r === gridSize && c === 1) ||
          (r === gridSize && c === gridSize);

        if (isCorner) {
          // 四角空余
          gridItems.push(<div key={key} className="w-10 h-10" />);
          continue;
        }

        // 判断是否是顶边 (Top): row = 1, col 从 side + 1 递减到 2
        if (r === 1) {
          const idx = gridSize - 1 - c; // 从右到左
          const pinNum = layout.top[idx];
          gridItems.push(renderGridPin(pinNum, key));
          continue;
        }

        // 判断是否是底边 (Bottom): row = gridSize, col 从 2 递增到 side + 1
        if (r === gridSize) {
          const idx = c - 2; // 从左到右
          const pinNum = layout.bottom[idx];
          gridItems.push(renderGridPin(pinNum, key));
          continue;
        }

        // 判断是否是左边 (Left): col = 1, row 从 2 递增到 side + 1
        if (c === 1) {
          const idx = r - 2; // 从上到下
          const pinNum = layout.left[idx];
          gridItems.push(renderGridPin(pinNum, key));
          continue;
        }

        // 判断是否是右边 (Right): col = gridSize, row 从 side + 1 递减到 2
        if (c === gridSize) {
          const idx = gridSize - 1 - r; // 从下到上
          const pinNum = layout.right[idx];
          gridItems.push(renderGridPin(pinNum, key));
          continue;
        }

        // 中间主区域，合并为一个大容器显示芯片规格
        if (r === 2 && c === 2) {
          gridItems.push(
            <div
              key="center-container"
              style={{
                gridRow: `2 / span ${side}`,
                gridColumn: `2 / span ${side}`,
              }}
              className="flex flex-col items-center justify-center bg-slate-900/90 border border-slate-700/50 rounded-xl m-2 shadow-[inset_0_0_20px_rgba(0,0,0,0.6)]"
            >
              <span className="text-xl font-bold bg-gradient-to-r from-indigo-400 to-cyan-400 bg-clip-text text-transparent">
                {chipSpec.chip_type.toUpperCase()}
              </span>
              <span className="text-[10px] text-slate-500 mt-1 uppercase tracking-wider">
                {chipSpec.package} - {chipSpec.pin_count} 引脚
              </span>
              <p className="text-[9px] text-slate-400 max-w-[80%] text-center mt-2 opacity-60">
                {chipSpec.description}
              </p>
            </div>
          );
        }
      }
    }

    return (
      <div className="overflow-auto max-w-full p-4 flex justify-center">
        <div className="grid gap-1.5" style={gridStyle}>
          {gridItems}
        </div>
      </div>
    );
  };

  const renderGridPin = (pinNum: string, key: string) => {
    const pin = getPinInfoByNum(pinNum);
    if (!pin) {
      return (
        <div
          key={key}
          className="relative w-10 h-10 bg-slate-800/20 rounded-lg border border-slate-700/50 flex items-center justify-center text-slate-500 text-[10px] font-bold select-none opacity-70"
          title={`${pinNum} - 未映射引脚（禁用）`}
        >
          {pinNum}
          <div className="absolute inset-0 flex items-center justify-center pointer-events-none">
            <svg className="w-6 h-6 text-red-500/80" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <line x1="18" y1="6" x2="6" y2="18"></line>
              <line x1="6" y1="6" x2="18" y2="18"></line>
            </svg>
          </div>
        </div>
      );
    }
    return (
      <PinButton
        key={pin.pin_name}
        pin={pin}
        isHighlighted={highlightedPins.has(pin.pin_name)}
        onSelectFunction={(fn) => setPinFunction(pin.pin_name, fn)}
      />
    );
  };

  return (
    <div className="bg-slate-900/60 backdrop-blur-xl border border-slate-700/50 rounded-2xl p-6 shadow-2xl flex flex-col items-center justify-center relative overflow-hidden group">
      {/* 渐变微光 */}
      <div className="absolute inset-0 bg-gradient-to-br from-indigo-500/5 to-cyan-500/5 pointer-events-none" />
      
      <div className="mb-4 text-center">
        <h3 className="font-bold text-slate-300">芯片引脚布局拓扑</h3>
        <p className="text-xs text-slate-500 mt-1">点击或右键引脚以配置对应复用功能</p>
      </div>

      <div className="relative w-full max-w-4xl bg-slate-950/40 border border-slate-800/80 rounded-xl p-4 shadow-inner">
        {isBga ? renderBgaLayout() : renderQfnLayout()}
      </div>

      {/* 颜色说明指南 */}
      <div className="mt-6 flex flex-wrap justify-center gap-4 text-xs">
        <div className="flex items-center gap-1.5">
          <span className="w-3 h-3 rounded bg-slate-500"></span>
          <span className="text-slate-400">GPIO</span>
        </div>
        <div className="flex items-center gap-1.5">
          <span className="w-3 h-3 rounded bg-rose-500"></span>
          <span className="text-slate-400">ADC</span>
        </div>
        <div className="flex items-center gap-1.5">
          <span className="w-3 h-3 rounded bg-sky-500"></span>
          <span className="text-slate-400">I2C</span>
        </div>
        <div className="flex items-center gap-1.5">
          <span className="w-3 h-3 rounded bg-emerald-500"></span>
          <span className="text-slate-400">UART</span>
        </div>
        <div className="flex items-center gap-1.5">
          <span className="w-3 h-3 rounded bg-amber-500"></span>
          <span className="text-slate-400">SPI</span>
        </div>
        <div className="flex items-center gap-1.5">
          <span className="w-3 h-3 rounded bg-purple-500"></span>
          <span className="text-slate-400">PWM</span>
        </div>
        <div className="flex items-center gap-1.5">
          <span className="w-3 h-3 rounded bg-indigo-500"></span>
          <span className="text-slate-400">其它</span>
        </div>
      </div>
    </div>
  );
}
