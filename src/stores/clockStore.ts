import { create } from 'zustand';
import { invoke } from '@tauri-apps/api/core';

export interface PllConfig {
  name: string;
  enabled: boolean;
  inputFreq: number;      // 前端使用驼峰，已在 Rust 中使用 rename_all = "camelCase" 支持
  outputFreq: number;
  divider: number;
  multiplier: number;
  source: string;
}

export interface ClockOutput {
  name: string;
  source: string;
  divider: number;
  multiplier: number;
  frequency: number;
  enabled: boolean;
}

export interface ModulePosition {
  moduleName: string;
  x: number;
  y: number;
  width: number;
  height: number;
}

export type PLLConfig = PllConfig;

export interface ClockTreeResult {
  pllConfigs: Map<string, PLLConfig> & Record<string, PLLConfig>;
  outputs: Map<string, ClockOutput> & Record<string, ClockOutput>;
  subNodes: Record<string, Map<string, ClockOutput> & Record<string, ClockOutput>>;
}

interface ClockState {
  pllConfigs: Map<string, PLLConfig> & Record<string, PLLConfig>;
  outputs: Map<string, ClockOutput> & Record<string, ClockOutput>;
  subNodes: Record<string, Map<string, ClockOutput> & Record<string, ClockOutput>>;
  modulePositions: Map<string, ModulePosition> & Record<string, ModulePosition>;
  searchText: string;
  isLoading: boolean;
  error: string | null;

  // Actions
  computeClockTree: (configs: Map<string, PLLConfig> & Record<string, PLLConfig> | Record<string, PLLConfig>) => Promise<void>;
  loadModulePositions: () => Promise<void>;
  saveModulePositions: (positions: Map<string, ModulePosition> & Record<string, ModulePosition> | Record<string, ModulePosition>) => Promise<void>;
  exportClockDefconfig: (sourcePath: string, chipType: string, configs: Map<string, PLLConfig> & Record<string, PLLConfig> | Record<string, PLLConfig>) => Promise<void>;
  searchClock: (text: string) => void;
}

export const useClockStore = create<ClockState>((set) => ({
  pllConfigs: {} as Map<string, PLLConfig> & Record<string, PLLConfig>,
  outputs: {} as Map<string, ClockOutput> & Record<string, ClockOutput>,
  subNodes: {},
  modulePositions: {} as Map<string, ModulePosition> & Record<string, ModulePosition>,
  searchText: '',
  isLoading: false,
  error: null,

  computeClockTree: async (configs) => {
    set({ isLoading: true, error: null });
    try {
      const result = await invoke<ClockTreeResult>('compute_clock_tree', { configs });
      set({
        pllConfigs: result.pllConfigs,
        outputs: result.outputs,
        subNodes: result.subNodes,
        isLoading: false,
      });
    } catch (err) {
      set({ error: String(err), isLoading: false });
    }
  },

  loadModulePositions: async () => {
    set({ isLoading: true, error: null });
    try {
      const positions = await invoke<Record<string, ModulePosition>>('load_module_positions');
      set({ modulePositions: positions as unknown as Map<string, ModulePosition> & Record<string, ModulePosition>, isLoading: false });
    } catch (err) {
      set({ error: String(err), isLoading: false });
    }
  },

  saveModulePositions: async (positions) => {
    set({ isLoading: true, error: null });
    try {
      await invoke('save_module_positions', { positions });
      set({ modulePositions: positions as unknown as Map<string, ModulePosition> & Record<string, ModulePosition>, isLoading: false });
    } catch (err) {
      set({ error: String(err), isLoading: false });
    }
  },

  exportClockDefconfig: async (sourcePath, chipType, configs) => {
    set({ isLoading: true, error: null });
    try {
      await invoke('export_clock_defconfig', { sourcePath, chipType, configs });
      set({ isLoading: false });
    } catch (err) {
      set({ error: String(err), isLoading: false });
      throw err;
    }
  },

  searchClock: (text: string) => {
    set({ searchText: text });
  },
}));

export type ClockStore = ClockState;
