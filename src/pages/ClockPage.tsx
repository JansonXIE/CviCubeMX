import React, { useEffect, useState, useRef } from "react";
import { useClockStore, PllConfig, ClockOutput, ModulePosition } from "../stores/clockStore";
import { Search, Clock } from "lucide-react";
import { useSdkStore } from "../stores/sdkStore";

// 31 个节点卡片的默认位置 (参考 clockconfig.cpp 中的 initializeModulePositions)
const DEFAULT_MODULE_POSITIONS: Record<string, { x: number; y: number; width: number; height: number }> = {
  "输入源": { x: 30, y: 50, width: 150, height: 400 },
  "锁相环": { x: 250, y: 50, width: 200, height: 700 },
  "clk_fpll子节点": { x: 520, y: 50, width: 180, height: 800 },
  "clk_xtal_misc子节点": { x: 770, y: 50, width: 180, height: 900 },
  "clk_i2c子节点": { x: 1020, y: 50, width: 180, height: 130 },
  "clk_appll子节点": { x: 520, y: 900, width: 180, height: 130 },
  "clk_tpu子节点": { x: 520, y: 1070, width: 180, height: 220 },
  "clk_rvpll子节点": { x: 520, y: 1320, width: 180, height: 130 },
  "子锁相环": { x: 1270, y: 530, width: 200, height: 780 },
  "clk_a0pll子节点": { x: 1530, y: 910, width: 180, height: 480 },
  "clk_cam0pll子节点": { x: 2350, y: 2075, width: 180, height: 130 },
  "clk_cam1pll子节点": { x: 2350, y: 1610, width: 180, height: 450 },
  "clk_disppll子节点": { x: 1530, y: 470, width: 180, height: 290 },
  "clk_raw_axi子节点": { x: 2590, y: 1860, width: 200, height: 450 },
  "clk_sys_disp子节点": { x: 1770, y: 620, width: 200, height: 130 },
  "clk_mpll子节点": { x: 520, y: 1500, width: 200, height: 1900 },
  "clk_rtc_sys子节点": { x: 790, y: 2630, width: 200, height: 1200 },
  "clk_hspi子节点": { x: 1330, y: 2470, width: 200, height: 1700 },
  "clk_vip_sys_0子节点": { x: 1060, y: 2960, width: 200, height: 500 },
  "clk_vip_sys_1子节点": { x: 1060, y: 2440, width: 200, height: 500 },
  "clk_vip_sys_3子节点": { x: 790, y: 2360, width: 200, height: 220 },
  "clk_fab100m子节点": { x: 770, y: 980, width: 200, height: 300 },
  "clk_apb_i2c子节点": { x: 1270, y: 50, width: 200, height: 470 },
  "clk_apb_vcsys子节点": { x: 1040, y: 950, width: 200, height: 220 },
  "clk_x2p子节点": { x: 1040, y: 1180, width: 200, height: 420 },
  "clk_vip_sys_2子节点": { x: 2580, y: 1610, width: 200, height: 220 },
  "clk_keyscan_xclk子节点": { x: 2350, y: 1150, width: 200, height: 130 },
  "clk_wgn_xclk子节点": { x: 2350, y: 1370, width: 200, height: 130 },
  "OSC输出": { x: 2060, y: 9, width: 220, height: 2100 },
  "clk_1M子节点": { x: 1020, y: 230, width: 180, height: 380 },
  "clk_spi子节点": { x: 790, y: 1920, width: 180, height: 400 },
};

// 31 个节点卡片的中文标题映射
const NODE_TITLES: Record<string, string> = {
  "输入源": "输入源 (XTAL)",
  "锁相环": "锁相环 (PLLs)",
  "子锁相环": "子锁相环 (SubPLLs)",
  "OSC输出": "OSC输出",
  "clk_fpll子节点": "clk_fpll 子节点",
  "clk_xtal_misc子节点": "clk_xtal_misc 子节点",
  "clk_i2c子节点": "clk_i2c 子节点",
  "clk_appll子节点": "clk_appll 子节点",
  "clk_tpu子节点": "clk_tpu 子节点",
  "clk_rvpll子节点": "clk_rvpll 子节点",
  "clk_a0pll子节点": "clk_a0pll 子节点",
  "clk_cam0pll子节点": "clk_cam0pll 子节点",
  "clk_cam1pll子节点": "clk_cam1pll 子节点",
  "clk_disppll子节点": "clk_disppll 子节点",
  "clk_raw_axi子节点": "clk_raw_axi 子节点",
  "clk_sys_disp子节点": "clk_sys_disp 子节点",
  "clk_mpll子节点": "clk_mpll 子节点",
  "clk_rtc_sys子节点": "clk_rtc_sys 子节点",
  "clk_hspi子节点": "clk_hspi 子节点",
  "clk_vip_sys_0子节点": "clk_vip_sys_0 子节点",
  "clk_vip_sys_1子节点": "clk_vip_sys_1 子节点",
  "clk_vip_sys_3子节点": "clk_vip_sys_3 子节点",
  "clk_fab100m子节点": "clk_fab_100M 子节点",
  "clk_apb_i2c子节点": "clk_apb_i2c 子节点",
  "clk_apb_vcsys子节点": "clk_apb_vcsys 子节点",
  "clk_x2p子节点": "clk_x2p 子节点",
  "clk_vip_sys_2子节点": "clk_vip_sys_2 子节点",
  "clk_keyscan_xclk子节点": "clk_keyscan_xclk 子节点",
  "clk_wgn_xclk子节点": "clk_wgn_xclk 子节点",
  "clk_1M子节点": "clk_1M 子节点",
  "clk_spi子节点": "clk_spi 子节点",
};

