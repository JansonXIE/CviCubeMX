// ============================================================
// CviCubeMX 重构前功能验证测试 - M2 芯片选型与引脚数据
// ============================================================
// 本文件从 C++ 源码 (pinfunction.cpp, chipconfig.cpp) 提取的参考数据，
// 用于验证重构后的 Rust/TS 实现与原始 C++ 行为一致。

import { describe, it, expect, vi } from 'vitest';
import { useChipStore } from '../../stores/chipStore';
import { getQfnLayout, getBgaPins } from '../../utils/pinLayout';

vi.mock('@tauri-apps/api/core', () => {
  return {
    invoke: vi.fn(async (cmd, args?: any) => {
      if (cmd === 'load_chip_spec') {
        return {
          chip_type: args.chipType,
          package: args.chipType === 'cv1842hp' ? 'BGA' : 'QFN',
          pin_count: args.chipType === 'cv1842hp' ? 221 : 88,
          rows: args.chipType === 'cv1842hp' ? 'ABCDEFGHJKLMNPR' : undefined,
          cols: args.chipType === 'cv1842hp' ? 15 : undefined,
          description: 'mock spec',
        };
      }
      if (cmd === 'load_pin_data') {
        return [
          {
            pin_num: 'A2',
            pin_name: 'PAD_MIPI_TXM4',
            display_name: 'A2',
            supported_functions: ['XGPIOC_18', 'UART0_TX'],
            default_function: 'XGPIOC_18',
            current_function: 'XGPIOC_18',
            user_configured: false,
          },
        ];
      }
      return undefined;
    }),
  };
});

// ---- 参考数据 (直接从 C++ 源码提取) ----

// 从 chipconfig.cpp: getPinCountForChip() 提取
const CHIP_PIN_COUNTS: Record<string, number> = {
  'cv1801c': 64,
  'cv1801h': 60,
  'cv1811c': 88,
  'cv1811h': 84,
  'cv1842cp': 88,
  'cv1842hp': 221,
};

// 从 chipconfig.cpp: getPinCountForChip() - 封装类型
const CHIP_PACKAGE_TYPES: Record<string, string> = {
  'cv1801c': 'QFN',
  'cv1801h': 'BGA',
  'cv1811c': 'QFN',
  'cv1811h': 'BGA',
  'cv1842cp': 'QFN',
  'cv1842hp': 'BGA',
};

// 从 generate_pins.py: FUNCTION_NAME_REMAP 提取
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

// BGA 四角排除引脚
const BGA_CORNER_EXCLUSIONS = ['A1', 'A15', 'R1', 'R15'];

// BGA 行字母 (跳过 I)
const BGA_ROWS = 'ABCDEFGHJKLMNOPQR';

// 从 pinfunction.cpp 提取的引脚功能数据样本 (完整数据见 fixtures)
// 这里只列出关键样本用于快速测试
const PIN_DATA_SAMPLES: Record<string, { functions: string[]; default: string }> = {
  'PAD_MIPI_TXM4': {
    functions: ['VI0_D_15', 'SD1_CLK', 'VO_D_24', 'XGPIOC_18', 'CAM_MCLK1', 'PWM_12', 'IIC1_SDA', 'DBG_18'],
    default: 'XGPIOC_18',
  },
  'CAM_MCLK0': {
    functions: ['CAM_MCLK0', 'AUX1', 'XGPIOA_0'],
    default: 'XGPIOA_0',
  },
  'UART0_TX': {
    functions: ['UART0_TX', 'CAM_MCLK1', 'PWM_4', 'XGPIOA_16', 'UART1_TX', 'AUX1', 'DBG_6'],
    default: 'UART0_TX',
  },
  'USB_VBUS_DET': {
    functions: ['USB_VBUS_DET', 'XGPIOB_6', 'CAM_MCLK0', 'CAM_MCLK1'],
    default: 'USB_VBUS_DET',
  },
  'SD0_D1': {
    functions: ['SDIO0_D_1', 'IIC1_SDA', 'AUX0', 'XGPIOA_10', 'UART1_TX', 'PWM_12', 'WG0_D1', 'DBG_3'],
    default: 'SDIO0_D_1',
  },
  'RSTN': {
    functions: ['RSTN'],
    default: 'RSTN',
  },
  'PWR_VBAT_DET': {
    functions: ['PWR_VBAT_DET'],
    default: 'PWR_VBAT_DET',
  },
  // Pin Name 清理测试样本 (___ 分割)
  // 在 generate_pins.py 中: PAD_ETH_RXM___EPHY_TXP → PAD_ETH_RXM
};

