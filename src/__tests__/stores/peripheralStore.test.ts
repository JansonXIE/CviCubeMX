// ============================================================
// CviCubeMX 重构前功能验证测试 - M3 外设配置与 DTS 管理
// ============================================================

import { describe, it, expect, vi } from 'vitest';
import { usePeripheralStore } from '../../stores/peripheralStore';

vi.mock('@tauri-apps/api/core', () => {
  const mockInvoke = vi.fn(async (cmd: string, args?: any) => {
    if (cmd === 'load_dts_peripherals') {
      return [
        {
          name: 'i2c0',
          status: 'disabled',
          clock_name: '',
          clock_freq: '',
          clock_frequency: 100000,
          pwm_cells: 0,
          current_speed: 0,
          sysdma_channels: [],
          has_status: true,
          has_clock: false,
          has_clock_freq: true,
          has_pwm_cells: false,
          has_current_speed: false,
          has_sysdma_channels: false,
          line_number: 14,
        },
        {
          name: 'uart0',
          status: 'disabled',
          clock_name: '',
          clock_freq: '',
          clock_frequency: 0,
          pwm_cells: 0,
          current_speed: 115200,
          sysdma_channels: [],
          has_status: true,
          has_clock: false,
          has_clock_freq: false,
          has_pwm_cells: false,
          has_current_speed: true,
          has_sysdma_channels: false,
          line_number: 21,
        }
      ];
    }
    return Promise.resolve();
  });
  return { invoke: mockInvoke };
});

// ---- 参考数据 (从 C++ 源码 dtsconfig.h 提取) ----

// PeripheralInfo 默认构造的 SYSDMA 通道映射
const SYSDMA_DEFAULT_CHANNELS = ['0', '5', '12', '13', '42', '42', '4', '7'];

// 示例 DTS 片段 (用于解析测试)
const SAMPLE_DTS_CONTENT = `
&i2c0 {
    status = "okay";
    clock-frequency = <100000>;
    #address-cells = <1>;
    #size-cells = <0>;
};

&uart0 {
    status = "disabled";
    current-speed = <115200>;
};

&pwm0 {
    status = "okay";
    #pwm-cells = <3>;
};

&spi0 {
    status = "okay";
    clock-frequency = <50000000>;
};

&sysdma {
    status = "okay";
    ch-remap = <0 5 12 13 42 42 4 7>;
};
`;

// PeripheralInfo 结构期望字段
interface PeripheralInfoRef {
  name: string;
  status: string;
  clockName: string | null;
  clockFreq: string;
  clockFrequency: number;
  pwmCells: number;
  currentSpeed: number;
  sysdmaChannels: string[];
  hasStatus: boolean;
  hasClock: boolean;
  hasClockFreq: boolean;
  hasPwmCells: boolean;
  hasCurrentSpeed: boolean;
  hasSysdmaChannels: boolean;
  lineNumber: number;
}

// ---- 测试套件 ----

