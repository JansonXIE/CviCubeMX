import { create } from 'zustand';

interface SdkState {
  sdkPath: string | null;
  chipType: string;
  isOnboardingOpen: boolean;
  setSdkPath: (path: string | null) => void;
  setChipType: (chip: string) => void;
  setIsOnboardingOpen: (open: boolean) => void;
}

export const useSdkStore = create<SdkState>((set) => ({
  sdkPath: localStorage.getItem('lastSdkPath') || null,
  chipType: localStorage.getItem('selectedChipType') || 'cv1842hp_wevb_0014a_emmc',
  isOnboardingOpen: false,
  setSdkPath: (path) => {
    if (path) {
      localStorage.setItem('lastSdkPath', path);
    } else {
      localStorage.removeItem('lastSdkPath');
    }
    set({ sdkPath: path });
  },
  setChipType: (chip) => {
    localStorage.setItem('selectedChipType', chip);
    set({ chipType: chip });
  },
  setIsOnboardingOpen: (open) => {
    set({ isOnboardingOpen: open });
  },
}));
