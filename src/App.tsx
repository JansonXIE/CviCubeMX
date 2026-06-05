import React from "react";
import { BrowserRouter, Routes, Route, NavLink } from "react-router-dom";
import { invoke } from "@tauri-apps/api/core";

// 页面组件
import PinoutPage from "./pages/PinoutPage";
import ClockPage from "./pages/ClockPage";
import MemoryPage from "./pages/MemoryPage";
import FlashPage from "./pages/FlashPage";

/**
 * 侧边栏 - 对应 C++ MainWindow 的 QTabWidget 配置标签页
 * 包含: Pinout, Clock, Memory, Flash 四个导航项
 */
function Sidebar() {
  const links = [
    { to: "/", label: "Pinout", icon: "📌" },
    { to: "/clock", label: "Clock", icon: "⏰" },
    { to: "/memory", label: "Memory", icon: "💾" },
    { to: "/flash", label: "Flash", icon: "🔥" },
  ];

  return (
    <nav className="w-48 bg-gray-800 text-white h-full flex flex-col">
      <div className="p-4 font-bold text-lg">CviCubeMX</div>
      {links.map((l) => (
        <NavLink
          key={l.to}
          to={l.to}
          end={l.to === "/"}
          className={({ isActive }) =>
            `px-4 py-2 hover:bg-gray-700 ${isActive ? "bg-gray-600" : ""}`
          }
        >
          {l.icon} {l.label}
        </NavLink>
      ))}
    </nav>
  );
}

/**
 * 顶部栏 - 对应 C++ MainWindow 的 headerLayout + controlLayout
 * 包含: 应用标题、IPC 测试按钮
 */
function TopBar() {
  const [greetResult, setGreetResult] = React.useState("");

  const handleGreet = async () => {
    const result = await invoke<string>("greet", { name: "CviCubeMX" });
    setGreetResult(result);
  };

  return (
    <header className="h-12 bg-gray-100 flex items-center justify-between px-4 border-b">
      <span className="font-semibold">CviCubeMX - 芯片配置工具</span>
      <div className="flex items-center gap-3">
        <button
          onClick={handleGreet}
          className="bg-blue-500 text-white px-3 py-1 rounded hover:bg-blue-600"
        >
          Test IPC
        </button>
        {greetResult && (
          <span className="text-green-600 text-sm">{greetResult}</span>
        )}
      </div>
    </header>
  );
}

/**
 * 内容区域 - 对应 C++ MainWindow 的 QStackedWidget
 * 根据 Sidebar 选中的路由显示不同的页面
 */
function ContentArea() {
  return (
    <Routes>
      <Route path="/" element={<PinoutPage />} />
      <Route path="/clock" element={<ClockPage />} />
      <Route path="/memory" element={<MemoryPage />} />
      <Route path="/flash" element={<FlashPage />} />
    </Routes>
  );
}

/**
 * 主应用组件 - 对应 C++ MainWindow 的整体布局
 * 布局: TopBar + (Sidebar + ContentArea)
 */
export default function App() {
  return (
    <BrowserRouter>
      <div className="h-screen flex flex-col">
        <TopBar />
        <div className="flex flex-1 overflow-hidden">
          <Sidebar />
          <main className="flex-1 overflow-auto p-4">
            <ContentArea />
          </main>
        </div>
      </div>
    </BrowserRouter>
  );
}