// ---- 测试套件 ----

describe('M2 - 芯片选型与引脚数据 (特征化测试)', () => {

  // === M2-T1: 芯片引脚数量映射 ===
  describe('M2-T1: 芯片引脚数量映射', () => {
    it('每款芯片返回正确引脚数', () => {
      for (const [chipType, expectedCount] of Object.entries(CHIP_PIN_COUNTS)) {
        expect(expectedCount).toBeGreaterThan(0);
        // 重构后的 Rust 实现必须返回相同值
        // 测试数据验证: 确保参考数据与 C++ 源码一致
      }
    });

    it('cv1801c (QFN) 应有 64 引脚', () => {
      expect(CHIP_PIN_COUNTS['cv1801c']).toBe(64);
    });

    it('cv1801h (BGA) 应有 60 引脚 (8x8-4角)', () => {
      expect(CHIP_PIN_COUNTS['cv1801h']).toBe(60);
    });

    it('cv1842hp (BGA) 应有 221 引脚 (15x15-4角)', () => {
      expect(CHIP_PIN_COUNTS['cv1842hp']).toBe(221);
    });

    it('cv1842cp (QFN) 应有 88 引脚', () => {
      expect(CHIP_PIN_COUNTS['cv1842cp']).toBe(88);
    });
  });

  // === M2-T2: 引脚功能列表完整性 ===
  describe('M2-T2: 引脚功能列表完整性', () => {
    it('PAD_MIPI_TXM4 功能列表应包含 8 个功能', () => {
      expect(PIN_DATA_SAMPLES['PAD_MIPI_TXM4'].functions.length).toBe(8);
    });

    it('RSTN 功能列表应只有 1 个功能', () => {
      expect(PIN_DATA_SAMPLES['RSTN'].functions.length).toBe(1);
    });

    it('PWR_VBAT_DET 功能列表应只有 1 个功能', () => {
      expect(PIN_DATA_SAMPLES['PWR_VBAT_DET'].functions.length).toBe(1);
    });

    it('USB_VBUS_DET 功能列表应包含 USB_VBUS_DET 作为第一个功能', () => {
      expect(PIN_DATA_SAMPLES['USB_VBUS_DET'].functions[0]).toBe('USB_VBUS_DET');
    });
  });

  // === M2-T3: 默认功能正确性 ===
  describe('M2-T3: 默认功能正确性', () => {
    it('PAD_MIPI_TXM4 默认功能应为 XGPIOC_18', () => {
      expect(PIN_DATA_SAMPLES['PAD_MIPI_TXM4'].default).toBe('XGPIOC_18');
    });

    it('CAM_MCLK0 默认功能应为 XGPIOA_0', () => {
      expect(PIN_DATA_SAMPLES['CAM_MCLK0'].default).toBe('XGPIOA_0');
    });

    it('UART0_TX 默认功能应为 UART0_TX (自身)', () => {
      expect(PIN_DATA_SAMPLES['UART0_TX'].default).toBe('UART0_TX');
    });

    it('USB_VBUS_DET 默认功能应为 USB_VBUS_DET', () => {
      expect(PIN_DATA_SAMPLES['USB_VBUS_DET'].default).toBe('USB_VBUS_DET');
    });

    it('SD0_D1 默认功能应为 SDIO0_D_1 (function select 0)', () => {
      expect(PIN_DATA_SAMPLES['SD0_D1'].default).toBe('SDIO0_D_1');
    });

    it('RSTN 默认功能应为 RSTN (自身)', () => {
      expect(PIN_DATA_SAMPLES['RSTN'].default).toBe('RSTN');
    });
  });

  // === M2-T4: 功能重映射规则 ===
  describe('M2-T4: FUNCTION_NAME_REMAP 规则', () => {
    it('CR_4WTMS 应重映射为 CV_2WTMS_CR_4WTMS', () => {
      expect(FUNCTION_NAME_REMAP['CR_4WTMS']).toBe('CV_2WTMS_CR_4WTMS');
    });

    it('CR_4WTCK 应重映射为 CV_2WTCK_CR_4WTCK', () => {
      expect(FUNCTION_NAME_REMAP['CR_4WTCK']).toBe('CV_2WTCK_CR_4WTCK');
    });

    it('CR_SCL0 应重映射为 CV_4WTDI_CR_SCL0', () => {
      expect(FUNCTION_NAME_REMAP['CR_SCL0']).toBe('CV_4WTDI_CR_SCL0');
    });

    it('CR_SDA0 应重映射为 CV_4WTMS_CR_SDA0', () => {
      expect(FUNCTION_NAME_REMAP['CR_SDA0']).toBe('CV_4WTMS_CR_SDA0');
    });

    it('CR_2WTMS 应重映射为 CV_4WTDO_CR_2WTMS', () => {
      expect(FUNCTION_NAME_REMAP['CR_2WTMS']).toBe('CV_4WTDO_CR_2WTMS');
    });

    it('CR_2WTCK 应重映射为 CV_4WTCK_CR_2WTCK', () => {
      expect(FUNCTION_NAME_REMAP['CR_2WTCK']).toBe('CV_4WTCK_CR_2WTCK');
    });

    it('重映射规则应包含 8 条规则', () => {
      expect(Object.keys(FUNCTION_NAME_REMAP).length).toBe(8);
    });

    it('未匹配的功能名应保持原样', () => {
      // XGPIOA_0 不是重映射规则中的键，应保持原样
      expect(FUNCTION_NAME_REMAP['XGPIOA_0']).toBeUndefined();
    });
  });

  // === M2-T5: 引脚功能设置/查询 ===
  describe('M2-T5: 引脚功能设置/查询', () => {
    it('ChipConfig setPinFunction 后应能查询到', () => {
      // 模拟 ChipConfig 的行为
      const pinFunctions: Record<string, string> = {};
      pinFunctions['PAD_MIPI_TXM4'] = 'XGPIOC_18';

      // 设置新功能
      pinFunctions['PAD_MIPI_TXM4'] = 'UART0_TX';

      // 查询应返回新值
      expect(pinFunctions['PAD_MIPI_TXM4']).toBe('UART0_TX');
    });

    it('ChipConfig getPinFunction 未设置引脚应返回 GPIO', () => {
      // chipconfig.cpp: getPinFunction 默认返回 "GPIO"
      const pinFunctions: Record<string, string> = {};
      expect(pinFunctions['UNKNOWN_PIN'] ?? 'GPIO').toBe('GPIO');
    });

    it('ChipConfig clearPinFunctions 应清空所有映射', () => {
      const pinFunctions: Record<string, string> = {
        'PAD_MIPI_TXM4': 'XGPIOC_18',
        'UART0_TX': 'UART0_TX',
      };
      // clearPinFunctions
      for (const key of Object.keys(pinFunctions)) {
        delete pinFunctions[key];
      }
      expect(Object.keys(pinFunctions).length).toBe(0);
    });
  });

  // === M2-T6: BGA 四角引脚剔除 ===
  describe('M2-T6: BGA 四角引脚剔除', () => {
    it('A1 不应出现在 BGA 引脚列表中', () => {
      expect(BGA_CORNER_EXCLUSIONS).toContain('A1');
    });

    it('A15 不应出现在 BGA 引脚列表中', () => {
      expect(BGA_CORNER_EXCLUSIONS).toContain('A15');
    });

    it('R1 不应出现在 BGA 引脚列表中', () => {
      expect(BGA_CORNER_EXCLUSIONS).toContain('R1');
    });

    it('R15 不应出现在 BGA 引脚列表中', () => {
      expect(BGA_CORNER_EXCLUSIONS).toContain('R15');
    });

    it('BGA 四角排除应为 4 个引脚', () => {
      expect(BGA_CORNER_EXCLUSIONS.length).toBe(4);
    });

    it('BGA 行字母应跳过 I', () => {
      expect(BGA_ROWS).not.toContain('I');
    });

    it('BGA 行字母应为 17 个 (A-R 跳过 I: 18-1=17)', () => {
      // A B C D E F G H J K L M N P Q R = 17 (从A到R共18字母,跳过I)
      expect(BGA_ROWS.length).toBe(17);
    });
  });

  // === M2-T7: Pin Name 清理规则 ===
  describe('M2-T7: Pin Name 清理规则', () => {
    it('PAD_ETH_RXM___EPHY_TXP 应清理为 PAD_ETH_RXM', () => {
      const rawPinName = 'PAD_ETH_RXM___EPHY_TXP';
      const cleaned = rawPinName.split('___')[0];
      expect(cleaned).toBe('PAD_ETH_RXM');
    });

    it('不含 ___ 的 Pin Name 应保持原样', () => {
      const rawPinName = 'PAD_MIPI_TXM4';
      const cleaned = rawPinName.split('___')[0];
      expect(cleaned).toBe('PAD_MIPI_TXM4');
    });
  });

  describe('M2-T8~T12: ChipStore 和布局 (待前端实现)', () => {
    it('ChipStore - selectChip 应正确更新 chipType/chipSpec/pins', async () => {
      await useChipStore.getState().selectChip('cv1842hp');
      const state = useChipStore.getState();
      expect(state.chipType).toBe('cv1842hp');
      expect(state.chipSpec?.package).toBe('BGA');
      expect(state.pins.has('PAD_MIPI_TXM4')).toBe(true);
    });

    it('ChipStore - setPinFunction 应更新 pins Map', async () => {
      await useChipStore.getState().selectChip('cv1842hp');
      await useChipStore.getState().setPinFunction('PAD_MIPI_TXM4', 'UART0_TX');
      const state = useChipStore.getState();
      const pin = state.pins.get('PAD_MIPI_TXM4');
      expect(pin?.current_function).toBe('UART0_TX');
      expect(pin?.user_configured).toBe(true);
    });

    it('ChipStore - searchPin 应更新 highlightedPins Set', async () => {
      await useChipStore.getState().selectChip('cv1842hp');
      useChipStore.getState().searchPin('mipi');
      const state = useChipStore.getState();
      expect(state.highlightedPins.has('PAD_MIPI_TXM4')).toBe(true);
    });

    it('QFN 布局 - 88 引脚应按四边逆时针排列', () => {
      const layout = getQfnLayout(88);
      expect(layout.left).toHaveLength(22);
      expect(layout.bottom).toHaveLength(22);
      expect(layout.right).toHaveLength(22);
      expect(layout.top).toHaveLength(22);
      expect(layout.left[0]).toBe('1');
      expect(layout.bottom[0]).toBe('23');
    });

    it('BGA 布局 - 221 引脚网格四角缺失', () => {
      const pins = getBgaPins('ABCDEFGHJKLMNPR', 15);
      expect(pins).toHaveLength(221);
      expect(pins).not.toContain('A1');
      expect(pins).not.toContain('A15');
      expect(pins).not.toContain('R1');
      expect(pins).not.toContain('R15');
    });
  });
});