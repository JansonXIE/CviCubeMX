import { beforeEach, describe, expect, it, vi } from 'vitest';

const mocks = vi.hoisted(() => ({
 defaultPins: [
 {
 pin_num: 'A2',
 pin_name: 'PAD_MIPI_TXM4',
 display_name: 'A2',
 supported_functions: ['XGPIOC_18', 'UART0_TX'],
 default_function: 'XGPIOC_18',
 current_function: 'XGPIOC_18',
 user_configured: false,
 current_state: null,
 },
 ],
 mockPins: [] as any[],
 loadPinDataDeferred: null as null | { promise: Promise<any[]>; resolve: (pins: any[]) => void },
 invokeMock: vi.fn(),
}));

vi.mock('@tauri-apps/api/core', () => ({
 invoke: (cmd: string, args?: any) => mocks.invokeMock(cmd, args),
}));

import { useChipStore } from '../../stores/chipStore';

function resetChipStore() {
 useChipStore.setState({
 chipType: null,
 chipSpec: null,
 pins: new Map(),
 muxFunctions: {},
 searchText: '',
 highlightedPins: new Set(),
 isLoading: false,
 error: null,
 });
}

describe('ChipStore syncBoardInit', () => {
 beforeEach(() => {
 mocks.mockPins = mocks.defaultPins;
 mocks.loadPinDataDeferred = null;
 mocks.invokeMock.mockReset();
 mocks.invokeMock.mockImplementation(async (cmd: string, args?: any) => {
 if (cmd === 'load_chip_spec') {
 return {
 chip_type: args.chipType,
 package: args.chipType === 'cv1842hp' ? 'BGA' : 'QFN',
 pin_count: args.chipType === 'cv1842hp' ?221 :88,
 rows: args.chipType === 'cv1842hp' ? 'ABCDEFGHJKLMNPR' : undefined,
 cols: args.chipType === 'cv1842hp' ?15 : undefined,
 description: 'mock spec',
 };
 }
 if (cmd === 'load_pin_data') {
 if (mocks.loadPinDataDeferred) {
 return mocks.loadPinDataDeferred.promise;
 }
 return mocks.mockPins;
 }
 if (cmd === 'get_mux_functions') {
 return [];
 }
 if (cmd === 'read_board_init_config') {
 return [];
 }
 return undefined;
 });
 resetChipStore();
 });

 it('re-reads board init config and reloads pins for the selected SDK path', async () => {
 await useChipStore.getState().selectChip('cv1842hp');
 mocks.mockPins = [
 {
 ...mocks.defaultPins[0],
 current_function: 'UART0_TX',
 user_configured: true,
 },
 ];

 await useChipStore.getState().syncBoardInit('/sdk-b');

 expect(mocks.invokeMock).toHaveBeenCalledWith('read_board_init_config', {
 sdkPath: '/sdk-b',
 chipType: 'cv1842hp',
 });
 const pin = useChipStore.getState().pins.get('PAD_MIPI_TXM4');
 expect(pin?.current_function).toBe('UART0_TX');
 expect(pin?.user_configured).toBe(true);
 });

 it('does not let a stale sync result overwrite pins after chip type changes', async () => {
 await useChipStore.getState().selectChip('cv1842hp');
 mocks.loadPinDataDeferred = {} as NonNullable<typeof mocks.loadPinDataDeferred>;
 mocks.loadPinDataDeferred.promise = new Promise((resolve) => {
 mocks.loadPinDataDeferred!.resolve = resolve;
 });

 const syncing = useChipStore.getState().syncBoardInit('/sdk-a');
 useChipStore.setState({ chipType: 'cv1842cp', pins: new Map() });
 mocks.loadPinDataDeferred.resolve([
 {
 ...mocks.defaultPins[0],
 current_function: 'UART0_TX',
 user_configured: true,
 },
 ]);
 await syncing;

 expect(useChipStore.getState().chipType).toBe('cv1842cp');
 expect(useChipStore.getState().pins.has('PAD_MIPI_TXM4')).toBe(false);
 });
});
