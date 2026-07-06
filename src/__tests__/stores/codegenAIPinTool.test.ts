// ============================================================
// CviCubeMX 重构前功能验证测试 - M7 代码生成 + M8 AI + M9 引脚工具
// ============================================================

import { describe, it, expect, vi } from 'vitest';
import { invoke } from '@tauri-apps/api/core';
import { listen } from '@tauri-apps/api/event';
import { useChatStore } from '../../stores/chatStore';
import { isMarkdownContent, parseSSEChunk } from '../../utils/markdownDetect';

vi.mock('@tauri-apps/api/core', () => ({
  invoke: vi.fn(),
}));

vi.mock('@tauri-apps/api/event', () => ({
  listen: vi.fn(),
}));

// ---- M7 参考数据 (从 codegenerator.cpp/h 提取) ----

// PINMUX 宏格式: PINMUX(PAD_name, FUNCTION_name)
function generatePinmuxMacro(pinName: string, function_: string): string {
  return `PINMUX(${pinName}, ${function_})`;
}

// 功能宏名称映射 (样本)
const FUNCTION_MACROS: Record<string, string> = {
  'XGPIOC_18': 'XGPIOC_18',
  'VI0_D_15': 'VI0_D_15',
  'SD1_CLK': 'SD1_CLK',
  'UART0_TX': 'UART0_TX',
  'CAM_MCLK0': 'CAM_MCLK0',
  'PWM_12': 'PWM_12',
  'IIC1_SDA': 'IIC1_SDA',
  'USB_VBUS_DET': 'USB_VBUS_DET',
  'SDIO0_D_1': 'SDIO0_D_1',
  'RSTN': 'RSTN',
};

// ETH 相关功能关键词
const ETH_KEYWORDS = ['RMII0', 'EPHY', 'PAD_ETH'];
// MIPI 相关功能关键词
const MIPI_KEYWORDS = ['MIPI', 'VI0_D', 'VI1_D', 'VI2_D'];
// Audio 相关功能关键词
const AUDIO_KEYWORDS = ['IIS', 'IIC', 'AUD', 'SPK'];

// 增量更新: 查找 "// Generated PINMUX configurations" 块
function findGeneratedBlock(content: string): { start: number; end: number } | null {
  const startRegex = /\/\/ Generated PINMUX configurations/;
  const endRegex = /return\s+0\s*;/;
  const startMatch = startRegex.exec(content);
  const endMatch = endRegex.exec(content);
  if (startMatch && endMatch) {
    return { start: startMatch.index, end: endMatch.index };
  }
  return null;
}

// ---- M7 测试套件 ----

