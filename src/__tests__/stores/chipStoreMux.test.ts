// ============================================================
// 二级引脚复用 (MUX_SPI1_*) 功能选择 - chipStore 行为测试
// ============================================================

import { describe, it, expect, vi, beforeEach } from 'vitest';

const invokeMock = vi.fn(async (cmd: string, args?: any) => {
  if (cmd === 'load_chip_spec') {
    return {
      chip_type: args.chipType,
      package: 'BGA',
      pin_count: 221,
      rows: 'ABCDEFGHJKLMNPR',
      cols: 15,
      description: 'mock spec',
    };
  }
  if (cmd === 'load_pin_data') {
    return [
      {
        pin_num: 'A6',
        pin_name: 'PAD_MIPIRX3N',
        display_name: 'A6',
        supported_functions: ['XGPIOC_4', 'CAM_MCLK0', 'MUX_SPI1_MISO'],
        default_function: 'XGPIOC_4',
        current_function: 'XGPIOC_4',
        user_configured: false,
        current_state: null,
      },
    ];
  }
  if (cmd === 'get_mux_functions') {
    return [
      {
        name: 'MUX_SPI1_MISO',
        default: 'XGPIOB_8',
        options: ['UART3_RTS', 'IIC1_SDA', 'XGPIOB_8', 'PWM_9', 'KEY_COL1', 'SPI1_SDI', 'DBG_14'],
      },
    ];
  }
  return undefined;
});

vi.mock('@tauri-apps/api/core', () => ({ invoke: (cmd: string, args?: any) => invokeMock(cmd, args) }));

import { useChipStore } from '../../stores/chipStore';

describe('二级 mux 功能选择', () => {
  beforeEach(async () => {
    invokeMock.mockClear();
    await useChipStore.getState().selectChip('cv1842hp');
  });

  it('selectChip 后应加载 muxFunctions 定义', () => {
    const { muxFunctions } = useChipStore.getState();
    expect(muxFunctions['MUX_SPI1_MISO']).toBeDefined();
    expect(muxFunctions['MUX_SPI1_MISO'].default).toBe('XGPIOB_8');
    expect(muxFunctions['MUX_SPI1_MISO'].options).toHaveLength(7);
  });

  it('选中 MUX 一级功能时 current_state 默认回填该 mux 默认值', async () => {
    await useChipStore.getState().setPinFunction('PAD_MIPIRX3N', 'MUX_SPI1_MISO');
    const pin = useChipStore.getState().pins.get('PAD_MIPIRX3N');
    expect(pin?.current_function).toBe('MUX_SPI1_MISO');
    expect(pin?.current_state).toBe('XGPIOB_8');
    expect(pin?.user_configured).toBe(true);
    // 传给后端的 state 应为默认二级功能
    expect(invokeMock).toHaveBeenCalledWith('set_pin_function', {
      chipType: 'cv1842hp',
      pinName: 'PAD_MIPIRX3N',
      function: 'MUX_SPI1_MISO',
      state: 'XGPIOB_8',
    });
  });

  it('setPinSubFunction 更新 current_state 且保持一级功能不变', async () => {
    await useChipStore.getState().setPinFunction('PAD_MIPIRX3N', 'MUX_SPI1_MISO');
    await useChipStore.getState().setPinSubFunction('PAD_MIPIRX3N', 'PWM_9');
    const pin = useChipStore.getState().pins.get('PAD_MIPIRX3N');
    expect(pin?.current_function).toBe('MUX_SPI1_MISO');
    expect(pin?.current_state).toBe('PWM_9');
    expect(invokeMock).toHaveBeenLastCalledWith('set_pin_function', {
      chipType: 'cv1842hp',
      pinName: 'PAD_MIPIRX3N',
      function: 'MUX_SPI1_MISO',
      state: 'PWM_9',
    });
  });

  it('切换回非 MUX 功能时 current_state 清空', async () => {
    await useChipStore.getState().setPinFunction('PAD_MIPIRX3N', 'MUX_SPI1_MISO');
    await useChipStore.getState().setPinFunction('PAD_MIPIRX3N', 'CAM_MCLK0');
    const pin = useChipStore.getState().pins.get('PAD_MIPIRX3N');
    expect(pin?.current_function).toBe('CAM_MCLK0');
    expect(pin?.current_state).toBeNull();
  });
});