// 31 个节点卡片的配色体系
const NODE_COLORS: Record<string, { border: string; bg: string; text: string; titleBg: string }> = {
  "输入源": { border: "border-emerald-500/35", bg: "bg-emerald-950/20", text: "text-emerald-400", titleBg: "bg-emerald-500/10" },
  "锁相环": { border: "border-amber-500/35", bg: "bg-amber-950/20", text: "text-amber-400", titleBg: "bg-amber-500/10" },
  "子锁相环": { border: "border-cyan-500/35", bg: "bg-cyan-950/20", text: "text-cyan-400", titleBg: "bg-cyan-500/10" },
  "OSC输出": { border: "border-rose-500/35", bg: "bg-rose-950/20", text: "text-rose-400", titleBg: "bg-rose-500/10" },

  // FPLL 分支
  "clk_fpll子节点": { border: "border-rose-500/20 hover:border-rose-500/35", bg: "bg-slate-900/60", text: "text-rose-400", titleBg: "bg-rose-500/5" },
  "clk_xtal_misc子节点": { border: "border-rose-500/20 hover:border-rose-500/35", bg: "bg-slate-900/60", text: "text-rose-400", titleBg: "bg-rose-500/5" },
  "clk_i2c子节点": { border: "border-rose-500/20 hover:border-rose-500/35", bg: "bg-slate-900/60", text: "text-rose-400", titleBg: "bg-rose-500/5" },
  "clk_apb_i2c子节点": { border: "border-rose-500/20 hover:border-rose-500/35", bg: "bg-slate-900/60", text: "text-rose-400", titleBg: "bg-rose-500/5" },
  "clk_fab100m子节点": { border: "border-rose-500/20 hover:border-rose-500/35", bg: "bg-slate-900/60", text: "text-rose-400", titleBg: "bg-rose-500/5" },
  "clk_apb_vcsys子节点": { border: "border-rose-500/20 hover:border-rose-500/35", bg: "bg-slate-900/60", text: "text-rose-400", titleBg: "bg-rose-500/5" },
  "clk_x2p子节点": { border: "border-rose-500/20 hover:border-rose-500/35", bg: "bg-slate-900/60", text: "text-rose-400", titleBg: "bg-rose-500/5" },
  "clk_1M子节点": { border: "border-rose-500/20 hover:border-rose-500/35", bg: "bg-slate-900/60", text: "text-rose-400", titleBg: "bg-rose-500/5" },

  // MPLL 分支
  "clk_mpll子节点": { border: "border-emerald-500/20 hover:border-emerald-500/35", bg: "bg-slate-900/60", text: "text-emerald-400", titleBg: "bg-emerald-500/5" },
  "clk_rtc_sys子节点": { border: "border-emerald-500/20 hover:border-emerald-500/35", bg: "bg-slate-900/60", text: "text-emerald-400", titleBg: "bg-emerald-500/5" },
  "clk_hspi子节点": { border: "border-emerald-500/20 hover:border-emerald-500/35", bg: "bg-slate-900/60", text: "text-emerald-400", titleBg: "bg-emerald-500/5" },
  "clk_vip_sys_0子节点": { border: "border-emerald-500/20 hover:border-emerald-500/35", bg: "bg-slate-900/60", text: "text-emerald-400", titleBg: "bg-emerald-500/5" },
  "clk_vip_sys_1子节点": { border: "border-emerald-500/20 hover:border-emerald-500/35", bg: "bg-slate-900/60", text: "text-emerald-400", titleBg: "bg-emerald-500/5" },
  "clk_vip_sys_3子节点": { border: "border-emerald-500/20 hover:border-emerald-500/35", bg: "bg-slate-900/60", text: "text-emerald-400", titleBg: "bg-emerald-500/5" },
  "clk_spi子节点": { border: "border-emerald-500/20 hover:border-emerald-500/35", bg: "bg-slate-900/60", text: "text-emerald-400", titleBg: "bg-emerald-500/5" },

  // TPLL 分支
  "clk_tpu子节点": { border: "border-cyan-500/20 hover:border-cyan-500/35", bg: "bg-slate-900/60", text: "text-cyan-400", titleBg: "bg-cyan-500/5" },

  // APPLL 分支
  "clk_appll子节点": { border: "border-purple-500/20 hover:border-purple-500/35", bg: "bg-slate-900/60", text: "text-purple-400", titleBg: "bg-purple-500/5" },

  // RVPLL 分支
  "clk_rvpll子节点": { border: "border-orange-500/20 hover:border-orange-500/35", bg: "bg-slate-900/60", text: "text-orange-400", titleBg: "bg-orange-500/5" },

  // DispPLL 分支
  "clk_disppll子节点": { border: "border-pink-500/20 hover:border-pink-500/35", bg: "bg-slate-900/60", text: "text-pink-400", titleBg: "bg-pink-500/5" },
  "clk_sys_disp子节点": { border: "border-pink-500/20 hover:border-pink-500/35", bg: "bg-slate-900/60", text: "text-pink-400", titleBg: "bg-pink-500/5" },

  // Cam0/1 分支
  "clk_cam0pll子节点": { border: "border-indigo-500/20 hover:border-indigo-500/35", bg: "bg-slate-900/60", text: "text-indigo-400", titleBg: "bg-indigo-500/5" },
  "clk_cam1pll子节点": { border: "border-indigo-500/20 hover:border-indigo-500/35", bg: "bg-slate-900/60", text: "text-indigo-400", titleBg: "bg-indigo-500/5" },
  "clk_raw_axi子节点": { border: "border-indigo-500/20 hover:border-indigo-500/35", bg: "bg-slate-900/60", text: "text-indigo-400", titleBg: "bg-indigo-500/5" },
  "clk_vip_sys_2子节点": { border: "border-indigo-500/20 hover:border-indigo-500/35", bg: "bg-slate-900/60", text: "text-indigo-400", titleBg: "bg-indigo-500/5" },
  "clk_keyscan_xclk子节点": { border: "border-indigo-500/20 hover:border-indigo-500/35", bg: "bg-slate-900/60", text: "text-indigo-400", titleBg: "bg-indigo-500/5" },
  "clk_wgn_xclk子节点": { border: "border-indigo-500/20 hover:border-indigo-500/35", bg: "bg-slate-900/60", text: "text-indigo-400", titleBg: "bg-indigo-500/5" },
};

// 子节点卡片名字到父时钟名称的映射
const NODE_NAME_TO_PARENT_CLOCK: Record<string, string> = {
  "clk_fpll子节点": "clk_fpll",
  "clk_xtal_misc子节点": "clk_xtal_misc",
  "clk_i2c子节点": "clk_i2c",
  "clk_apb_i2c子节点": "clk_apb_i2c",
  "clk_fab100m子节点": "clk_fab_100M",
  "clk_apb_vcsys子节点": "clk_apb_vcsys",
  "clk_x2p子节点": "clk_x2p",
  "clk_1M子节点": "clk_1M",
  "clk_mpll子节点": "clk_mpll",
  "clk_rtc_sys子节点": "clk_rtc_sys",
  "clk_hspi子节点": "clk_hsperi",
  "clk_vip_sys_0子节点": "clk_vip_sys_0",
  "clk_vip_sys_1子节点": "clk_vip_sys_1",
  "clk_vip_sys_3子节点": "clk_vip_sys_3",
  "clk_spi子节点": "clk_spi",
  "clk_tpu子节点": "clk_tpll",
  "clk_appll子节点": "clk_appll",
  "clk_rvpll子节点": "clk_rvpll",
  "clk_disppll子节点": "clk_disppll",
  "clk_sys_disp子节点": "clk_sys_disp",
  "clk_cam0pll子节点": "clk_cam0pll",
  "clk_cam1pll子节点": "clk_cam1pll",
  "clk_raw_axi子节点": "clk_raw_axi",
  "clk_vip_sys_2子节点": "clk_vip_sys_2",
  "clk_keyscan_xclk子节点": "clk_keyscan_xclk",
  "clk_wgn_xclk子节点": "clk_wgn_xclk",
  "clk_a0pll子节点": "clk_a0pll",
};

