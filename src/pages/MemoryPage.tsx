/**
 * 内存配置页面 - 对应 C++ MainWindow 的 m_memoryTab
 * 包含: 内存区域管理、地址/大小格式化、重叠检测
 */
export default function MemoryPage() {
  return (
    <div className="flex flex-col h-full">
      <h2 className="text-xl font-bold mb-4">内存配置</h2>
      <p className="text-gray-500">内存区域配置与管理页面（待实现）</p>
      <div className="mt-4 border rounded p-4 text-gray-400">
        MemoryConfigWidget (区域管理 + 重叠检测)
      </div>
    </div>
  );
}