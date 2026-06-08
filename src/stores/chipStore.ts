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
}

export interface ChipSpec {
  chip_type: string;
  package: string;
  pin_count: number;
  rows?: string;
  cols?: number;
  description: string;
}

interface ChipState {
  chipType: string | null;
  chipSpec: ChipSpec | null;
  pins: Map<string, PinInfo>;
  searchText: string;
  highlightedPins: Set<string>;
  isLoading: boolean;
  error: string | null;

  // Actions
  selectChip: (type: string) => Promise<void>;
  setPinFunction: (pin: string, fn: string) => Promise<void>;
  searchPin: (text: string) => void;
}

export const useChatStore = useChatStoreMock; // Placeholder to avoid conflict if any
function useChatStoreMock() {}

export const useChipStore = create<ChipState>((set, get) => ({
  chipType: null,
  chipSpec: null,
  pins: new Map(),
  searchText: '',
  highlightedPins: new Set(),
  isLoading: false,
  error: null,

  selectChip: async (type: string) => {
    set({ isLoading: true, error: null });
    try {
      const spec = await invoke<ChipSpec>('load_chip_spec', { chipType: type });
      const pinList = await invoke<PinInfo[]>('load_pin_data', { chipType: type });
      
      const pinsMap = new Map<string, PinInfo>();
      pinList.forEach((p) => {
        pinsMap.set(p.pin_name, p);
      });
      
      set({
        chipType: type,
        chipSpec: spec,
        pins: pinsMap,
        highlightedPins: new Set(),
        isLoading: false,
      });
    } catch (err) {
      set({ error: String(err), isLoading: false });
    }
  },

  setPinFunction: async (pin: string, fn: string) => {
    const { chipType, pins } = get();
    if (!chipType) return;
    try {
      await invoke('set_pin_function', {
        chipType,
        pinName: pin,
        function: fn,
        state: null,
      });

      const newPins = new Map(pins);
      const existing = newPins.get(pin);
      if (existing) {
        newPins.set(pin, {
          ...existing,
          current_function: fn,
          user_configured: true,
        });
      }
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