// 静态定义的子时钟项列表数组 (匹配 clock_calc.rs 排序)
const CLK_1M_SUB_NODES = ["clk_gpio_dbclk", "clk_emmc_100K", "clk_100k_sd1", "clk_100k_sd0"];
const CLK_CAM1PLL_SUB_NODES = ["clk_emmc_card", "clk_sd1", "clk_sd0", "clk_vip_sys_2", "clk_raw_axi", "clk_vc_src1"];
const CLK_RAW_AXI_SUB_NODES = ["clk_oenc", "clk_lvds1_vip", "clk_lvds0_vip", "clk_raw_vip", "clk_disp_vip"];
const CLK_CAM0PLL_SUB_NODES = ["clk_cam0_vip"];
const CLK_DISPPLL_SUB_NODES = ["clk_cam2_vip", "clk_cam1_vip", "clk_sys_disp"];
const CLK_SYS_DISP_SUB_NODES = ["clk_vo_mac_vip"];
const CLK_A0PLL_SUB_NODES = ["clk_aud3", "clk_aud2", "clk_aud1", "clk_aud0", "clk_audsrc"];
const CLK_RVPLL_SUB_NODES = ["clk_rv1"];
const CLK_APPLL_SUB_NODES = ["clk_cpu"];
const CLK_FPLL_SUB_NODES = ["clk_xtal_misc", "clk_pwm", "clk_i2c", "clk_eth_pll", "clk_cyc_dsi_esc", "clk_scan_100M", "clk_video_axi", "clk_fab_500M", "clk_fab_100M"];
const CLK_TPLL_SUB_NODES = ["clk_tpu", "clk_tpu_gdma"];
const CLK_MPLL_SUB_NODES = [
  "clk_uart0", "clk_uart4", "clk_uart3", "clk_uart2", "clk_uart1", "clk_spi", "clk_spi_nand", "clk_spi_nor",
  "clk_usb20_ref", "clk_usb20_bus_early", "clk_rtc_spi_nor", "clk_cyc_scan_300M", "clk_vip_sys_4", "clk_vip_sys_3",
  "clk_vip_sys_1", "clk_vip_sys_0", "clk_vc_src0", "clk_tpu_sys", "clk_gic", "clk_bus", "clk_rtc_sys", "clk_hsperi"
];
const CLK_FAB_100M_SUB_NODES = ["clk_apb_gpio", "clk_apb_wdt", "clk_apb_vcsys", "clk_apb_jpeg", "clk_apb_ve", "clk_fab6_100M_free", "clk_efuse_pclk", "clk_x2p"];
const CLK_XTAL_MISC_SUB_NODES = ["clk_timer7", "clk_timer6", "clk_timer5", "clk_timer4", "clk_timer3", "clk_timer2", "clk_timer1", "clk_timer0", "clk_1M", "clk_usb20_suspend"];
const CLK_I2C_SUB_NODES = ["clk_apb_i2c"];
const CLK_APB_I2C_SUB_NODES = ["clk_apb_i2c4", "clk_apb_i2c3", "clk_apb_i2c2", "clk_apb_i2c1", "clk_apb_i2c0"];
const CLK_APB_VCSYS_SUB_NODES = ["clk_apb_jpeg", "clk_apb_ve"];
const CLK_X2P_SUB_NODES = ["clk_2de_vip", "clk_csi2_rx_vip", "clk_csi1_rx_vip", "clk_csi0_rx_vip", "clk_dsi_mac_vip"];
const CLK_RTC_SYS_SUB_NODES = [
  "clk_rtc_sys_apb_saradc1", "clk_rtc_sys_apb_wdt", "clk_rtc_sys_apb_saradc", "clk_rtc_sys_apb_i2c", "clk_rtc_sys_apb_osc",
  "clk_rtc_sys_apb_gpio", "clk_rtc_sys_apb_mbox", "clk_rtc_sys_apb_ictrl", "clk_rtc_sys_apb_uart", "clk_rtc_sys_apb_timer",
  "clk_rtc_sys_fab_sram", "clk_rtc_sys_rtc2ap_slv", "clk_rtc_sys_hs2rtc_mst", "clk_rtc_sys_mcu"
];
const CLK_HSPERI_SUB_NODES = [
  "clk_apb_usb", "clk_axi4_usb", "clk_apb_i2s3", "clk_apb_i2s2", "clk_apb_i2s1", "clk_apb_i2s0",
  "clk_apb_uart4", "clk_apb_uart3", "clk_apb_uart2", "clk_apb_uart1", "clk_apb_uart0", "clk_sdma1_axi",
  "clk_sdma0_axi", "clk_ahb_sf", "clk_axi4_eth0", "clk_spi_nand_gate", "clk_axi4_sd1", "clk_axi4_sd0",
  "clk_axi4_emmc", "clk_apb_audsrc"
];
const CLK_VIP_SYS_0_SUB_NODES = ["clk_pad_vi2_clk_vip", "clk_pad_vi1_clk_vip", "clk_pad_vi0_clk1_vip", "clk_pad_vi0_clk0_vip", "clk_csi_mac2_vip"];
const CLK_VIP_SYS_1_SUB_NODES = ["clk_vpss3_vip", "clk_vpss2_vip", "clk_vpss1_vip", "clk_vpss0_vip", "clk_isp_top_vip"];
const CLK_VIP_SYS_2_SUB_NODES = ["clk_ldc_vip", "clk_csi_mac1_vip"];
const CLK_VIP_SYS_3_SUB_NODES = ["clk_csi_be_vip", "clk_csi_mac0_vip"];
const CLK_SPI_SUB_NODES = ["clk_apb_spi3", "clk_apb_spi2", "clk_apb_spi1", "clk_apb_spi0"];
const CLK_KEYSCAN_XCLK_SUB_NODES = ["clk_keyscan"];
const CLK_WGN_XCLK_SUB_NODES = ["clk_wgn"];

