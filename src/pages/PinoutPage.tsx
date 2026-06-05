/**
 * Pinout 配置页面 - 对应 C++ MainWindow 的 m_pinoutTab
 * 包含: 芯片选型下拉框、引脚配置面板、芯片视图 (QFN/BGA 布局)
 */
export default function PinoutPage() {
  return (
    <div className="flex flex-col h-full">
      <h2 className="text-xl font-bold mb-4">Pinout 配置</h2>
      <p className="text-gray-500">引脚功能配置页面（待实现）</p>
      <div className="mt-4 flex gap-4">
        <div className="border rounded p-4 text-gray-400">
          芯片选型 ComboBox
        </div>
        <div className="border rounded p-4 text-gray-400">
          引脚配置面板 (PinConfigTree)
        </div>
        <div className="border rounded p-4 text-gray-400">
          芯片视图 (QFN/BGA 布局)
        </div>
      </div>
    </div>
  );
}