describe('M7 - 代码生成器 (特征化测试)', () => {

  describe('M7-T1: PINMUX 宏格式', () => {
    it('PINMUX(PAD_MIPI_TXM4, XGPIOC_18) 格式正确', () => {
      expect(generatePinmuxMacro('PAD_MIPI_TXM4', 'XGPIOC_18')).toBe('PINMUX(PAD_MIPI_TXM4, XGPIOC_18)');
    });

    it('PINMUX(UART0_TX, UART0_TX) 格式正确', () => {
      expect(generatePinmuxMacro('UART0_TX', 'UART0_TX')).toBe('PINMUX(UART0_TX, UART0_TX)');
    });
  });

  describe('M7-T2: 功能宏名称映射', () => {
    it('XGPIOC_18 → XGPIOC_18', () => {
      expect(FUNCTION_MACROS['XGPIOC_18']).toBe('XGPIOC_18');
    });

    it('USB_VBUS_DET → USB_VBUS_DET', () => {
      expect(FUNCTION_MACROS['USB_VBUS_DET']).toBe('USB_VBUS_DET');
    });

    it('RSTN → RSTN', () => {
      expect(FUNCTION_MACROS['RSTN']).toBe('RSTN');
    });
  });

  describe('M7-T3: 已有文件增量更新', () => {
    it('应找到 Generated PINMUX 块的位置', () => {
      const content = `
int board_init(void) {
    // Generated PINMUX configurations
    PINMUX(PAD_MIPI_TXM4, XGPIOC_18);
    return 0;
}`;
      const block = findGeneratedBlock(content);
      expect(block).not.toBeNull();
      expect(block!.start).toBeGreaterThan(0);
      expect(block!.end).toBeGreaterThan(block!.start);
    });

    it('无 Generated 块时应返回 null', () => {
      const content = `
int board_init(void) {
    // no generated block
    return 0;
}`;
      const block = findGeneratedBlock(content);
      // 没有 "// Generated PINMUX configurations" 标记
      expect(block).toBeNull();
    });
  });

  describe('M7-T4: ETH 序列生成', () => {
    it('RMII0_ 相关功能应识别为 ETH', () => {
      expect(ETH_KEYWORDS.some(k => 'RMII0_TXD1'.includes(k))).toBe(true);
    });

    it('EPHY_ 相关功能应识别为 ETH', () => {
      expect(ETH_KEYWORDS.some(k => 'EPHY_SPD_LED'.includes(k))).toBe(true);
    });

    it('PAD_ETH_ 相关功能应识别为 ETH', () => {
      expect(ETH_KEYWORDS.some(k => 'PAD_ETH_RXM'.includes(k))).toBe(true);
    });

    it('UART0_TX 不应识别为 ETH', () => {
      expect(ETH_KEYWORDS.some(k => 'UART0_TX'.includes(k))).toBe(false);
    });
  });

  describe('M7-T5: MIPI 序列生成', () => {
    it('MIPI_ 相关功能应识别为 MIPI', () => {
      expect(MIPI_KEYWORDS.some(k => 'PAD_MIPI_TXM4'.includes(k))).toBe(true);
    });

    it('VI0_D_ 相关功能应识别为 MIPI', () => {
      expect(MIPI_KEYWORDS.some(k => 'VI0_D_15'.includes(k))).toBe(true);
    });
  });

  describe('M7-T6: Audio 序列生成', () => {
    it('IIS_ 相关功能应识别为 Audio', () => {
      expect(AUDIO_KEYWORDS.some(k => 'IIS1_BCLK'.includes(k))).toBe(true);
    });

    it('IIC_ 相关功能应识别为 Audio', () => {
      expect(AUDIO_KEYWORDS.some(k => 'IIC1_SDA'.includes(k))).toBe(true);
    });

    it('AUD_ 相关功能应识别为 Audio', () => {
      expect(AUDIO_KEYWORDS.some(k => 'PAD_AUD_AOUTL'.includes(k))).toBe(true);
    });
  });
});

// ---- M8 参考数据 ----

const AI_API_CONFIG = {
  baseUrl: 'https://api.example.com/v1/chat/completions',
  model: 'gpt-4',
};

// 使用导入的 parseSSEChunk 和 isMarkdownContent

// ---- M8 测试套件 ----

describe('M8 - AI 对话助手 (特征化测试)', () => {

  describe('M8-T1: SSE 请求构造', () => {
    it('请求体应包含 model 和 messages 字段', () => {
      const requestBody = {
        model: AI_API_CONFIG.model,
        messages: [{ role: 'user', content: 'Hello' }],
        stream: true,
      };
      expect(requestBody.model).toBe('gpt-4');
      expect(requestBody.messages).toHaveLength(1);
      expect(requestBody.stream).toBe(true);
    });
  });

  describe('M8-T2: SSE chunk 解析', () => {
    it('应解析 data: {"choices":[...]} 格式', () => {
      const sseLine = 'data: {"choices":[{"delta":{"content":"Hello"}}]}';
      const result = parseSSEChunk(sseLine);
      expect(result).not.toBeNull();
      expect(result!.content).toBe('Hello');
    });

    it('应解析空 content delta', () => {
      const sseLine = 'data: {"choices":[{"delta":{"content":""}}]}';
      const result = parseSSEChunk(sseLine);
      expect(result).not.toBeNull();
      expect(result!.content).toBe('');
    });

    it('data: [DONE] 应返回 null', () => {
      const sseLine = 'data: [DONE]';
      const result = parseSSEChunk(sseLine);
      expect(result).toBeNull();
    });

    it('非 data: 开头的行应返回 null', () => {
      const result = parseSSEChunk('event: message');
      expect(result).toBeNull();
    });
  });

  describe('M8-T3: Markdown 检测', () => {
    it('# 标题应被识别为 Markdown', () => {
      expect(isMarkdownContent('# Title')).toBe(true);
    });

    it('代码块 ``` 应被识别为 Markdown', () => {
      expect(isMarkdownContent('```python')).toBe(true);
    });

    it('**bold** 应被识别为 Markdown', () => {
      expect(isMarkdownContent('**bold text**')).toBe(true);
    });

    it('- 列表项应被识别为 Markdown', () => {
      expect(isMarkdownContent('- item')).toBe(true);
    });

    it('纯文本不应被识别为 Markdown', () => {
      expect(isMarkdownContent('plain text')).toBe(false);
    });
  });

  describe('M8-T4: ChatStore - API 配置 (待前端实现)', () => {
    it('baseUrl/model 保存后可读取', async () => {
      const newConfig = {
        api_key: 'test_key',
        base_url: 'https://test.api.com',
        model: 'deepseek-chat',
      };
      
      const mockInvoke = vi.mocked(invoke);
      mockInvoke.mockResolvedValueOnce(undefined);
      
      await useChatStore.getState().saveConfig(newConfig);
      
      expect(useChatStore.getState().aiConfig).toEqual(newConfig);
      expect(mockInvoke).toHaveBeenCalledWith('save_ai_config', { config: newConfig });
    });
  });

  describe('M8-T5: ChatStore - 消息流式追加 (待前端实现)', () => {
    it('收到 AI chunk 后消息内容实时追加', async () => {
      const mockListen = vi.mocked(listen);
      let eventCallback: any = null;
      
      mockListen.mockImplementationOnce(async (event, callback) => {
        eventCallback = callback;
        return (() => {}) as any;
      });

      await useChatStore.getState().initListener();
      expect(eventCallback).not.toBeNull();

      useChatStore.setState({
        messages: [
          { id: '1', role: 'assistant', content: 'Thinking...', timestamp: Date.now(), isMarkdown: false }
        ],
        currentAIResponse: 'Thinking...'
      });

      eventCallback({
        payload: {
          content: 'Hello',
          done: false
        }
      });
      expect(useChatStore.getState().messages[0].content).toBe('Thinking...Hello');

      eventCallback({
        payload: {
          content: ' World',
          done: false
        }
      });
      expect(useChatStore.getState().messages[0].content).toBe('Thinking...Hello World');

      eventCallback({
        payload: {
          content: '',
          done: true
        }
      });
      expect(useChatStore.getState().isStreaming).toBe(false);
      expect(useChatStore.getState().messages[0].content).toBe('Thinking...Hello World');
    });
  });
});

