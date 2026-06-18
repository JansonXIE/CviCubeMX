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
  warnings: string[];

  // Actions
  loadMemoryRegions: () => Promise<void>;
  addRegion: (region: MemoryRegion) => void;
  removeRegion: (name: string) => void;
  updateRegion: (name: string, updated: Partial<MemoryRegion>) => void;
  validateMemory: () => Promise<string[]>;
  exportDefconfig: (sourcePath: string, chipType: string) => Promise<void>;
  exportMemoryJson: (path: string) => Promise<void>;
}

export const useMemoryStore = create<MemoryState>((set, get) => ({
  regions: [],
  isLoading: false,
  error: null,
  warnings: [],

  loadMemoryRegions: async () => {
    set({ isLoading: true, error: null, warnings: [] });
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

  updateRegion: (name: string, updated: Partial<MemoryRegion>) => {
    const { regions } = get();
    
    // 1. 深拷贝当前的内存区域列表
    let newRegions = regions.map((r) => {
      if (r.name === name) {
        const nextReg = { ...r, ...updated };
        // 普通区域在修改 start 或 size 时，自动级联重算 end_address
        if (name !== "ION" && name !== "RTOS_ION") {
          nextReg.end_address = nextReg.start_address + nextReg.size;
        }
        return nextReg;
      }
      return { ...r };
    });

    const target = newRegions.find((r) => r.name === name);
    if (!target) return;

    const baseAddr = 0x80000000;
    const totalSpan = 0x10000000; // 256MB
    const endBoundary = baseAddr + totalSpan; // 0x90000000 (256M 边界)

    // 2. 级联联动计算 (对照 memoryconfig.cpp 逻辑)
    if (name === "ION") {
      // ION 区域：End Address 基于 RTOS_ION 的 Start Address；如果没有则默认为 256M边界 - 96M
      const rtosIon = newRegions.find((r) => r.name === "RTOS_ION");
      if (rtosIon) {
        target.end_address = rtosIon.start_address;
      } else {
        target.end_address = endBoundary - 96 * 1024 * 1024;
      }
      // 计算新的起始地址
      target.start_address = target.end_address - target.size;

      // H26X_BITSTREAM、H26X_ENC_BUFF、ISP_MEM_BASE 区域与 ION 共享起始物理地址
      const relatedRegions = ["H26X_BITSTREAM", "H26X_ENC_BUFF", "ISP_MEM_BASE"];
      newRegions = newRegions.map((r) => {
        if (relatedRegions.includes(r.name)) {
          const start = target.start_address;
          return {
            ...r,
            start_address: start,
            end_address: start + r.size,
          };
        }
        return r;
      });
    } else if (name === "RTOS_ION") {
      // RTOS_ION 区域：结束物理地址固定在 256M 边界，起始物理地址向前调整
      target.end_address = endBoundary;
      target.start_address = target.end_address - target.size;

      // 当 RTOS_ION 大小改变时，需要调整 ION 及其级联子区域的地址
      const newIonEndAddress = target.start_address;

      newRegions = newRegions.map((r) => {
        if (r.name === "ION") {
          const start = newIonEndAddress - r.size;
          return {
            ...r,
            start_address: start,
            end_address: newIonEndAddress,
          };
        }
        return r;
      });

      // 同步获取更新后的 ION 起始地址，并将 H26X_BITSTREAM、H26X_ENC_BUFF、ISP_MEM_BASE 与其同步对齐
      const updatedIon = newRegions.find((r) => r.name === "ION");
      if (updatedIon) {
        const relatedRegions = ["H26X_BITSTREAM", "H26X_ENC_BUFF", "ISP_MEM_BASE"];
        newRegions = newRegions.map((r) => {
          if (relatedRegions.includes(r.name)) {
            const start = updatedIon.start_address;
            return {
              ...r,
              start_address: start,
              end_address: start + r.size,
            };
          }
          return r;
        });
      }
    }

    set({ regions: newRegions });
  },

  validateMemory: async () => {
    set({ isLoading: true, error: null, warnings: [] });
    try {
      // 后端返回重叠警告列表（信息性，不代表失败）；硬错误会以异常形式抛出
      const warnings = await invoke<string[]>('validate_memory', { regions: get().regions });
      set({ isLoading: false, warnings });
      return warnings;
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
