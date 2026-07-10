import { create } from "zustand";
import { invoke } from "@tauri-apps/api/core";

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
  updateRegion: (
    name: string,
    updated: Partial<MemoryRegion>,
    chipType?: string,
  ) => void;
  validateMemory: () => Promise<string[]>;
  exportDefconfig: (sourcePath: string, chipType: string) => Promise<void>;
  exportMemoryJson: (path: string) => Promise<void>;
}

// 物理内存基地址（CV184x 系列固定为 0x80000000）
export const MEMORY_BASE_ADDRESS = 0x80000000;

// 默认 DRAM 容量：256MB（与历史行为保持一致），用于未指定/未识别的芯片
export const DEFAULT_DRAM_SIZE = 0x10000000;

// 不同芯片型号对应的 DRAM 容量
// 对应 build/boards/cv184x/<board>/<board>_defconfig 中的 CONFIG_DRAM_SIZE
export const DRAM_SIZE_BY_CHIP: Record<string, number> = {
  cv1841: 0x08000000, // 128MB
  cv1842: 0x10000000, // 256MB
  cv1843: 0x20000000, // 512MB
};

/**
 * 根据芯片型号推导 DRAM 容量（字节）。
 * chipType 可以是完整板名（如 "cv1842hp_wevb_0014a_emmc"），按型号前缀匹配；
 * 未识别时回退到默认 256MB，保证旧调用方行为不变。
 */
export function getDramSizeByChip(chipType?: string | null): number {
  if (!chipType) return DEFAULT_DRAM_SIZE;
  const lc = chipType.toLowerCase();
  for (const [model, size] of Object.entries(DRAM_SIZE_BY_CHIP)) {
    if (lc.includes(model)) return size;
  }
  return DEFAULT_DRAM_SIZE;
}

export const useMemoryStore = create<MemoryState>((set, get) => ({
  regions: [],
  isLoading: false,
  error: null,
  warnings: [],

  loadMemoryRegions: async () => {
    set({ isLoading: true, error: null, warnings: [] });
    try {
      const list = await invoke<MemoryRegion[]>("load_memory_regions");
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

  updateRegion: (
    name: string,
    updated: Partial<MemoryRegion>,
    chipType?: string,
  ) => {
    const { regions } = get();

    const syncBootlogoToIonEnd = (list: MemoryRegion[], ionEnd: number) =>
      list.map((r) => {
        if (r.name === "BOOTLOGO") {
          return {
            ...r,
            start_address: ionEnd - r.size,
            end_address: ionEnd,
          };
        }
        return r;
      });

    //1. 深拷贝当前的内存区域列表
    let newRegions = regions.map((r) => {
      if (r.name === name) {
        const nextReg = { ...r, ...updated };
        // 普通区域在修改 start 或 size 时，自动级联重算 end_address
        if (name !== "ION" && name !== "RTOS_ION" && name !== "BOOTLOGO") {
          nextReg.end_address = nextReg.start_address + nextReg.size;
        }
        return nextReg;
      }
      return { ...r };
    });

    const target = newRegions.find((r) => r.name === name);
    if (!target) return;

    const baseAddr = MEMORY_BASE_ADDRESS;
    // DRAM末尾边界随芯片型号变化：cv1841=128M / cv1842=256M / cv1843=512M
    // 未指定芯片时回退默认256M，保持历史行为不变
    const dramSize = getDramSizeByChip(chipType);
    const endBoundary = baseAddr + dramSize; // RTOS_ION 钉死的 DDR末尾

    //2.级联联动计算 (对照 memoryconfig.cpp逻辑)
    if (name === "ION") {
      // ION 区域：End Address 基于 RTOS_ION 的 Start Address；如果没有则默认为256M边界 -96M
      const rtosIon = newRegions.find((r) => r.name === "RTOS_ION");
      if (rtosIon) {
        target.end_address = rtosIon.start_address;
      } else {
        target.end_address = endBoundary - 96 * 1024 * 1024;
      }
      //计算新的起始地址
      target.start_address = target.end_address - target.size;

      // BOOTLOGO 位于 ION 尾部，end 与 ION end 保持一致
      newRegions = syncBootlogoToIonEnd(newRegions, target.end_address);

      // H26X_BITSTREAM、H26X_ENC_BUFF、ISP_MEM_BASE 区域与 ION共享起始物理地址
      const relatedRegions = [
        "H26X_BITSTREAM",
        "H26X_ENC_BUFF",
        "ISP_MEM_BASE",
      ];
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
      // RTOS_ION 区域：结束物理地址固定在256M 边界，起始物理地址向前调整
      target.end_address = endBoundary;
      target.start_address = target.end_address - target.size;

      // 当 RTOS_ION 大小改变时，需要调整 ION及其级联子区域的地址
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
        newRegions = syncBootlogoToIonEnd(newRegions, updatedIon.end_address);

        const relatedRegions = [
          "H26X_BITSTREAM",
          "H26X_ENC_BUFF",
          "ISP_MEM_BASE",
        ];
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
    } else if (name === "BOOTLOGO") {
      const ion = newRegions.find((r) => r.name === "ION");
      if (ion) {
        target.end_address = ion.end_address;
        target.start_address = target.end_address - target.size;
      } else {
        target.end_address = target.start_address + target.size;
      }
    }

    set({ regions: newRegions });
  },

  validateMemory: async () => {
    set({ isLoading: true, error: null, warnings: [] });
    try {
      // 后端返回重叠警告列表（信息性，不代表失败）；硬错误会以异常形式抛出
      const warnings = await invoke<string[]>("validate_memory", {
        regions: get().regions,
      });
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
      await invoke("export_memory_defconfig", {
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
      await invoke("export_memory_json", {
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
