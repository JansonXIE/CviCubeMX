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

export interface FlashBoardInfo {
  flash_size: string;
  flash_size_kb: number;
  partition_count: number;
  partitions: FlashPartition[];
}

// 默认 Flash 容量：32GB（KB），在读取板卡信息前作为占位
const DEFAULT_FLASH_SIZE_KB = 32 * 1024 * 1024;

/** 将 KB 容量格式化为 GB/MB/KB（对齐 Rust format_flash_size） */
export function formatFlashKb(sizeKb: number): string {
  if (sizeKb <= 0) return '0KB';
  const MB = 1024;
  const GB = MB * 1024;
  if (sizeKb >= GB) {
    const v = sizeKb / GB;
    return Number.isInteger(v) ? `${v}GB` : `${v.toFixed(2)}GB`;
  }
  if (sizeKb >= MB) {
    const v = sizeKb / MB;
    return Number.isInteger(v) ? `${v}MB` : `${v.toFixed(2)}MB`;
  }
  return `${sizeKb}KB`;
}

interface FlashBaseline {
  partitions: FlashPartition[];
  flashSize: string;
  flashSizeKb: number;
  partitionCount: number;
}

interface FlashState {
  partitions: FlashPartition[];
  flashSize: string;
  flashSizeKb: number;
  partitionCount: number;
  // 板卡载入时的基线快照，供「重置」恢复（即使已保存到 defconfig 也能撤回本次会话的修改）
  baseline: FlashBaseline | null;
  isLoading: boolean;
  error: string | null;

  // Actions
  loadBoardInfo: (sourcePath: string, chipType: string) => Promise<void>;
  loadPartitions: () => Promise<void>;
  addPartition: (partition: FlashPartition) => void;
  updatePartitionSize: (partitionNumber: number, sizeKb: number) => void;
  removePartition: (partitionNumber: number) => void;
  resetToBaseline: () => void;
  validatePartitions: () => Promise<void>;
  exportDefconfig: (sourcePath: string, chipType: string) => Promise<void>;
  exportFlashJson: (path: string) => Promise<void>;
}

export const useFlashStore = create<FlashState>((set, get) => ({
  partitions: [],
  flashSize: '32GB',
  flashSizeKb: DEFAULT_FLASH_SIZE_KB,
  partitionCount: 0,
  baseline: null,
  isLoading: false,
  error: null,

  // 从 SDK 对应板卡的 defconfig 读取真实分区信息，并记录为「重置」基线
  loadBoardInfo: async (sourcePath: string, chipType: string) => {
    set({ isLoading: true, error: null });
    try {
      const info = await invoke<FlashBoardInfo>('read_flash_board_info', {
        sourcePath,
        chipType,
      });
      set({
        partitions: info.partitions,
        flashSize: info.flash_size,
        flashSizeKb: info.flash_size_kb,
        partitionCount: info.partition_count,
        baseline: {
          partitions: info.partitions.map((p) => ({ ...p })),
          flashSize: info.flash_size,
          flashSizeKb: info.flash_size_kb,
          partitionCount: info.partition_count,
        },
        isLoading: false,
      });
    } catch (err) {
      set({ error: String(err), isLoading: false });
    }
  },

  loadPartitions: async () => {
    set({ isLoading: true, error: null });
    try {
      const list = await invoke<FlashPartition[]>('load_partitions');
      set({
        partitions: list,
        baseline: {
          partitions: list.map((p) => ({ ...p })),
          flashSize: get().flashSize,
          flashSizeKb: get().flashSizeKb,
          partitionCount: get().partitionCount,
        },
        isLoading: false,
      });
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

  // 仅修改分区大小（size 为 0 表示自动分配剩余空间，如 DATA 分区）
  updatePartitionSize: (partitionNumber: number, sizeKb: number) => {
    const { partitions } = get();
    set({
      partitions: partitions.map((p) =>
        p.partition_number === partitionNumber
          ? { ...p, size: sizeKb, size_string: formatFlashKb(sizeKb) }
          : p
      ),
    });
  },

  removePartition: (partitionNumber: number) => {
    const { partitions } = get();
    set({ partitions: partitions.filter((p) => p.partition_number !== partitionNumber) });
  },

  // 重置：撤回本次会话对当前板卡的所有修改，恢复到载入时的基线快照。
  // 注意：这只恢复内存中的分区表，不会改动磁盘；如需持久化需再次「保存并导出」。
  resetToBaseline: () => {
    const { baseline } = get();
    if (!baseline) return;
    set({
      partitions: baseline.partitions.map((p) => ({ ...p })),
      flashSize: baseline.flashSize,
      flashSizeKb: baseline.flashSizeKb,
      partitionCount: baseline.partitionCount,
      error: null,
    });
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
