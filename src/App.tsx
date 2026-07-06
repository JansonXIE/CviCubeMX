import React from "react";
import { BrowserRouter, Routes, Route, NavLink } from "react-router-dom";
import { invoke } from "@tauri-apps/api/core";
import { MapPin, Clock, Cpu, Zap, Beaker, Settings, MessageSquare } from "lucide-react";

// 页面组件
import PinoutPage from "./pages/PinoutPage";
import PeripheralPage from "./pages/PeripheralPage";
import ClockPage from "./pages/ClockPage";
import MemoryPage from "./pages/MemoryPage";
import FlashPage from "./pages/FlashPage";
import AIChatPage from "./pages/AIChatPage";

// 引导组件与状态管理
import SdkOnboardingModal from "./components/SdkOnboardingModal";
import { useSdkStore } from "./stores/sdkStore";
import { useChipStore } from "./stores/chipStore";
import { usePeripheralStore } from "./stores/peripheralStore";

/**
 * 侧边栏 - 现代深色毛玻璃风格
 */
function Sidebar() {
  const links = [
    { to: "/", label: "Pinout", icon: <MapPin size={20} /> },
    { to: "/peripherals", label: "Peripherals", icon: <Settings size={20} /> },
    { to: "/clock", label: "Clock", icon: <Clock size={20} /> },
    { to: "/memory", label: "Memory", icon: <Cpu size={20} /> },
    { to: "/flash", label: "Flash", icon: <Zap size={20} /> },
    { to: "/aichat", label: "AI Chat", icon: <MessageSquare size={20} /> },
  ];

  return (
    <nav className="w-64 bg-slate-900/60 backdrop-blur-xl border-r border-slate-700/50 flex flex-col transition-all duration-300">
      <div className="p-6 flex items-center gap-3">
        <div className="bg-indigo-500/20 p-2 rounded-xl text-indigo-400">
          <Beaker size={24} />
        </div>
        <span className="font-bold text-xl tracking-wide bg-gradient-to-r from-indigo-400 to-cyan-400 bg-clip-text text-transparent">
          CviCubeMX
        </span>
      </div>
      
      <div className="flex-1 px-4 py-2 space-y-2">
        {links.map((l) => (
          <NavLink
            key={l.to}
            to={l.to}
            end={l.to === "/"}
            className={({ isActive }) =>
              `flex items-center gap-3 px-4 py-3 rounded-xl transition-all duration-300 group relative overflow-hidden ${
                isActive
                  ? "bg-indigo-500/10 text-indigo-400 shadow-[inset_4px_0_0_0_rgba(99,102,241,1)]"
                  : "text-slate-400 hover:bg-slate-800/50 hover:text-slate-200"
              }`
            }
          >
            <span className="relative z-10">{l.icon}</span>
            <span className="font-medium relative z-10">{l.label}</span>
            {/* 悬停微光动效 */}
            <div className="absolute inset-0 bg-gradient-to-r from-transparent via-white/[0.05] to-transparent -translate-x-full group-hover:animate-shimmer pointer-events-none" />
          </NavLink>
        ))}
      </div>
    </nav>
  );
}

/**
 * 顶部栏 - 现代卡片风格，包含交互按钮
 */
function TopBar() {
  const { sdkPath, chipType, setIsOnboardingOpen } = useSdkStore();

  return (
    <header className="h-16 bg-slate-900/40 backdrop-blur-lg flex items-center justify-between px-6 border-b border-slate-700/50 z-10">
      <div className="flex items-center gap-6">
        <span className="text-slate-300 font-medium text-sm">芯片配置工具</span>
        {sdkPath && (
          <div className="hidden md:flex items-center gap-3 text-xs">
            <div className="bg-slate-800/80 px-3 py-1.5 rounded-lg border border-slate-700/40 flex items-center gap-2 max-w-xs">
              <span className="text-slate-500 font-bold uppercase tracking-wider text-[10px]">SDK</span>
              <span className="text-slate-300 font-mono truncate max-w-[150px]" title={sdkPath}>{sdkPath}</span>
            </div>
            <div className="bg-slate-800/80 px-3 py-1.5 rounded-lg border border-slate-700/40 flex items-center gap-2">
              <span className="text-slate-500 font-bold uppercase tracking-wider text-[10px]">CHIP</span>
              <span className="text-slate-300 font-mono truncate max-w-[180px]" title={chipType}>{chipType}</span>
            </div>
            <button
              onClick={() => setIsOnboardingOpen(true)}
              className="px-2.5 py-1.5 rounded-lg bg-indigo-500/10 text-indigo-400 hover:bg-indigo-500/20 active:bg-indigo-500/30 transition-all font-semibold outline-none focus:outline-none focus:ring-0 border-transparent cursor-pointer"
            >
              重新配置
            </button>
          </div>
        )}
      </div>
    </header>
  );
}