// ---- M9 参考数据 ----

const FUNCTION_NAME_REMAP: Record<string, string> = {
  'CR_4WTMS': 'CV_2WTMS_CR_4WTMS',
  'CR_4WTCK': 'CV_2WTCK_CR_4WTCK',
  'CR_4WTDI': 'CV_SCL0__CR_4WTDI',
  'CR_4WTDO': 'CV_SDA0__CR_4WTDO',
  'CR_SCL0': 'CV_4WTDI_CR_SCL0',
  'CR_SDA0': 'CV_4WTMS_CR_SDA0',
  'CR_2WTMS': 'CV_4WTDO_CR_2WTMS',
  'CR_2WTCK': 'CV_4WTCK_CR_2WTCK',
};

// Description 解析 (模拟 generate_pins.py: parse_function_select_cell)
function parseFunctionSelectCell(cellContent: string): { functions: string[]; default: string } {
  const functions: string[] = [];
  let defaultFunction = 'GPIO';

  if (!cellContent) return { functions: ['GPIO'], default: 'GPIO' };

  const lines = cellContent.split('\n');
  let foundDefault = false;

  for (const line of lines) {
    const trimmed = line.trim();
    if (!trimmed || trimmed.includes('function select') || trimmed.includes('Others :')) continue;

    if (trimmed.includes(':')) {
      const parts = trimmed.split(':', 2);
      if (parts.length === 2) {
        let funcInfo = parts[1].trim();
        if (funcInfo.includes('(default)')) {
          let funcName = funcInfo.replace('(default)', '').trim().replace('[', '_').replace(']', '');
          funcName = FUNCTION_NAME_REMAP[funcName] ?? funcName;
          defaultFunction = funcName;
          functions.push(funcName);
          foundDefault = true;
        } else {
          let funcName = funcInfo.trim().replace('[', '_').replace(']', '');
          funcName = FUNCTION_NAME_REMAP[funcName] ?? funcName;
          if (funcName) functions.push(funcName);
        }
      }
    }
  }

  if (!foundDefault && functions.length > 0) {
    const gpioFallback = functions.filter(f => f.includes('XGPIO'));
    defaultFunction = gpioFallback.length > 0 ? gpioFallback[0] : functions[0];
  }

  if (functions.length === 0) {
    functions.push('GPIO');
    defaultFunction = 'GPIO';
  }

  if (!functions.includes(defaultFunction)) {
    functions.push(defaultFunction);
  }

  return { functions, default: defaultFunction };
}

// Pin Name 清理
function cleanPinName(pinName: string): string {
  if (pinName.includes('___')) {
    return pinName.split('___')[0];
  }
  return pinName;
}

