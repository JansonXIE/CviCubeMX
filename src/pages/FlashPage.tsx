/**
 * Flash 分区页面 - 对应 C++ MainWindow 的 m_flashTab
 * 包含: 分区管理、大小校验、defconfig 导出
 */
export default function FlashPage() {
  return (
    <div className="flex flex-col h-full">
      <h2 className="text-xl font-bold mb-4">Flash 分区</h2>
      <p className="text-gray-500">Flash 分区管理页面（待实现）</p>
      <div className="mt-4 border rounded p-4 text-gray-400">
        FlashConfigWidget (分区增删 + 大小校验)
      </div>
    </div>
  );
}