/**
 * 内容区域 - 对应 C++ MainWindow 的 QStackedWidget
 */
function ContentArea() {
  return (
    <div className="bg-slate-800/30 rounded-2xl border border-slate-700/50 p-6 h-full shadow-2xl backdrop-blur-sm overflow-hidden flex flex-col">
      <Routes>
        <Route path="/" element={<PinoutPage />} />
        <Route path="/peripherals" element={<PeripheralPage />} />
        <Route path="/clock" element={<ClockPage />} />
        <Route path="/memory" element={<MemoryPage />} />
        <Route path="/flash" element={<FlashPage />} />
        <Route path="/aichat" element={<AIChatPage />} />
      </Routes>
    </div>
  );
}

/**
 * 主应用组件
 */
export default function App() {
  const { sdkPath, chipType, setSdkPath, setChipType, isOnboardingOpen, setIsOnboardingOpen } = useSdkStore();
  const { selectChip, syncBoardInit } = useChipStore();
  const { loadPeripherals } = usePeripheralStore();

  // 校验并加载数据
  const initializeApp = async (path: string, chip: string) => {
    try {
      await selectChip(chip);
      // 恢复该板卡 cvi_board_init.c 中已保存的引脚复用配置
      await syncBoardInit(path);
      const dtsPath = `${path}/build/boards/default/dts/cv184x/cv184x_base.dtsi`;
      await loadPeripherals(dtsPath);
    } catch (e) {
      console.error("初始化应用数据失败:", e);
    }
  };

  React.useEffect(() => {
    const checkOnboarding = async () => {
      if (!sdkPath) {
        setIsOnboardingOpen(true);
      } else {
        try {
          const isValid = await invoke<boolean>("validate_sdk_path", { path: sdkPath });
          if (!isValid) {
            setIsOnboardingOpen(true);
          } else {
            // 路径合法，进行自动加载
            await initializeApp(sdkPath, chipType);
          }
        } catch (e) {
          console.error("验证路径出错", e);
          setIsOnboardingOpen(true);
        }
      }
    };
    checkOnboarding();
  }, [sdkPath, chipType]);

  const handleOnboardingComplete = async (path: string, chip: string) => {
    setSdkPath(path);
    setChipType(chip);
    await initializeApp(path, chip);
    setIsOnboardingOpen(false);
  };

  return (
    <BrowserRouter>
      <div className="h-screen flex flex-col bg-slate-900 text-slate-50 overflow-hidden font-sans">
        <TopBar />
        <div className="flex flex-1 overflow-hidden relative">
          {/* 背景装饰光晕 */}
          <div className="absolute top-[-10%] left-[-10%] w-[40%] h-[40%] rounded-full bg-indigo-500/10 blur-[120px] pointer-events-none" />
          <div className="absolute bottom-[-10%] right-[-10%] w-[40%] h-[40%] rounded-full bg-cyan-500/10 blur-[120px] pointer-events-none" />
          
          <Sidebar />
          <main className="flex-1 p-6 relative z-10 overflow-hidden flex flex-col">
            <ContentArea />
          </main>
        </div>

        {/* 启动引导 Modal */}
        <SdkOnboardingModal
          isOpen={isOnboardingOpen}
          onClose={() => setIsOnboardingOpen(false)}
          onComplete={handleOnboardingComplete}
          isClosable={!!sdkPath}
        />
      </div>
    </BrowserRouter>
  );
}