import { create } from 'zustand';
import { invoke } from '@tauri-apps/api/core';

export interface FlashPartition {
  partition_number: number;
  label: string;
  size: number;
  size_string: string;
  file: string;
  mountpoint: string;
  type_field: string;
  enabled: boolean;
}

interface FlashState {
  partitions: FlashPartition[];
  isLoading: boolean;
  error: string | null;

  // Actions
  loadPartitions: () => Promise<void>;
  addPartition: (partition: FlashPartition) => void;
  removePartition: (partitionNumber: number) => void;
  validatePartitions: () => Promise<void>;
  exportDefconfig: (sourcePath: string, chipType: string) => Promise<void>;
  exportFlashJson: (path: string) => Promise<void>;
}

export const useFlashStore = create<FlashState>((set, get) => ({
  partitions: [],
  isLoading: false,
  error: null,

  loadPartitions: async () => {
    set({ isLoading: true, error: null });
    try {
      const list = await invoke<FlashPartition[]>('load_partitions');
      set({ partitions: list, isLoading: false });
    } catch (err) {
      set({ error: String(err), isLoading: false });
    }
  },

  addPartition: (partition: FlashPartition) => {
    const { partitions } = get();
    if (partitions.some((p) => p.partition_number === partition.partition_number)) {
      set({ error: `Partition number ${partition.partition_number} already exists` });
      return;
    }
    set({ partitions: [...partitions, partition] });
  },

  removePartition: (partitionNumber: number) => {
    const { partitions } = get();
    set({ partitions: partitions.filter((p) => p.partition_number !== partitionNumber) });
  },

  validatePartitions: async () => {
    set({ isLoading: true, error: null });
    try {
      await invoke('validate_partitions', { partitions: get().partitions });
      set({ isLoading: false });
    } catch (err) {
      set({ error: String(err), isLoading: false });
      throw err;
    }
  },

  exportDefconfig: async (sourcePath: string, chipType: string) => {
    set({ isLoading: true, error: null });
    try {
      await invoke('export_flash_defconfig', {
        partitions: get().partitions,
        sourcePath,
        chipType,
      });
      set({ isLoading: false });
    } catch (err) {
      set({ error: String(err), isLoading: false });
      throw err;
    }
  },

  exportFlashJson: async (path: string) => {
    set({ isLoading: true, error: null });
    try {
      await invoke('export_flash_json', {
        partitions: get().partitions,
        path,
      });
      set({ isLoading: false });
    } catch (err) {
      set({ error: String(err), isLoading: false });
      throw err;
    }
  },
}));

export type FlashStore = FlashState;