const SUB_NODE_LISTS: Record<string, string[]> = {
  "clk_1M": CLK_1M_SUB_NODES,
  "clk_cam1pll": CLK_CAM1PLL_SUB_NODES,
  "clk_raw_axi": CLK_RAW_AXI_SUB_NODES,
  "clk_cam0pll": CLK_CAM0PLL_SUB_NODES,
  "clk_disppll": CLK_DISPPLL_SUB_NODES,
  "clk_sys_disp": CLK_SYS_DISP_SUB_NODES,
  "clk_a0pll": CLK_A0PLL_SUB_NODES,
  "clk_rvpll": CLK_RVPLL_SUB_NODES,
  "clk_appll": CLK_APPLL_SUB_NODES,
  "clk_fpll": CLK_FPLL_SUB_NODES,
  "clk_tpll": CLK_TPLL_SUB_NODES,
  "clk_mpll": CLK_MPLL_SUB_NODES,
  "clk_fab_100M": CLK_FAB_100M_SUB_NODES,
  "clk_xtal_misc": CLK_XTAL_MISC_SUB_NODES,
  "clk_i2c": CLK_I2C_SUB_NODES,
  "clk_apb_i2c": CLK_APB_I2C_SUB_NODES,
  "clk_apb_vcsys": CLK_APB_VCSYS_SUB_NODES,
  "clk_x2p": CLK_X2P_SUB_NODES,
  "clk_rtc_sys": CLK_RTC_SYS_SUB_NODES,
  "clk_hsperi": CLK_HSPERI_SUB_NODES,
  "clk_vip_sys_0": CLK_VIP_SYS_0_SUB_NODES,
  "clk_vip_sys_1": CLK_VIP_SYS_1_SUB_NODES,
  "clk_vip_sys_2": CLK_VIP_SYS_2_SUB_NODES,
  "clk_vip_sys_3": CLK_VIP_SYS_3_SUB_NODES,
  "clk_spi": CLK_SPI_SUB_NODES,
  "clk_keyscan_xclk": CLK_KEYSCAN_XCLK_SUB_NODES,
  "clk_wgn_xclk": CLK_WGN_XCLK_SUB_NODES,
};

// OSC 直接输出名字
const OUTPUT_NAMES = [
  "clk_rtc_sys_saradc1", "clk_rtc_sys_irrx", "clk_rtc_sys_saradc", "clk_rtc_sys_i2c", "clk_rtc_sys_uart",
  "clk_rtc_sys_timer1", "clk_rtc_sys_timer0", "clk_rtc_sys_rtc_spinor", "clk_rtc_sys_spinor1", "clk_pm",
  "clk_saradc", "clk_tempsen", "clk_ahb_sf1", "clk_dbgsys", "clk_efuse_clk", "clk_keyscan_xclk", "clk_wgn_xclk",
  "clk_wdt_pclk", "clk_mipimpll_d3", "clk_cam1pll", "clk_cam0pll"
];

// 时钟节点级联连线定义
const CONNECTIONS = [
  // 输入源 (OSC) -> 6 个主 PLL
  { fromNode: "输入源", fromItem: "OSC", toNode: "锁相环", toItem: "clk_fpll", color: "#dc3545" },
  { fromNode: "输入源", fromItem: "OSC", toNode: "锁相环", toItem: "clk_mipimpll", color: "#ffc107" },
  { fromNode: "输入源", fromItem: "OSC", toNode: "锁相环", toItem: "clk_mpll", color: "#28a745" },
  { fromNode: "输入源", fromItem: "OSC", toNode: "锁相环", toItem: "clk_tpll", color: "#17a2b8" },
  { fromNode: "输入源", fromItem: "OSC", toNode: "锁相环", toItem: "clk_appll", color: "#6610f2" },
  { fromNode: "输入源", fromItem: "OSC", toNode: "锁相环", toItem: "clk_rvpll", color: "#ff6600" },

  // MIPIMPLL -> 子锁相环
  { fromNode: "锁相环", fromItem: "clk_mipimpll", toNode: "子锁相环", toItem: "", color: "#ffa500" },

  // OSC -> OSC输出
  { fromNode: "输入源", fromItem: "OSC", toNode: "OSC输出", toItem: "", color: "#28a745" },

  // clk_1M -> clk_1M子节点
  { fromNode: "clk_xtal_misc子节点", fromItem: "clk_1M", toNode: "clk_1M子节点", toItem: "", color: "#6610f2" },

  // clk_cam1pll -> clk_cam1pll子节点
  { fromNode: "OSC输出", fromItem: "clk_cam1pll", toNode: "clk_cam1pll子节点", toItem: "", color: "#ffa500" },

  // clk_raw_axi -> clk_raw_axi子节点
  { fromNode: "clk_cam1pll子节点", fromItem: "clk_raw_axi", toNode: "clk_raw_axi子节点", toItem: "", color: "#228b22" },

  // clk_cam0pll -> clk_cam0pll子节点
  { fromNode: "OSC输出", fromItem: "clk_cam0pll", toNode: "clk_cam0pll子节点", toItem: "", color: "#191970" },

  // clk_disppll -> clk_disppll子节点
  { fromNode: "子锁相环", fromItem: "clk_disppll", toNode: "clk_disppll子节点", toItem: "", color: "#4b0082" },

  // clk_sys_disp -> clk_sys_disp子节点
  { fromNode: "clk_disppll子节点", fromItem: "clk_sys_disp", toNode: "clk_sys_disp子节点", toItem: "", color: "#009688" },

  // clk_a0pll -> clk_a0pll子节点
  { fromNode: "子锁相环", fromItem: "clk_a0pll", toNode: "clk_a0pll子节点", toItem: "", color: "#8b0000" },

  // clk_rvpll -> clk_rvpll子节点
  { fromNode: "锁相环", fromItem: "clk_rvpll", toNode: "clk_rvpll子节点", toItem: "", color: "#ff8c00" },

  // clk_appll -> clk_appll子节点
  { fromNode: "锁相环", fromItem: "clk_appll", toNode: "clk_appll子节点", toItem: "", color: "#800080" },

  // clk_fpll -> clk_fpll子节点
  { fromNode: "锁相环", fromItem: "clk_fpll", toNode: "clk_fpll子节点", toItem: "", color: "#00008b" },

  // clk_tpll -> clk_tpu子节点
  { fromNode: "锁相环", fromItem: "clk_tpll", toNode: "clk_tpu子节点", toItem: "", color: "#008b8b" },

  // clk_mpll -> clk_mpll子节点
  { fromNode: "锁相环", fromItem: "clk_mpll", toNode: "clk_mpll子节点", toItem: "", color: "#008000" },

  // clk_fab_100M -> clk_fab100m子节点
  { fromNode: "clk_fpll子节点", fromItem: "clk_fab_100M", toNode: "clk_fab100m子节点", toItem: "", color: "#ff1493" },

  // clk_xtal_misc -> clk_xtal_misc子节点
  { fromNode: "clk_fpll子节点", fromItem: "clk_xtal_misc", toNode: "clk_xtal_misc子节点", toItem: "", color: "#ff8c00" },

  // clk_i2c -> clk_i2c子节点
  { fromNode: "clk_fpll子节点", fromItem: "clk_i2c", toNode: "clk_i2c子节点", toItem: "", color: "#ff8c00" },

  // clk_apb_i2c -> clk_apb_i2c子节点
  { fromNode: "clk_i2c子节点", fromItem: "clk_apb_i2c", toNode: "clk_apb_i2c子节点", toItem: "", color: "#007bff" },

  // clk_apb_vcsys -> clk_apb_vcsys子节点
  { fromNode: "clk_fab100m子节点", fromItem: "clk_apb_vcsys", toNode: "clk_apb_vcsys子节点", toItem: "", color: "#007bff" },

  // clk_x2p -> clk_x2p子节点
  { fromNode: "clk_fab100m子节点", fromItem: "clk_x2p", toNode: "clk_x2p子节点", toItem: "", color: "#007bff" },

  // clk_hsperi -> clk_hspi子节点
  { fromNode: "clk_mpll子节点", fromItem: "clk_hsperi", toNode: "clk_hspi子节点", toItem: "", color: "#006400" },

  // clk_rtc_sys -> clk_rtc_sys子节点
  { fromNode: "clk_mpll子节点", fromItem: "clk_rtc_sys", toNode: "clk_rtc_sys子节点", toItem: "", color: "#ff8c00" },

  // clk_vip_sys_0 -> clk_vip_sys_0子节点
  { fromNode: "clk_mpll子节点", fromItem: "clk_vip_sys_0", toNode: "clk_vip_sys_0子节点", toItem: "", color: "#ff8c00" },

  // clk_vip_sys_1 -> clk_vip_sys_1子节点
  { fromNode: "clk_mpll子节点", fromItem: "clk_vip_sys_1", toNode: "clk_vip_sys_1子节点", toItem: "", color: "#ff8c00" },

  // clk_vip_sys_2 -> clk_vip_sys_2子节点
  { fromNode: "clk_cam1pll子节点", fromItem: "clk_vip_sys_2", toNode: "clk_vip_sys_2子节点", toItem: "", color: "#33cc33" },

  // clk_vip_sys_3 -> clk_vip_sys_3子节点
  { fromNode: "clk_mpll子节点", fromItem: "clk_vip_sys_3", toNode: "clk_vip_sys_3子节点", toItem: "", color: "#ff8c00" },

  // clk_spi -> clk_spi子节点
  { fromNode: "clk_mpll", fromItem: "clk_spi", toNode: "clk_spi子节点", toItem: "", color: "#ff8c00" },

  // clk_keyscan_xclk -> clk_keyscan_xclk子节点
  { fromNode: "OSC输出", fromItem: "clk_keyscan_xclk", toNode: "clk_keyscan_xclk子节点", toItem: "", color: "#cc0000" },

  // clk_wgn_xclk -> clk_wgn_xclk子节点
  { fromNode: "OSC输出", fromItem: "clk_wgn_xclk", toNode: "clk_wgn_xclk子节点", toItem: "", color: "#0066cc" },
];

