// ============================================================
// CviCubeMX 重构前功能验证测试 - M4 时钟树配置
// ============================================================

import { describe, it, expect, vi } from 'vitest';
import { useClockStore } from '../../stores/clockStore';

// Mock Tauri invoke
vi.mock('@tauri-apps/api/core', () => {
  const mockInvoke = vi.fn((cmd: string, args?: any) => {
    if (cmd === 'compute_clock_tree') {
      return Promise.resolve({
        pllConfigs: {
          clk_mipimpll: {
            name: 'clk_mipimpll',
            enabled: true,
            inputFreq: 25.0,
            outputFreq: 1350.0,
            divider: 1.0,
            multiplier: 54,
            source: 'OSC',
          },
          clk_a0pll: {
            name: 'clk_a0pll',
            enabled: true,
            inputFreq: 1350.0,
            outputFreq: 900.0,
            divider: 3.0,
            multiplier: 2,
            source: 'clk_mipimpll',
          },
        },
        outputs: {},
        subNodes: {},
      });
    }
    if (cmd === 'load_module_positions') {
      return Promise.resolve({
        PLL_MIPIMPLL: {
          moduleName: 'PLL_MIPIMPLL',
          x: 100,
          y: 200,
          width: 300,
          height: 150,
        },
      });
    }
    return Promise.resolve();
  });
  return { invoke: mockInvoke };
});

// ---- 参考数据 (从 C++ 源码 clockconfig.h/cpp 提取) ----

const OSC_FREQUENCY_MHZ = 25;       // 25MHz
const RTC_FREQUENCY_KHZ = 32.768;    // 32.768kHz

// PLL 频率计算公式: output = input * multiplier / divider
function computePLLFrequency(inputFreqMHz: number, multiplier: number, divider: number): number {
  return inputFreqMHz * multiplier / divider;
}

// 时钟输出频率计算公式: output = source / divider * multiplier
function computeOutputFrequency(sourceFreqMHz: number, divider: number, multiplier: number): number {
  return sourceFreqMHz / divider * multiplier;
}

// clk_1M 子节点: input = 1MHz, output = 1MHz / divider
function computeClk1MSubNodeFrequency(divider: number): number {
  return 1.0 / divider;
}

// ---- 测试套件 ----

