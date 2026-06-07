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

interface PeripheralState {
  peripherals: PeripheralInfo[];
  isLoading: boolean;
  error: string | null;

  // Actions
  loadPeripherals: (path: string) => Promise<void>;
  setPeripheralStatus: (name: string, status: string) => Promise<void>;
  setClockFrequency: (name: string, frequency: number) => Promise<void>;
  setPwmCells: (name: string, cells: number) => Promise<void>;
  setCurrentSpeed: (name: string, speed: number) => Promise<void>;
  setSysdmaChannels: (name: string, channels: string[]) => Promise<void>;
}

export const usePeripheralStore = create<PeripheralState>((set, get) => ({
  peripherals: [],
  isLoading: false,
  error: null,

  loadPeripherals: async (path: string) => {
    set({ isLoading: true, error: null });
    try {
      const list = await invoke<PeripheralInfo[]>('load_dts_peripherals', { filePath: path });
      set({ peripherals: list, isLoading: false });
    } catch (err) {
      set({ error: String(err), isLoading: false });
    }
  },

  setPeripheralStatus: async (name: string, status: string) => {
    try {
      await invoke('set_peripheral_status', { peripheral: name, status });
      const updated = get().peripherals.map((p) =>
        p.name === name ? { ...p, status } : p
      );
      set({ peripherals: updated });
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
    } catch (err) {
      set({ error: String(err) });
    }
  },
}));

export type PeripheralStore = PeripheralState;