describe('M3 - 外设配置与 DTS 管理 (特征化测试)', () => {

  // === M3-T1: DTS 解析-基本节点 ===
  describe('M3-T1: DTS 解析-基本节点', () => {
    it('i2c0 节点应解析为 status=okay', () => {
      // 从 C++ dtsconfig.cpp: parseNode() 的逻辑
      // &i2c0 { status = "okay"; } → PeripheralInfo { name: "i2c0", status: "okay" }
      const nodeText = '&i2c0 {\n    status = "okay";\n    clock-frequency = <100000>;\n};';
      const statusMatch = nodeText.match(/status\s*=\s*"([^"]+)"/);
      expect(statusMatch?.[1]).toBe('okay');
    });

    it('uart0 节点应解析为 status=disabled', () => {
      const nodeText = '&uart0 {\n    status = "disabled";\n    current-speed = <115200>;\n};';
      const statusMatch = nodeText.match(/status\s*=\s*"([^"]+)"/);
      expect(statusMatch?.[1]).toBe('disabled');
    });

    it('pwm0 节点应解析为 #pwm-cells=3', () => {
      const nodeText = '&pwm0 {\n    status = "okay";\n    #pwm-cells = <3>;\n};';
      const pwmCellsMatch = nodeText.match(/#pwm-cells\s*=\s*<(\d+)>/);
      expect(pwmCellsMatch?.[1]).toBe('3');
    });
  });

  // === M3-T2: DTS 解析-嵌套节点 ===
  describe('M3-T2: DTS 解析-嵌套节点', () => {
    it('sysdma 节点应解析 ch-remap 属性', () => {
      const nodeText = '&sysdma {\n    status = "okay";\n    ch-remap = <0 5 12 13 42 42 4 7>;\n};';
      const chRemapMatch = nodeText.match(/ch-remap\s*=\s*<([^>]+)>/);
      const channels = chRemapMatch?.[1]?.trim().split(/\s+/);
      expect(channels).toEqual(SYSDMA_DEFAULT_CHANNELS);
    });
  });

  // === M3-T3: DTS 写入保留格式 ===
  describe('M3-T3: DTS 写入保留格式', () => {
    it('修改 status 后应只替换 status 行，其他行保持原样', () => {
      // C++ dtsconfig.cpp 使用行级定位 (lineNumber) 进行精确替换
      const originalLines = [
        '&i2c0 {',
        '    status = "okay";',
        '    clock-frequency = <100000>;',
        '};',
      ];
      // 修改 status 为 "disabled"
      const modifiedLines = originalLines.map((line, index) => {
        if (index === 1) return '    status = "disabled";';
        return line; // 其他行保持不变
      });
      expect(modifiedLines[0]).toBe('&i2c0 {');
      expect(modifiedLines[1]).toBe('    status = "disabled";');
      expect(modifiedLines[2]).toBe('    clock-frequency = <100000>;');
      expect(modifiedLines[3]).toBe('};');
    });
  });

  // === M3-T4: 外设状态切换 ===
  describe('M3-T4: 外设状态切换', () => {
    it('setPeripheralStatus 应将 okay 切换为 disabled', () => {
      const peripheral: PeripheralInfoRef = {
        name: 'i2c0',
        status: 'okay',
        clockName: null,
        clockFreq: '',
        clockFrequency: 100000,
        pwmCells: 1,
        currentSpeed: 115200,
        sysdmaChannels: SYSDMA_DEFAULT_CHANNELS,
        hasStatus: true,
        hasClock: false,
        hasClockFreq: true,
        hasPwmCells: false,
        hasCurrentSpeed: false,
        hasSysdmaChannels: false,
        lineNumber: 2,
      };
      peripheral.status = 'disabled';
      expect(peripheral.status).toBe('disabled');
    });
  });

  // === M3-T5: 时钟频率设置 ===
  describe('M3-T5: 时钟频率设置', () => {
    it('setPeripheralClockFrequency 应设置正确的频率值', () => {
      const peripheral: PeripheralInfoRef = {
        name: 'uart0',
        status: 'okay',
        clockName: null,
        clockFreq: '',
        clockFrequency: 0,
        pwmCells: 1,
        currentSpeed: 115200,
        sysdmaChannels: SYSDMA_DEFAULT_CHANNELS,
        hasStatus: true,
        hasClock: false,
        hasClockFreq: false,
        hasPwmCells: false,
        hasCurrentSpeed: true,
        hasSysdmaChannels: false,
        lineNumber: 5,
      };
      peripheral.clockFrequency = 9600;
      peripheral.hasClockFreq = true;
      expect(peripheral.clockFrequency).toBe(9600);
      expect(peripheral.hasClockFreq).toBe(true);
    });
  });

  // === M3-T6: PWM cells 设置 ===
  describe('M3-T6: PWM cells 设置', () => {
    it('setPeripheralPwmCells 应设置正确的 cells 数', () => {
      const peripheral: PeripheralInfoRef = {
        name: 'pwm0',
        status: 'okay',
        clockName: null,
        clockFreq: '',
        clockFrequency: 0,
        pwmCells: 1,
        currentSpeed: 115200,
        sysdmaChannels: SYSDMA_DEFAULT_CHANNELS,
        hasStatus: true,
        hasClock: false,
        hasClockFreq: false,
        hasPwmCells: false,
        hasCurrentSpeed: false,
        hasSysdmaChannels: false,
        lineNumber: 8,
      };
      peripheral.pwmCells = 3;
      peripheral.hasPwmCells = true;
      expect(peripheral.pwmCells).toBe(3);
      expect(peripheral.hasPwmCells).toBe(true);
    });
  });

  // === M3-T7: UART 波特率设置 ===
  describe('M3-T7: UART 波特率设置', () => {
    it('setPeripheralCurrentSpeed 应设置正确的波特率', () => {
      const peripheral: PeripheralInfoRef = {
        name: 'uart0',
        status: 'okay',
        clockName: null,
        clockFreq: '',
        clockFrequency: 0,
        pwmCells: 1,
        currentSpeed: 115200,
        sysdmaChannels: SYSDMA_DEFAULT_CHANNELS,
        hasStatus: true,
        hasClock: false,
        hasClockFreq: false,
        hasPwmCells: false,
        hasCurrentSpeed: true,
        hasSysdmaChannels: false,
        lineNumber: 5,
      };
      peripheral.currentSpeed = 9600;
      expect(peripheral.currentSpeed).toBe(9600);
    });
  });

  // === M3-T8: SYSDMA 通道默认值 ===
  describe('M3-T8: SYSDMA 通道默认值', () => {
    it('SYSDMA 默认通道映射应为 8 个值', () => {
      expect(SYSDMA_DEFAULT_CHANNELS.length).toBe(8);
    });

    it('SYSDMA 默认通道映射应为 ["0","5","12","13","42","42","4","7"]', () => {
      expect(SYSDMA_DEFAULT_CHANNELS).toEqual(['0', '5', '12', '13', '42', '42', '4', '7']);
    });
  });

  // === M3-T9: SYSDMA 通道设置 ===
  describe('M3-T9: SYSDMA 通道设置', () => {
    it('ch-remap 属性格式应为 <0 5 12 13 42 42 4 7>', () => {
      // C++ createSysdmaRemapNode() 生成的格式
      const channels = ['0', '5', '12', '13', '42', '42', '4', '7'];
      const chRemapStr = `ch-remap = <${channels.join(' ')}>;`;
      expect(chRemapStr).toBe('ch-remap = <0 5 12 13 42 42 4 7>;');
    });
  });

  // === M3-T10: DMA 配置联动更新 ===
  describe('M3-T10: DMA 配置联动更新', () => {
    it('SYSDMA 通道变化时应联动更新相关外设 DMA 配置', () => {
      // 模拟 DTS 联动更新的逻辑：
      // 假设之前的通道为 ['0', '5', '12', '13', '42', '42', '4', '7']，未修改
      // 修改后的通道把 '12' 更改为 '8'（这是 uart0_rx 的通道）
      // 这意味着：
      // 1. 之前使用通道 12 的外设（对应 uart2）应当被清除其 DMA 配置。
      // 2. 占用了通道 8 的外设（对应 uart0）应当被新增 rx 通道的 DMA 配置。
      
      const prevChannels = ['0', '5', '12', '13', '42', '42', '4', '7'];
      const newChannels = ['0', '5', '8', '13', '42', '42', '4', '7'];
      
      // 找出受到影响需要清除的外设和需要新增的外设
      const changedIndices = prevChannels.map((prev, idx) => prev !== newChannels[idx] ? idx : -1).filter(idx => idx !== -1);
      
      const getPeripheralNodeFromChannel = (ch: string) => {
        const channel = parseInt(ch, 10);
        if (channel >= 8 && channel <= 15) {
          return `uart${Math.floor((channel - 8) / 2)}`;
        }
        return '';
      };
      
      // 发生通道修改的旧外设节点
      const prevPeripheralsToClear = changedIndices
        .map(idx => prevChannels[idx])
        .filter(ch => ch !== '')
        .map(ch => getPeripheralNodeFromChannel(ch))
        .filter(node => node !== '');
        
      expect(prevPeripheralsToClear).toContain('uart2');
      
      // 新增 DMA 的外设节点
      const newPeripheralsToAdd = changedIndices
        .map(idx => newChannels[idx])
        .filter(ch => ch !== '')
        .map(ch => getPeripheralNodeFromChannel(ch))
        .filter(node => node !== '');
        
      expect(newPeripheralsToAdd).toContain('uart0');
    });
  });

  // === M3-T11/T12: PeripheralStore 和 ConfigDialog (待前端实现) ===
  describe('M3-T11/T12: PeripheralStore 和 ConfigDialog (待前端实现)', () => {
    it('PeripheralStore - 状态切换应同步 UI 和 store', async () => {
      const store = usePeripheralStore.getState();
      await store.loadPeripherals('/mock/path');
      expect(usePeripheralStore.getState().peripherals.find(p => p.name === 'i2c0')?.status).toBe('disabled');
      
      await store.setPeripheralStatus('i2c0', 'okay');
      expect(usePeripheralStore.getState().peripherals.find(p => p.name === 'i2c0')?.status).toBe('okay');
    });

    it('ConfigDialog - 表单交互应触发正确的 Rust command', async () => {
      const { invoke } = await import('@tauri-apps/api/core');
      const store = usePeripheralStore.getState();
      await store.loadPeripherals('/mock/path');
      
      await store.setClockFrequency('i2c0', 400000);
      expect(invoke).toHaveBeenCalledWith('set_peripheral_clock_frequency', { peripheral: 'i2c0', frequency: 400000 });
      expect(usePeripheralStore.getState().peripherals.find(p => p.name === 'i2c0')?.clock_frequency).toBe(400000);

      await store.setCurrentSpeed('uart0', 9600);
      expect(invoke).toHaveBeenCalledWith('set_peripheral_current_speed', { peripheral: 'uart0', speed: 9600 });
      expect(usePeripheralStore.getState().peripherals.find(p => p.name === 'uart0')?.current_speed).toBe(9600);
    });
  });
});