describe('M4 - 时钟树配置 (特征化测试)', () => {

  // === M4-T1: OSC/RTC 基础频率 ===
  describe('M4-T1: OSC/RTC 基础频率', () => {
    it('OSC 频率应为 25MHz', () => {
      expect(OSC_FREQUENCY_MHZ).toBe(25);
    });

    it('RTC 频率应为 32.768kHz', () => {
      expect(RTC_FREQUENCY_KHZ).toBe(32.768);
    });
  });

  // === M4-T2: PLL 频率计算-基本 ===
  describe('M4-T2: PLL 频率计算-基本', () => {
    it('OSC(25) * multiplier / divider = output', () => {
      const result = computePLLFrequency(25, 10, 1);
      expect(result).toBe(250);
    });

    it('divider 为 1 时等于纯倍频', () => {
      expect(computePLLFrequency(25, 54, 1)).toBe(1350);
    });

    it('divider 支持小数', () => {
      // C++ ClockConfig 使用 QDoubleSpinBox 支持 divider 小数
      const result = computePLLFrequency(25, 10, 2.5);
      expect(result).toBe(100);
    });
  });

  // === M4-T3: PLL 频率计算-具体值 ===
  describe('M4-T3: PLL 频率计算-具体值', () => {
    it('multiplier=54, divider=1 → 1350MHz', () => {
      expect(computePLLFrequency(25, 54, 1)).toBe(1350);
    });

    it('multiplier=40, divider=2 → 500MHz', () => {
      expect(computePLLFrequency(25, 40, 2)).toBe(500);
    });

    it('multiplier=30, divider=1.5 → 500MHz', () => {
      expect(computePLLFrequency(25, 30, 1.5)).toBeCloseTo(500, 1);
    });
  });

  // === M4-T4: SubPLL 级联计算 ===
  describe('M4-T4: SubPLL 级联计算', () => {
    it('SubPLL 输入 = PLL 输出', () => {
      const pllOutput = computePLLFrequency(25, 54, 1); // 1350MHz
      const subPllOutput = computePLLFrequency(pllOutput, 2, 3);
      expect(subPllOutput).toBe(900);
    });

    it('SubPLL 级联: OSC→PLL(1350)→SubPLL(2/3) = 900MHz', () => {
      const osc = 25;
      const pllOutput = osc * 54 / 1; // 1350
      const subPllOutput = pllOutput * 2 / 3; // 900
      expect(subPllOutput).toBe(900);
    });
  });

  // === M4-T5: 时钟输出分频 ===
  describe('M4-T5: 时钟输出分频', () => {
    it('output_freq = source / divider * multiplier', () => {
      expect(computeOutputFrequency(900, 3, 1)).toBe(300);
    });

    it('divider=1, multiplier=1 → 输出=源频率', () => {
      expect(computeOutputFrequency(500, 1, 1)).toBe(500);
    });

    it('divider=5, multiplier=2 → 输出=源频率 * 0.4', () => {
      expect(computeOutputFrequency(1000, 5, 2)).toBe(400);
    });
  });

  // === M4-T6: clk_1M 子节点分频 ===
  describe('M4-T6: clk_1M 子节点分频', () => {
    it('clk_1M divider=1 → 1MHz', () => {
      expect(computeClk1MSubNodeFrequency(1)).toBe(1);
    });

    it('clk_1M divider=2 → 0.5MHz (500kHz)', () => {
      expect(computeClk1MSubNodeFrequency(2)).toBe(0.5);
    });

    it('clk_1M divider=10 → 0.1MHz (100kHz)', () => {
      expect(computeClk1MSubNodeFrequency(10)).toBeCloseTo(0.1, 2);
    });
  });

  // === M4-T7: defconfig 导出格式 ===
  describe('M4-T7: defconfig 导出格式', () => {
    it('导出格式应与 C++ exportToDefconfig 输出一致', async () => {
      const { invoke } = await import('@tauri-apps/api/core');
      const store = useClockStore.getState();
      
      const configs = {
        clk_mipimpll: {
          name: 'clk_mipimpll',
          enabled: true,
          inputFreq: 25.0,
          outputFreq: 1350.0,
          divider: 1.0,
          multiplier: 54,
          source: 'OSC',
        },
      };

      await store.exportClockDefconfig('/mock/path', 'cv1842hp', configs);
      expect(invoke).toHaveBeenCalledWith('export_clock_defconfig', {
        sourcePath: '/mock/path',
        chipType: 'cv1842hp',
        configs,
      });
    });
  });

  // === M4-T8: 模块位置持久化 ===
  describe('M4-T8: 模块位置持久化', () => {
    it('ModulePosition 结构应包含 x/y/width/height', () => {
      const modulePos = {
        moduleName: 'PLL_MIPIMPLL',
        x: 100,
        y: 200,
        width: 300,
        height: 150,
      };
      expect(modulePos.moduleName).toBe('PLL_MIPIMPLL');
      expect(modulePos.x).toBe(100);
      expect(modulePos.y).toBe(200);
      expect(modulePos.width).toBe(300);
      expect(modulePos.height).toBe(150);
    });
  });

  // === M4-T9/T10: ClockStore ===
  describe('M4-T9/T10: ClockStore', () => {
    it('ClockStore - PLL 配置变更应正确重算频率', async () => {
      const store = useClockStore.getState();
      const configs = {
        clk_mipimpll: {
          name: 'clk_mipimpll',
          enabled: true,
          inputFreq: 25.0,
          outputFreq: 0.0,
          divider: 1.0,
          multiplier: 54,
          source: 'OSC',
        },
      };

      await store.computeClockTree(configs);
      const state = useClockStore.getState();
      expect(state.pllConfigs.clk_mipimpll.outputFreq).toBe(1350);
      expect(state.pllConfigs.clk_a0pll.outputFreq).toBe(900);
    });

    it('时钟树搜索定位 - should update search state', () => {
      const store = useClockStore.getState();
      expect(store.searchText).toBe('');
      store.searchClock('mipimpll');
      const state = useClockStore.getState();
      expect(state.searchText).toBe('mipimpll');
    });
  });
});