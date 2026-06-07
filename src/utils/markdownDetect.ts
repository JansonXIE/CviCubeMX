/**
 * Markdown 检测函数
 *
 * 参考 C++ aichatdialog.cpp isMarkdownContent() 实现
 * 检测内容是否包含常见的 Markdown 标记：
 * - 标题: # ## ### 等
 * - 粗体: **text**
 * - 代码块: ```code```
 * - 无序列表: - * +
 * - 有序列表: 1. 2.
 * - 引用: >
 * - 链接: [text](url)
 * - 分割线: ---
 * - 表格分隔符: |
 */
export function isMarkdownContent(content: string): boolean {
  return /^#{1,6}\s+/.test(content) ||         // 标题 # ## ### 等
    /\*\*[^*]+\*\*/.test(content) ||           // 粗体 **text**
    /```/.test(content) ||                     // 代码块 ```code```
    /^\s*[-*+]\s+/.test(content) ||            // 无序列表 - * + (行首)
    /^\s*\d+\.\s+/.test(content) ||            // 有序列表 1. 2. (行首)
    /^>\s+/.test(content) ||                   // 引用 >
    /\[.*\]\(.*\)/.test(content) ||            // 链接 [text](url)
    content.includes('---') ||                 // 分割线
    content.includes('|');                     // 表格分隔符
}

/**
 * SSE chunk 解析函数
 *
 * 参考 C++ aichatdialog.cpp onNetworkReplyReadyRead() 实现
 * 解析 SSE 格式: data: {"choices":[{"delta":{"content":"..."}}]}
 * 结束标记: data: [DONE]
 * 非 data: 开头的行跳过
 */
export function parseSSEChunk(line: string): { content: string } | null {
  if (!line.startsWith('data: ')) return null;
  const data = line.slice(6).trim();
  if (data === '[DONE]') return null;
  if (data === '') return null;
  try {
    const parsed = JSON.parse(data);
    const content = parsed.choices?.[0]?.delta?.content ?? '';
    return { content };
  } catch {
    return null;
  }
}