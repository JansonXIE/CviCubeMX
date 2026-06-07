import React from "react";
import { BrowserRouter, Routes, Route, NavLink } from "react-router-dom";
import { invoke } from "@tauri-apps/api/core";
import { MapPin, Clock, Cpu, Zap, Beaker, Settings, Code, MessageSquare } from "lucide-react";

// 页面组件
import PinoutPage from "./pages/PinoutPage";
import PeripheralPage from "./pages/PeripheralPage";
import ClockPage from "./pages/ClockPage";
import MemoryPage from "./pages/MemoryPage";
import FlashPage from "./pages/FlashPage";
import CodeGenPage from "./pages/CodeGenPage";
import AIChatPage from "./pages/AIChatPage";

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
    { to: "/codegen", label: "CodeGen", icon: <Code size={20} /> },
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
  const [greetResult, setGreetResult] = React.useState("");

  const handleGreet = async () => {
    try {
      const result = await invoke<string>("greet", { name: "CviCubeMX" });
      setGreetResult(result);
    } catch (e) {
      console.error(e);
    }
  };

  return (
    <header className="h-16 bg-slate-900/40 backdrop-blur-lg flex items-center justify-between px-6 border-b border-slate-700/50 z-10">
      <div className="text-slate-300 font-medium">芯片配置工具</div>
      <div className="flex items-center gap-4">
        {greetResult && (
          <span className="text-emerald-400 text-sm font-medium animate-fade-in">
            {greetResult}
          </span>
        )}
        <button
          onClick={handleGreet}
          className="bg-indigo-500 hover:bg-indigo-600 text-white px-5 py-2 rounded-lg shadow-lg shadow-indigo-500/20 hover:shadow-indigo-500/40 transition-all duration-300 transform hover:-translate-y-0.5 active:translate-y-0 active:scale-95 outline-none focus:outline-none focus:ring-2 focus:ring-indigo-500/50 border-transparent font-medium"
        >
          测试 IPC
        </button>
      </div>
    </header>
  );
}

/**
 * 内容区域 - 对应 C++ MainWindow 的 QStackedWidget
 */
function ContentArea() {
  return (
    <div className="bg-slate-800/30 rounded-2xl border border-slate-700/50 p-6 h-full shadow-2xl backdrop-blur-sm overflow-auto">
      <Routes>
        <Route path="/" element={<PinoutPage />} />
        <Route path="/peripherals" element={<PeripheralPage />} />
        <Route path="/clock" element={<ClockPage />} />
        <Route path="/memory" element={<MemoryPage />} />
        <Route path="/flash" element={<FlashPage />} />
        <Route path="/codegen" element={<CodeGenPage />} />
        <Route path="/aichat" element={<AIChatPage />} />
      </Routes>
    </div>
  );
}

/**
 * 主应用组件
 */
export default function App() {
  return (
    <BrowserRouter>
      <div className="h-screen flex flex-col bg-slate-900 text-slate-50 overflow-hidden font-sans">
        <TopBar />
        <div className="flex flex-1 overflow-hidden relative">
          {/* 背景装饰光晕 */}
          <div className="absolute top-[-10%] left-[-10%] w-[40%] h-[40%] rounded-full bg-indigo-500/10 blur-[120px] pointer-events-none" />
          <div className="absolute bottom-[-10%] right-[-10%] w-[40%] h-[40%] rounded-full bg-cyan-500/10 blur-[120px] pointer-events-none" />
          
          <Sidebar />
          <main className="flex-1 p-6 relative z-10">
            <ContentArea />
          </main>
        </div>
      </div>
    </BrowserRouter>
  );
}