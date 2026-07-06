import { create } from 'zustand';
import { invoke } from '@tauri-apps/api/core';

export interface PinInfo {
  pin_num: string;
  pin_name: string;
  display_name: string;
  supported_functions: string[];
  default_function: string;
  current_function: string;
  user_configured: boolean;
  /** 二级 mux 功能选择 (仅当 current_function 为 MUX_SPI1_* 时有值) */
  current_state?: string | null;
}

export interface ChipSpec {
  chip_type: string;
  package: string;
  pin_count: number;
  rows?: string;
  cols?: number;
  description: string;
}

/** 二级 mux 定义 (来自后端 get_mux_functions) */
export interface MuxDef {
  name: string;
  default: string;
  options: string[];
}

interface ChipState {
  chipType: string | null;
  chipSpec: ChipSpec | null;
  pins: Map<string, PinInfo>;
  /** mux 名称 -> 定义，用于渲染二级功能菜单 */
  muxFunctions: Record<string, MuxDef>;
  searchText: string;
  highlightedPins: Set<string>;
  isLoading: boolean;
  error: string | null;

  // Actions
  selectChip: (type: string) => Promise<void>;
  syncBoardInit: (sdkPath: string) => Promise<void>;
  setPinFunction: (pin: string, fn: string) => Promise<void>;
  setPinSubFunction: (pin: string, subFn: string) => Promise<void>;
  searchPin: (text: string) => void;
}

export const useChatStore = useChatStoreMock; // Placeholder to avoid conflict if any
function useChatStoreMock() {}

/** 从后端加载指定芯片的引脚列表并构建 pin_name -> PinInfo 的 Map */
async function buildPinsMap(chipType: string): Promise<Map<string, PinInfo>> {
  const pinList = await invoke<PinInfo[]>('load_pin_data', { chipType });
  const pinsMap = new Map<string, PinInfo>();
  pinList.forEach((p) => {
    pinsMap.set(p.pin_name, p);
  });
  return pinsMap;
}

export const useChipStore = create<ChipState>((set, get) => ({
  chipType: null,
  chipSpec: null,
  pins: new Map(),
  muxFunctions: {},
  searchText: '',
  highlightedPins: new Set(),
  isLoading: false,
  error: null,

  selectChip: async (type: string) => {
    set({ isLoading: true, error: null });
    try {
      const spec = await invoke<ChipSpec>('load_chip_spec', { chipType: type });
      const pinsMap = await buildPinsMap(type);

      // 加载二级 mux 定义 (与芯片无关，但随选型一并拉取一次)
      let muxFunctions = get().muxFunctions;
      if (Object.keys(muxFunctions).length === 0) {
        try {
          const defs = await invoke<MuxDef[]>('get_mux_functions');
          muxFunctions = Object.fromEntries(defs.map((d) => [d.name, d]));
        } catch {
          muxFunctions = {};
        }
      }

      set({
        chipType: type,
        chipSpec: spec,
        pins: pinsMap,
        muxFunctions,
        highlightedPins: new Set(),
        isLoading: false,
      });
    } catch (err) {
      set({ error: String(err), isLoading: false });
    }
  },

  /**
   * 读取当前 SDK 内已有的 cvi_board_init.c，将其中保存的引脚复用配置
   * 恢复到界面上（避免每次打开都是默认状态）。
   * 需在 selectChip 之后调用；若文件不存在则保持默认状态。
   */
  syncBoardInit: async (sdkPath: string) => {
    const { chipType } = get();
    if (!chipType || !sdkPath) return;
    try {
      // 后端读取并解析文件，同时把配置写入 USER_CONFIG
      await invoke('read_board_init_config', { sdkPath, chipType });
      // 重新加载引脚数据，此时已反映恢复出的复用状态
      const pinsMap = await buildPinsMap(chipType);
      set({ pins: pinsMap });
    } catch (err) {
      set({ error: String(err) });
    }
  },

  setPinFunction: async (pin: string, fn: string) => {
    const { chipType, pins, muxFunctions } = get();
    if (!chipType) return;
    try {
      // 选中二级 mux 时，state 默认回填该 mux 的默认二级功能
      const mux = muxFunctions[fn];
      const state = mux ? mux.default : null;

      await invoke('set_pin_function', {
        chipType,
        pinName: pin,
        function: fn,
        state,
      });

      const newPins = new Map(pins);
      const existing = newPins.get(pin);
      if (existing) {
        newPins.set(pin, {
          ...existing,
          current_function: fn,
          current_state: state,
          user_configured: true,
        });
      }
      set({ pins: newPins });
    } catch (err) {
      set({ error: String(err) });
    }
  },

  /**
   * 设置二级 mux 功能 (仅当引脚 current_function 为 MUX_SPI1_* 时有意义)。
   * 保持一级功能不变，仅更新 state。
   */
  setPinSubFunction: async (pin: string, subFn: string) => {
    const { chipType, pins } = get();
    if (!chipType) return;
    const existing = pins.get(pin);
    if (!existing) return;
    try {
      await invoke('set_pin_function', {
        chipType,
        pinName: pin,
        function: existing.current_function,
        state: subFn,
      });

      const newPins = new Map(pins);
      newPins.set(pin, {
        ...existing,
        current_state: subFn,
        user_configured: true,
      });
      set({ pins: newPins });
    } catch (err) {
      set({ error: String(err) });
    }
  },

  searchPin: (text: string) => {
    const { pins } = get();
    const highlighted = new Set<string>();
    const trimmed = text.trim().toLowerCase();
    
    if (trimmed) {
      pins.forEach((info) => {
        if (
          info.pin_name.toLowerCase().includes(trimmed) ||
          info.pin_num.toLowerCase().includes(trimmed) ||
          info.current_function.toLowerCase().includes(trimmed) ||
          info.supported_functions.some((fn) => fn.toLowerCase().includes(trimmed))
        ) {
          highlighted.add(info.pin_name);
        }
      });
    }
    
    set({ searchText: text, highlightedPins: highlighted });
  },
}));

export type ChipStore = ChipState;
