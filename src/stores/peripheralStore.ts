import { create } from 'zustand';
import { invoke } from '@tauri-apps/api/core';

export interface PeripheralInfo {
  name: string;
  status: string;
  clock_name: string;
  clock_freq: string;
  clock_frequency: number;
  pwm_cells: number;
  current_speed: number;
  sysdma_channels: string[];
  has_status: boolean;
  has_clock: boolean;
  has_clock_freq: boolean;
  has_pwm_cells: boolean;
  has_current_speed: boolean;
  has_sysdma_channels: boolean;
  line_number: number;
}

/** A single raw property parsed from a DTS node (generic key/value editor). */
export interface RawProperty {
  key: string;
  value: string;
  kind: 'cell' | 'string' | 'bool';
  protected: boolean;
}

interface PeripheralState {
  peripherals: PeripheralInfo[];
  isLoading: boolean;
  error: string | null;
  dtsContent: string;

  // Actions
  loadPeripherals: (path: string) => Promise<void>;
  fetchDtsContent: () => Promise<void>;
  setPeripheralStatus: (name: string, status: string) => Promise<void>;
  setClockFrequency: (name: string, frequency: number) => Promise<void>;
  setPwmCells: (name: string, cells: number) => Promise<void>;
  setCurrentSpeed: (name: string, speed: number) => Promise<void>;
  setSysdmaChannels: (name: string, channels: string[]) => Promise<void>;

  // Raw property actions (generic key/value editor)
  getRawProperties: (name: string) => Promise<RawProperty[]>;
  setRawProperty: (name: string, key: string, value: string, kind: string) => Promise<void>;
  deleteRawProperty: (name: string, key: string) => Promise<void>;
}

export const usePeripheralStore = create<PeripheralState>((set, get) => ({
  peripherals: [],
  isLoading: false,
  error: null,
  dtsContent: '',

  loadPeripherals: async (path: string) => {
    set({ isLoading: true, error: null });
    try {
      const list = await invoke<PeripheralInfo[]>('load_dts_peripherals', { filePath: path });
      set({ peripherals: list, isLoading: false });
      await get().fetchDtsContent();
    } catch (err) {
      set({ error: String(err), isLoading: false });
    }
  },

  fetchDtsContent: async () => {
    try {
      const content = await invoke<string>('get_dts_content');
      set({ dtsContent: content });
    } catch (err) {
      console.error('Failed to fetch DTS content:', err);
    }
  },

  setPeripheralStatus: async (name: string, status: string) => {
    try {
      await invoke('set_peripheral_status', { peripheral: name, status });
      const updated = get().peripherals.map((p) =>
        p.name === name ? { ...p, status } : p
      );
      set({ peripherals: updated });
      await get().fetchDtsContent();
    } catch (err) {
      set({ error: String(err) });
    }
  },

  setClockFrequency: async (name: string, frequency: number) => {
    try {
      await invoke('set_peripheral_clock_frequency', { peripheral: name, frequency });
      const updated = get().peripherals.map((p) =>
        p.name === name ? { ...p, clock_frequency: frequency } : p
      );
      set({ peripherals: updated });
      await get().fetchDtsContent();
    } catch (err) {
      set({ error: String(err) });
    }
  },

  setPwmCells: async (name: string, cells: number) => {
    try {
      await invoke('set_peripheral_pwm_cells', { peripheral: name, cells });
      const updated = get().peripherals.map((p) =>
        p.name === name ? { ...p, pwm_cells: cells } : p
      );
      set({ peripherals: updated });
      await get().fetchDtsContent();
    } catch (err) {
      set({ error: String(err) });
    }
  },

  setCurrentSpeed: async (name: string, speed: number) => {
    try {
      await invoke('set_peripheral_current_speed', { peripheral: name, speed });
      const updated = get().peripherals.map((p) =>
        p.name === name ? { ...p, current_speed: speed } : p
      );
      set({ peripherals: updated });
      await get().fetchDtsContent();
    } catch (err) {
      set({ error: String(err) });
    }
  },

  setSysdmaChannels: async (name: string, channels: string[]) => {
    try {
      await invoke('set_peripheral_sysdma_channels', { peripheral: name, channels });
      const updated = get().peripherals.map((p) =>
        p.name === name ? { ...p, sysdma_channels: channels } : p
      );
      set({ peripherals: updated });
      await get().fetchDtsContent();
    } catch (err) {
      set({ error: String(err) });
    }
  },

  // ── Raw property actions ──────────────────────────────────
  // Errors intentionally propagate (no swallow) so the ConfigDialog can keep
  // the dialog open and surface failures (e.g. protected-property rejection).
  getRawProperties: async (name: string) =>
    await invoke<RawProperty[]>('get_peripheral_raw_properties', { peripheral: name }),

  setRawProperty: async (name: string, key: string, value: string, kind: string) => {
    await invoke('set_peripheral_raw_property', { peripheral: name, key, value, kind });
    await get().fetchDtsContent();
  },

  deleteRawProperty: async (name: string, key: string) => {
    await invoke('delete_peripheral_raw_property', { peripheral: name, key });
    await get().fetchDtsContent();
  },
}));

export type PeripheralStore = PeripheralState;