// 引脚自然排序 (模拟 generate_pins.py: get_pin_sort_key)
function getPinSortKey(pinNum: string): [string, number] {
  const match = pinNum.match(/^([A-Z]+)(\d+)$/);
  if (match) {
    return [match[1], parseInt(match[2])];
  }
  try {
    return ['', parseInt(pinNum)];
  } catch {
    return [pinNum, 0];
  }
}

// ---- M9 测试套件 ----

describe('M9 - 引脚数据生成工具 (特征化测试)', () => {

  describe('M9-T1: Description 解析-默认功能', () => {
    it('"0 : UART0_TX (default)" → default=UART0_TX', () => {
      const result = parseFunctionSelectCell('0 : UART0_TX (default)\n3 : XGPIOA_16');
      expect(result.default).toBe('UART0_TX');
    });

    it('默认功能应在功能列表中', () => {
      const result = parseFunctionSelectCell('0 : UART0_TX (default)\n3 : XGPIOA_16');
      expect(result.functions).toContain('UART0_TX');
    });
  });

  describe('M9-T2: Description 解析-多功能', () => {
    it('多行解析为完整功能列表', () => {
      const content = `0 : VI0_D_15
1 : SD1_CLK
2 : VO_D_24
3 : XGPIOC_18 (default)
4 : CAM_MCLK1
5 : PWM_12
6 : IIC1_SDA
7 : DBG_18`;
      const result = parseFunctionSelectCell(content);
      expect(result.functions.length).toBe(8);
      expect(result.default).toBe('XGPIOC_18');
    });

    it('空内容应返回 GPIO', () => {
      const result = parseFunctionSelectCell('');
      expect(result.functions).toEqual(['GPIO']);
      expect(result.default).toBe('GPIO');
    });
  });

  describe('M9-T3: FUNCTION_NAME_REMAP', () => {
    it('CR_4WTMS → CV_2WTMS_CR_4WTMS', () => {
      expect(FUNCTION_NAME_REMAP['CR_4WTMS']).toBe('CV_2WTMS_CR_4WTMS');
    });

    it('重映射应在 Description 解析中自动应用', () => {
      const content = '0 : CR_4WTMS (default)';
      const result = parseFunctionSelectCell(content);
      expect(result.default).toBe('CV_2WTMS_CR_4WTMS');
      expect(result.functions).toContain('CV_2WTMS_CR_4WTMS');
    });
  });

  describe('M9-T4: Pin Name 清理', () => {
    it('PAD_ETH_RXM___EPHY_TXP → PAD_ETH_RXM', () => {
      expect(cleanPinName('PAD_ETH_RXM___EPHY_TXP')).toBe('PAD_ETH_RXM');
    });

    it('不含 ___ 的 Pin Name 保持原样', () => {
      expect(cleanPinName('PAD_MIPI_TXM4')).toBe('PAD_MIPI_TXM4');
    });
  });

  describe('M9-T5: JSON 输出格式', () => {
    it('输出应包含必要字段', () => {
      const pinEntry = {
        chip_type: 'cv1842hp',
        pin_num: 'A2',
        pin_name: 'PAD_MIPI_TXM4',
        functions: ['VI0_D_15', 'SD1_CLK', 'VO_D_24', 'XGPIOC_18', 'CAM_MCLK1', 'PWM_12', 'IIC1_SDA', 'DBG_18'],
        default: 'XGPIOC_18',
      };
      expect(pinEntry).toHaveProperty('chip_type');
      expect(pinEntry).toHaveProperty('pin_num');
      expect(pinEntry).toHaveProperty('pin_name');
      expect(pinEntry).toHaveProperty('functions');
      expect(pinEntry).toHaveProperty('default');
    });
  });

  describe('M9-T6: 引脚排序', () => {
    it('A2 < A10 (自然排序)', () => {
      const keyA2 = getPinSortKey('A2');
      const keyA10 = getPinSortKey('A10');
      expect(keyA2[1]).toBeLessThan(keyA10[1]); // 2 < 10
    });

    it('B1 < B10', () => {
      const keyB1 = getPinSortKey('B1');
      const keyB10 = getPinSortKey('B10');
      expect(keyB1[1]).toBeLessThan(keyB10[1]);
    });

    it('A < B (字母排序)', () => {
      const keyA2 = getPinSortKey('A2');
      const keyB1 = getPinSortKey('B1');
      expect(keyA2[0] < keyB1[0]).toBe(true); // 'A' < 'B'
    });

    it('纯数字引脚排序', () => {
      const key1 = getPinSortKey('1');
      const key10 = getPinSortKey('10');
      expect(key1[1]).toBeLessThan(key10[1]); // 1 < 10
    });
  });
});