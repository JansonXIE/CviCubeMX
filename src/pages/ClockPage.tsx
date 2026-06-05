/**
 * 时钟树配置页面 - 对应 C++ MainWindow 的 m_clockTab
 * 包含: 时钟树可视化、PLL 配置、时钟输出配置
 */
export default function ClockPage() {
  return (
    <div className="flex flex-col h-full">
      <h2 className="text-xl font-bold mb-4">时钟树配置</h2>
      <p className="text-gray-500">时钟树可视化与 PLL 配置页面（待实现）</p>
      <div className="mt-4 border rounded p-4 text-gray-400">
        ClockConfigWidget (ClockConfig 可视化)
      </div>
    </div>
  );
}