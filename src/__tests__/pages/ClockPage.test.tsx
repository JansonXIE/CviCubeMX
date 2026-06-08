import React from 'react';
import { describe, it, expect, vi } from 'vitest';
import { render, screen } from '@testing-library/react';
import ClockPage from '../../pages/ClockPage';

// Mock Tauri invoke
vi.mock('@tauri-apps/api/core', () => {
  const mockInvoke = vi.fn((cmd: string, args?: any) => {
    if (cmd === 'compute_clock_tree') {
      return Promise.resolve({
        pllConfigs: {
          clk_fpll: { name: 'clk_fpll', enabled: true, inputFreq: 25.0, outputFreq: 1000.0, divider: 1.0, multiplier: 40, source: 'OSC' },
          clk_mipimpll: { name: 'clk_mipimpll', enabled: true, inputFreq: 25.0, outputFreq: 900.0, divider: 1.0, multiplier: 36, source: 'OSC' },
          clk_mpll: { name: 'clk_mpll', enabled: true, inputFreq: 25.0, outputFreq: 1200.0, divider: 1.0, multiplier: 48, source: 'OSC' },
          clk_tpll: { name: 'clk_tpll', enabled: true, inputFreq: 25.0, outputFreq: 1500.0, divider: 1.0, multiplier: 60, source: 'OSC' },
          clk_appll: { name: 'clk_appll', enabled: true, inputFreq: 25.0, outputFreq: 1000.0, divider: 1.0, multiplier: 40, source: 'OSC' },
          clk_rvpll: { name: 'clk_rvpll', enabled: true, inputFreq: 25.0, outputFreq: 1200.0, divider: 1.0, multiplier: 48, source: 'OSC' },
          clk_a24k: { name: 'clk_a24k', enabled: true, inputFreq: 900.0, outputFreq: 0.0, divider: 1.0, multiplier: 1, source: 'clk_mipimpll' },
          clk_vivo_mipimpll: { name: 'clk_vivo_mipimpll', enabled: true, inputFreq: 900.0, outputFreq: 900.0, divider: 1.0, multiplier: 1, source: 'clk_mipimpll' },
          clk_cyc_dsi_syn: { name: 'clk_cyc_dsi_syn', enabled: true, inputFreq: 900.0, outputFreq: 900.0, divider: 1.0, multiplier: 1, source: 'clk_mipimpll' },
          clk_disppll: { name: 'clk_disppll', enabled: true, inputFreq: 900.0, outputFreq: 900.0, divider: 1.0, multiplier: 1, source: 'clk_mipimpll' },
          clk_a0pll: { name: 'clk_a0pll', enabled: true, inputFreq: 900.0, outputFreq: 900.0, divider: 1.0, multiplier: 1, source: 'clk_mipimpll' },
        },
        outputs: {},
        subNodes: {},
      });
    }
    if (cmd === 'load_module_positions') {
      return Promise.resolve({});
    }
    return Promise.resolve();
  });
  return { invoke: mockInvoke };
});

describe('ClockPage Component Render Test', () => {
  it('should render ClockPage without throwing errors', () => {
    const { container } = render(<ClockPage />);
    expect(container).toBeDefined();
    
    // Ensure header title is rendered
    const titleElement = screen.getByText(/时钟配置/);
    expect(titleElement).toBeDefined();
  });
});
