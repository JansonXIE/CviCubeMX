import { describe, it, expect, vi, beforeEach } from 'vitest';

const invokeMock = vi.fn();
vi.mock('@tauri-apps/api/core', () => ({ invoke: (...a: any[]) => invokeMock(...a) }));

import { usePeripheralStore } from '../stores/peripheralStore';

describe('raw property store actions', () => {
  beforeEach(() => {
    invokeMock.mockReset();
    usePeripheralStore.setState({ peripherals: [], dtsContent: '' });
  });

  // B1. getRawProperties 透传命令名与参数
  it('getRawProperties invokes correct command', async () => {
    invokeMock.mockResolvedValueOnce([
      { key: 'status', value: 'okay', kind: 'string', protected: false },
    ]);
    const res = await usePeripheralStore.getState().getRawProperties('i2c0');
    expect(invokeMock).toHaveBeenCalledWith('get_peripheral_raw_properties', { peripheral: 'i2c0' });
    expect(res[0].key).toBe('status');
  });

  // B2. setRawProperty 透传全部参数并刷新 DTS 预览
  it('setRawProperty invokes set + fetchDtsContent', async () => {
    invokeMock.mockResolvedValueOnce(undefined)      // set_peripheral_raw_property
             .mockResolvedValueOnce('NEW DTS CONTENT'); // get_dts_content
    await usePeripheralStore.getState().setRawProperty('spi0', 'spi-max-frequency', '50000000', 'cell');
    expect(invokeMock).toHaveBeenNthCalledWith(1, 'set_peripheral_raw_property', {
      peripheral: 'spi0', key: 'spi-max-frequency', value: '50000000', kind: 'cell',
    });
    expect(invokeMock).toHaveBeenNthCalledWith(2, 'get_dts_content');
    expect(usePeripheralStore.getState().dtsContent).toBe('NEW DTS CONTENT');
  });

  // B3. deleteRawProperty 透传命令并刷新
  it('deleteRawProperty invokes delete + fetchDtsContent', async () => {
    invokeMock.mockResolvedValueOnce(undefined).mockResolvedValueOnce('DTS');
    await usePeripheralStore.getState().deleteRawProperty('i2c0', 'wakeup-source');
    expect(invokeMock).toHaveBeenNthCalledWith(1, 'delete_peripheral_raw_property', {
      peripheral: 'i2c0', key: 'wakeup-source',
    });
  });

  // B4. 错误冒泡
  it('setRawProperty propagates backend error', async () => {
    invokeMock.mockRejectedValueOnce('属性 compatible 受保护，不可修改');
    await expect(
      usePeripheralStore.getState().setRawProperty('i2c0', 'compatible', 'x', 'string')
    ).rejects.toBeTruthy();
  });
});
