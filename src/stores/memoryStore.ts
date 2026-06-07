import { create } from 'zustand';
import { invoke } from '@tauri-apps/api/core';

export interface MemoryRegion {
  name: string;
  start_address: number;
  end_address: number;
  size: number;
  size_string: string;
  is_editable: boolean;
  description: string;
}

interface MemoryState {
  regions: MemoryRegion[];
  isLoading: boolean;
  error: string | null;

  // Actions
  loadMemoryRegions: () => Promise<void>;
  addRegion: (region: MemoryRegion) => void;
  removeRegion: (name: string) => void;
  validateMemory: () => Promise<void>;
  exportDefconfig: (sourcePath: string, chipType: string) => Promise<void>;
  exportMemoryJson: (path: string) => Promise<void>;
}

export const useMemoryStore = create<MemoryState>((set, get) => ({
  regions: [],
  isLoading: false,
  error: null,

  loadMemoryRegions: async () => {
    set({ isLoading: true, error: null });
    try {
      const list = await invoke<MemoryRegion[]>('load_memory_regions');
      set({ regions: list, isLoading: false });
    } catch (err) {
      set({ error: String(err), isLoading: false });
    }
  },

  addRegion: (region: MemoryRegion) => {
    const { regions } = get();
    if (regions.some((r) => r.name === region.name)) {
      set({ error: `Region ${region.name} already exists` });
      return;
    }
    set({ regions: [...regions, region] });
  },

  removeRegion: (name: string) => {
    const { regions } = get();
    set({ regions: regions.filter((r) => r.name !== name) });
  },

  validateMemory: async () => {
    set({ isLoading: true, error: null });
    try {
      await invoke('validate_memory', { regions: get().regions });
      set({ isLoading: false });
    } catch (err) {
      set({ error: String(err), isLoading: false });
      throw err;
    }
  },

  exportDefconfig: async (sourcePath: string, chipType: string) => {
    set({ isLoading: true, error: null });
    try {
      await invoke('export_memory_defconfig', {
        regions: get().regions,
        sourcePath,
        chipType,
      });
      set({ isLoading: false });
    } catch (err) {
      set({ error: String(err), isLoading: false });
      throw err;
    }
  },

  exportMemoryJson: async (path: string) => {
    set({ isLoading: true, error: null });
    try {
      await invoke('export_memory_json', {
        regions: get().regions,
        path,
      });
      set({ isLoading: false });
    } catch (err) {
      set({ error: String(err), isLoading: false });
      throw err;
    }
  },
}));

export type MemoryStore = MemoryState;