export default function ClockPage() {
  const { sdkPath, chipType } = useSdkStore();
  const {
    pllConfigs,
    outputs,
    subNodes,
    modulePositions,
    searchText,
    isLoading,
    error,
    computeClockTree,
    loadModulePositions,
    saveModulePositions,
    searchClock,
    exportClockDefconfig,
  } = useClockStore();

  const [localPlls, setLocalPlls] = useState<Record<string, PllConfig>>({});
  const [localSearch, setLocalSearch] = useState("");
  const [exportMsg, setExportMsg] = useState<string | null>(null);

  // 1. 使用 useRef 存储拖拽中的精准瞬态坐标，消除因为 state 改变导致 useEffect 频繁注册/注销的问题
  const dragRef = useRef<{
    nodeName: string;
    startX: number;
    startY: number;
    startMouseX: number;
    startMouseY: number;
  } | null>(null);

  // 2. 用一个状态管理当前的卡片拖拽位置（松手时更新），并以此渲染 UI 和连线
  const [dragPositions, setDragPositions] = useState<Record<string, { x: number; y: number; width: number; height: number }>>({});
  
  // 3. 记录当前正在拖拽的卡片名字，用以在 UI 上提高 z-index 和缩放特效
  const [activeDragName, setActiveDragName] = useState<string | null>(null);

  // 初始化时钟树和坐标
  useEffect(() => {
    loadModulePositions();
    computeClockTree({});
  }, []);

  // 当 store 里的 pllConfigs 变化时同步到本地状态以编辑
  useEffect(() => {
    if (Object.keys(pllConfigs).length > 0) {
      setLocalPlls(JSON.parse(JSON.stringify(pllConfigs)));
    }
  }, [pllConfigs]);

  const handlePllChange = async (name: string, field: keyof PllConfig, value: any) => {
    const updated = {
      ...localPlls,
      [name]: {
        ...localPlls[name],
        [field]: value,
      },
    };
    setLocalPlls(updated);
    // 触发联动重算
    await computeClockTree(updated);
  };

  const handleSearchChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const val = e.target.value;
    setLocalSearch(val);
    searchClock(val);
  };

  const handleExport = async () => {
    if (!sdkPath || !chipType) {
      setExportMsg("请先在顶部配置全局 SDK 源码路径和芯片型号！");
      setTimeout(() => setExportMsg(null), 4000);
      return;
    }
    setExportMsg(null);
    try {
      await exportClockDefconfig(sdkPath, chipType, localPlls);
      setExportMsg(`时钟配置已成功保存并同步到: build/boards/cv184x/${chipType}/${chipType}_defconfig`);
      setTimeout(() => setExportMsg(null), 4000);
    } catch (e) {
      setExportMsg(`时钟配置导出失败: ${e}`);
      setTimeout(() => setExportMsg(null), 5000);
    }
  };

  // 过滤显示卡片高亮匹配
  const filterNodes = (name: string) => {
    if (!searchText) return true;
    return name.toLowerCase().includes(searchText.toLowerCase());
  };

  // 检查卡片内部是否有匹配项
  const checkItemsMatch = (nodeName: string, text: string) => {
    if (!text) return true;
    const t = text.toLowerCase();

    if (nodeName.toLowerCase().includes(t)) return true;

    if (nodeName === "输入源") {
      return "32768hz".includes(t) || "rtc".includes(t) || "25mhz".includes(t) || "osc".includes(t);
    }
    if (nodeName === "锁相环") {
      return ["clk_fpll", "clk_mipimpll", "clk_mpll", "clk_tpll", "clk_appll", "clk_rvpll"].some(name => name.toLowerCase().includes(t));
    }
    if (nodeName === "子锁相环") {
      return ["clk_a24k", "clk_vivo_mipimpll", "clk_cyc_dsi_syn", "clk_disppll", "clk_a0pll"].some(name => name.toLowerCase().includes(t));
    }
    if (nodeName === "OSC输出") {
      return OUTPUT_NAMES.some(name => name.toLowerCase().includes(t));
    }

    // 普通子节点
    const parentClock = NODE_NAME_TO_PARENT_CLOCK[nodeName];
    if (!parentClock) return false;
    const items = SUB_NODE_LISTS[parentClock] || [];
    return items.some(name => name.toLowerCase().includes(t));
  };

  // 合并后台加载的坐标与默认坐标
  const mergedPositions = { ...DEFAULT_MODULE_POSITIONS, ...modulePositions };

  // 鼠标按在标题栏，初始化拖拽
  const handleMouseDown = (e: React.MouseEvent, nodeName: string) => {
    if (e.button !== 0) return; // 仅限左键
    e.preventDefault();

    const currentPos = dragPositions[nodeName] || mergedPositions[nodeName];
    if (!currentPos) return;

    dragRef.current = {
      nodeName,
      startX: currentPos.x,
      startY: currentPos.y,
      startMouseX: e.clientX,
      startMouseY: e.clientY,
    };
    setActiveDragName(nodeName);
  };

  // 4. 组件生命周期中仅在 Mount 时单次注册全局鼠标监听器，绝不因为 state 的渲染改变而注销，确保流畅不丢失事件
  useEffect(() => {
    const handleMouseMove = (e: MouseEvent) => {
      const active = dragRef.current;
      if (!active) return;

      const dx = e.clientX - active.startMouseX;
      const dy = e.clientY - active.startMouseY;

      // 实时获取默认坐标属性以确保有 width 和 height 基础限制
      const pos = DEFAULT_MODULE_POSITIONS[active.nodeName];
      if (!pos) return;

      // 限制坐标不要飞出 2850 * 4300 画布范围外
      const newX = Math.max(0, Math.min(2850 - pos.width, active.startX + dx));
      const newY = Math.max(0, Math.min(4300 - pos.height, active.startY + dy));

      setDragPositions(prev => ({
        ...prev,
        [active.nodeName]: {
          x: newX,
          y: newY,
          width: pos.width,
          height: pos.height,
        }
      }));
    };

    const handleMouseUp = async () => {
      const active = dragRef.current;
      if (!active) return;

      dragRef.current = null;
      setActiveDragName(null);

      // 通过 setState 回调读取最新 dragPositions，避开闭包过时问题，并安全自动持久化存盘
      setDragPositions(latest => {
        const finalPos = latest[active.nodeName];
        if (finalPos) {
          const newPositions = {
            ...DEFAULT_MODULE_POSITIONS,
            ...useClockStore.getState().modulePositions,
            ...latest,
            [active.nodeName]: finalPos,
          };

          const positionsToSave: Record<string, ModulePosition> = {};
          Object.entries(newPositions).forEach(([name, p]) => {
            positionsToSave[name] = {
              moduleName: name,
              x: Math.round(p.x),
              y: Math.round(p.y),
              width: Math.round(p.width),
              height: Math.round(p.height),
            };
          });

          // 存盘至后端 JSON 文件
          useClockStore.getState().saveModulePositions(positionsToSave)
            .catch(err => console.error("时钟布局坐标自动持久化失败:", err));
        }
        return latest;
      });
    };

    window.addEventListener("mousemove", handleMouseMove);
    window.addEventListener("mouseup", handleMouseUp);

    return () => {
      window.removeEventListener("mousemove", handleMouseMove);
      window.removeEventListener("mouseup", handleMouseUp);
    };
  }, []);

  // 根据卡片绝对定位坐标，计算时钟项的精确连线点
  const getCoordinate = (nodeName: string, itemName: string, isOutput: boolean) => {
    const pos = dragPositions[nodeName] || mergedPositions[nodeName];
    if (!pos) return null;

    if (!isOutput) {
      // 终点一律位于卡片左侧垂直居中
      return { x: pos.x, y: pos.y + pos.height / 2 };
    }

    // 起点位于右侧相应时钟项处
    const startX = pos.x + pos.width;

    if (nodeName === "输入源") {
      // OSC 是下半部分输入源
      return { x: startX, y: pos.y + pos.height * 0.72 };
    }

    if (nodeName === "锁相环") {
      const pllList = ["clk_fpll", "clk_mipimpll", "clk_mpll", "clk_tpll", "clk_appll", "clk_rvpll"];
      const i = pllList.indexOf(itemName);
      if (i !== -1) {
        return { x: startX, y: pos.y + 40 + (i + 0.5) * (pos.height - 40) / 6 };
      }
    }

    if (nodeName === "子锁相环") {
      const subPllList = ["clk_a24k", "clk_vivo_mipimpll", "clk_cyc_dsi_syn", "clk_disppll", "clk_a0pll"];
      const i = subPllList.indexOf(itemName);
      if (i !== -1) {
        return { x: startX, y: pos.y + 40 + (i + 0.5) * (pos.height - 40) / 5 };
      }
    }

    if (nodeName === "OSC输出") {
      const i = OUTPUT_NAMES.indexOf(itemName);
      if (i !== -1) {
        return { x: startX, y: pos.y + 40 + (i + 0.5) * (pos.height - 40) / 21 };
      }
    }

    // 普通子节点
    const parentClock = NODE_NAME_TO_PARENT_CLOCK[nodeName];
    if (parentClock) {
      const items = SUB_NODE_LISTS[parentClock] || [];
      const i = items.indexOf(itemName);
      if (i !== -1) {
        return { x: startX, y: pos.y + 40 + (i + 0.5) * (pos.height - 40) / items.length };
      }
    }

    // 默认回退到卡片右侧居中
    return { x: startX, y: pos.y + pos.height / 2 };
  };

  // 渲染卡片的内容
  const renderCardContent = (nodeName: string) => {
    if (nodeName === "输入源") {
      return (
        <div className="space-y-4 p-4 font-sans text-xs">
          <div className="flex flex-col gap-1.5 p-3.5 bg-slate-950/40 rounded-xl border border-slate-800">
            <div className="flex justify-between items-center">
              <span className="font-mono font-bold text-slate-200">32768Hz</span>
              <span className="font-mono text-emerald-400 font-bold">0.033 MHz</span>
            </div>
            <span className="text-[10px] text-slate-500">RTC 实时低速时钟源</span>
          </div>
          <div className="flex flex-col gap-1.5 p-3.5 bg-slate-950/40 rounded-xl border border-slate-800">
            <div className="flex justify-between items-center">
              <span className="font-mono font-bold text-slate-200">25MHz</span>
              <span className="font-mono text-emerald-400 font-bold">25.000 MHz</span>
            </div>
            <span className="text-[10px] text-slate-500">OSC 外部高速晶振</span>
          </div>
        </div>
      );
    }

    if (nodeName === "锁相环") {
      const pllList = ["clk_fpll", "clk_mipimpll", "clk_mpll", "clk_tpll", "clk_appll", "clk_rvpll"];
      return (
        <div className="p-3 space-y-3 font-mono text-xs flex flex-col justify-around h-[calc(100%-40px)]">
          {pllList.map((pllName) => {
            const pll = localPlls[pllName];
            if (!pll) return null;
            return (
              <div key={pllName} className="p-3 bg-slate-950/40 rounded-xl border border-slate-800 space-y-2 relative group hover:border-amber-500/20 transition-all">
                <div className="flex justify-between items-center">
                  <span className="font-bold text-slate-200 text-xs">{pll.name}</span>
                  <span className="font-bold text-amber-400">{(pll.outputFreq || 0).toFixed(3)} MHz</span>
                </div>
                <div className="flex items-center justify-between gap-2 text-[10px] text-slate-400">
                  <span>倍频 multiplier</span>
                  <input
                    type="number"
                    value={pll.multiplier || 0}
                    onChange={(e) => handlePllChange(pllName, "multiplier", Number(e.target.value))}
                    className="bg-slate-900 border border-slate-700/80 rounded px-1.5 py-0.5 text-slate-200 w-14 text-center font-mono focus:outline-none focus:ring-1 focus:ring-amber-500 transition-all"
                  />
                </div>
                <div className="text-[9px] text-slate-600 flex justify-between leading-none">
                  <span>输入: {pll.inputFreq.toFixed(1)} MHz</span>
                  <span>分频: {pll.divider}</span>
                </div>
              </div>
            );
          })}
        </div>
      );
    }

    if (nodeName === "子锁相环") {
      const subPllList = ["clk_a24k", "clk_vivo_mipimpll", "clk_cyc_dsi_syn", "clk_disppll", "clk_a0pll"];
      const mipimpllOutputFreq = localPlls["clk_mipimpll"]?.outputFreq || 25.0;
      return (
        <div className="p-3 space-y-3 font-mono text-xs flex flex-col justify-around h-[calc(100%-40px)]">
          {subPllList.map((pllName) => {
            const pll = localPlls[pllName];
            if (!pll) return null;
            return (
              <div key={pllName} className="p-3 bg-slate-950/40 rounded-xl border border-slate-800 space-y-2 hover:border-cyan-500/20 transition-all">
                <div className="flex justify-between items-center">
                  <span className="font-bold text-slate-200 text-xs">{pll.name}</span>
                  <span className="font-bold text-cyan-400">{(pll.outputFreq || 0).toFixed(3)} MHz</span>
                </div>
                <div className="flex items-center justify-between gap-2 text-[10px] text-slate-400">
                  <span>分频 divider</span>
                  <input
                    type="number"
                    step="0.0001"
                    value={pll.divider || 0}
                    onChange={(e) => handlePllChange(pllName, "divider", Number(e.target.value))}
                    disabled={pllName === "clk_a24k"}
                    className="bg-slate-900 border border-slate-700/80 rounded px-1.5 py-0.5 text-slate-200 w-16 text-center font-mono focus:outline-none focus:ring-1 focus:ring-cyan-500 disabled:opacity-50 disabled:cursor-not-allowed transition-all"
                  />
                </div>
                <div className="text-[9px] text-slate-600 flex justify-between leading-none">
                  <span>输入: {mipimpllOutputFreq.toFixed(1)} MHz</span>
                  <span>倍频: {pll.multiplier}</span>
                </div>
              </div>
            );
          })}
        </div>
      );
    }

    if (nodeName === "OSC输出") {
      return (
        <div className="p-2 space-y-2 font-mono text-[11px] flex flex-col justify-around h-[calc(100%-40px)]">
          {OUTPUT_NAMES.map((name) => {
            const out = outputs[name] || { source: "OSC", divider: 1, frequency: 25.0 };
            return (
              <div key={name} className="flex justify-between items-center p-1.5 bg-slate-950/30 rounded-lg border border-slate-850 hover:border-rose-500/10 transition-all">
                <div className="flex flex-col">
                  <span className="font-bold text-slate-300 text-[10px]">{name}</span>
                  <span className="text-[9px] text-slate-600 leading-none">源: {out.source} / 分频: {out.divider}</span>
                </div>
                <span className="font-bold text-rose-400 font-mono text-[10.5px]">{(out.frequency || 0).toFixed(3)}M</span>
              </div>
            );
          })}
        </div>
      );
    }

    // 普通子节点
    const parentClock = NODE_NAME_TO_PARENT_CLOCK[nodeName];
    if (!parentClock) return null;
    const items = SUB_NODE_LISTS[parentClock] || [];
    const groupNodes = subNodes[parentClock] || {};

    return (
      <div className="p-2 space-y-2 font-mono text-[11px] flex flex-col justify-around h-[calc(100%-40px)]">
        {items.map((name) => {
          const out = groupNodes[name] as ClockOutput | undefined;
          const displayFreq = out?.frequency ?? 0;
          const source = out ? out.source : parentClock;
          const div = out ? out.divider : 1;

          return (
            <div key={name} className="flex justify-between items-center p-1.5 bg-slate-950/30 rounded-lg border border-slate-850 hover:border-indigo-500/10 transition-all">
              <div className="flex flex-col">
                <span className="font-bold text-slate-300 text-[10px]">{name}</span>
                <span className="text-[9px] text-slate-600 leading-none">源: {source} / 分频: {div}</span>
              </div>
              <span className="font-bold text-indigo-400 font-mono text-[10.5px]">{displayFreq.toFixed(3)}M</span>
            </div>
          );
        })}
      </div>
    );
  };

  const uniqueColors = Array.from(new Set(CONNECTIONS.map((c) => c.color)));

  return (
    <div className="flex flex-col h-full space-y-5 overflow-hidden">
      {/* 头部配置栏 */}
      <div className="flex flex-wrap items-center justify-between gap-4 bg-slate-900/40 border border-slate-800 p-4 rounded-2xl relative overflow-hidden">
        <div className="absolute inset-0 bg-gradient-to-r from-indigo-500/5 to-transparent pointer-events-none" />

        <div className="flex items-center gap-3 relative z-10">
          <div className="bg-indigo-500/10 p-2.5 rounded-xl text-indigo-400 border border-indigo-500/20">
            <Clock size={22} className="animate-pulse" />
          </div>
          <div>
            <h2 className="text-base font-bold text-slate-100">时钟配置 (Clock Layout Config)</h2>
            <p className="text-xs text-slate-500 font-medium">按住卡片顶部标题栏可自由拖拽位置；查看并在线调整各时钟节点的倍频分频系数，全自动实时计算级联频率并存盘</p>
          </div>
        </div>

        <div className="flex items-center gap-3 relative z-10">
          {/* 搜索 */}
          <div className="relative">
            <Search size={14} className="absolute left-3 top-1/2 -translate-y-1/2 text-slate-500" />
            <input
              type="text"
              value={localSearch}
              onChange={handleSearchChange}
              placeholder="搜索时钟节点..."
              className="bg-slate-950/80 border border-slate-800 focus:border-indigo-500 focus:ring-1 focus:ring-indigo-500 focus:outline-none rounded-xl pl-9 pr-4 py-2 text-xs text-slate-200 w-48 placeholder-slate-600 transition-all"
            />
          </div>

          <button
            type="button"
            onClick={handleExport}
            className="bg-indigo-600 hover:bg-indigo-500 active:scale-95 text-white px-5 py-2 rounded-xl text-xs font-bold shadow-lg shadow-indigo-500/20 flex items-center gap-1.5 transition-all focus:outline-none focus:ring-0 border-transparent outline-none cursor-pointer"
          >
            保存时钟树
          </button>
        </div>
      </div>

      {exportMsg && (
        <div className="flex items-center gap-2 bg-indigo-500/10 border border-indigo-500/30 text-indigo-400 p-4 rounded-xl text-xs font-semibold animate-fade-in">
          <span>{exportMsg}</span>
        </div>
      )}

      {/* 主画布拓扑流程图 */}
      <div className="flex-1 w-full overflow-auto custom-scrollbar border border-slate-800/80 rounded-2xl bg-slate-950 relative">
        <div className="w-[2850px] h-[4300px] relative bg-[radial-gradient(#1e293b_1.2px,transparent_1.2px)] [background-size:28px_28px] select-none p-6">
          
          {/* SVG 拓扑连线层 */}
          <svg className="absolute inset-0 w-full h-full pointer-events-none z-0">
            <defs>
              {uniqueColors.map((color) => (
                <marker
                  key={color}
                  id={`arrow-${color.replace("#", "")}`}
                  viewBox="0 0 10 10"
                  refX="8"
                  refY="5"
                  markerWidth="5.5"
                  markerHeight="5.5"
                  orient="auto-start-reverse"
                >
                  <path d="M 0 1.5 L 10 5 L 0 8.5 z" fill={color} />
                </marker>
              ))}
            </defs>

            {/* 渲染所有连线 */}
            {CONNECTIONS.map((conn, idx) => {
              const start = getCoordinate(conn.fromNode, conn.fromItem, true);
              const end = getCoordinate(conn.toNode, conn.toItem, false);
              if (!start || !end) return null;

              // 计算肘形拐点
              const horizontalOffset = 40;
              let elbow1 = { x: start.x, y: start.y };
              let elbow2 = { x: start.x, y: end.y };

              if (start.x < end.x) {
                elbow1 = { x: start.x + horizontalOffset, y: start.y };
                elbow2 = { x: elbow1.x, y: end.y };
              } else {
                elbow1 = { x: start.x - horizontalOffset, y: start.y };
                elbow2 = { x: elbow1.x, y: end.y };
              }

              // 搜索匹配过滤 opacity 动画
              const sourceMatch = checkItemsMatch(conn.fromNode, searchText);
              const targetMatch = checkItemsMatch(conn.toNode, searchText);
              const isHighLight = !searchText || sourceMatch || targetMatch;

              return (
                <g key={idx} className={`${isHighLight ? "opacity-90" : "opacity-[0.06]"} transition-all duration-300`}>
                  <path
                    d={`M ${start.x} ${start.y} L ${elbow1.x} ${elbow1.y} L ${elbow2.x} ${elbow2.y} L ${end.x} ${end.y}`}
                    fill="none"
                    stroke={conn.color}
                    strokeWidth="2.5"
                    markerEnd={`url(#arrow-${conn.color.replace("#", "")})`}
                    className="transition-all duration-300"
                  />
                </g>
              );
            })}
          </svg>

          {/* 渲染 31 个绝对定位卡片 */}
          {Object.entries(mergedPositions).map(([nodeName, defaultPos]) => {
            const colors = NODE_COLORS[nodeName] || {
              border: "border-slate-800",
              bg: "bg-slate-900/60",
              text: "text-slate-200",
              titleBg: "bg-slate-950/20",
            };

            const pos = dragPositions[nodeName] || defaultPos;
            const isMatched = checkItemsMatch(nodeName, searchText);
            const cardOpacity = !searchText || isMatched ? "opacity-100 scale-100" : "opacity-15 scale-[0.98] blur-[0.4px]";

            const isDraggingThis = activeDragName === nodeName;

            return (
              <div
                key={nodeName}
                style={{
                  position: "absolute",
                  left: `${pos.x}px`,
                  top: `${pos.y}px`,
                  width: `${pos.width}px`,
                  height: `${pos.height}px`,
                  transform: isDraggingThis ? "scale(1.02)" : "scale(1)",
                  transition: isDraggingThis ? "none" : "transform 250ms, opacity 250ms",
                }}
                className={`backdrop-blur-md rounded-2xl border ${colors.border} ${colors.bg} ${cardOpacity} shadow-2xl flex flex-col overflow-hidden hover:shadow-[0_0_20px_rgba(99,102,241,0.15)] ${isDraggingThis ? "z-30 shadow-[0_10px_30px_rgba(99,102,241,0.3)] ring-1 ring-indigo-500/20" : "z-10"}`}
              >
                {/* 卡片头部 - 绑定拖拽事件并加上 cursor-move 手势 */}
                <div
                  onMouseDown={(e) => handleMouseDown(e, nodeName)}
                  className={`px-4 py-2 border-b border-slate-800 ${colors.titleBg} flex items-center justify-between cursor-move select-none active:bg-slate-800/20 transition-all`}
                >
                  <span className={`font-bold text-xs ${colors.text} truncate`}>{NODE_TITLES[nodeName] || nodeName}</span>
                </div>

                {/* 卡片主体 */}
                <div className="flex-1 overflow-y-auto custom-scrollbar">
                  {renderCardContent(nodeName)}
                </div>
              </div>
            );
          })}
        </div>
      </div>
    </div>
  );
}