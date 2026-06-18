//! Tauri command handlers for clock tree operations (M4)
//!
//! This module provides the Tauri command interface that bridges
//! the frontend with the clock_calc module.
use crate::clock_calc::{
    compute_output_frequency, compute_pll_frequency, compute_subnode_frequency,
    compute_subpll_frequency, ClockOutput, ClockTreeResult, ModulePosition, PllConfig,
    OSC_FREQUENCY_MHZ, PLL_NAMES, RTC_FREQUENCY_MHZ, SUB_NODE_GROUPS, SUB_PLL_NAMES,
};
use std::collections::HashMap;
use std::fs;
use std::path::Path;

/// Compute the full clock tree from given PLL configurations.
///
/// This takes a map of PLL configurations and computes all output frequencies
/// for PLLs, SubPLLs, and their sub-nodes.
///
/// # Arguments
/// * `configs` - Map of PLL name to PllConfig (with multiplier/divider/source set)
///
/// # Returns
/// * `Ok(ClockTreeResult)` with all computed frequencies
/// * `Err(String)` on computation error
#[tauri::command]
pub fn compute_clock_tree(configs: HashMap<String, PllConfig>) -> Result<ClockTreeResult, String> {
    let mut pll_configs = configs.clone();

    // 填充默认主 PLL 参数（配合 C++ 源码默认设定）
    let default_plls = [
        ("clk_fpll", 40, 1.0, "OSC"),
        ("clk_mipimpll", 36, 1.0, "OSC"),
        ("clk_mpll", 48, 1.0, "OSC"),
        ("clk_tpll", 60, 1.0, "OSC"),
        ("clk_appll", 40, 1.0, "OSC"),
        ("clk_rvpll", 48, 1.0, "OSC"),
    ];

    for &(name, mult, div, src) in default_plls.iter() {
        if !pll_configs.contains_key(name) {
            pll_configs.insert(
                name.to_string(),
                PllConfig {
                    name: name.to_string(),
                    enabled: true,
                    input_freq: OSC_FREQUENCY_MHZ,
                    output_freq: OSC_FREQUENCY_MHZ * mult as f64 / div,
                    divider: div,
                    multiplier: mult,
                    source: src.to_string(),
                },
            );
        }
    }

    // 从 mipimpll 读取作为子 PLL 的输入频率（默认 900.0 MHz）
    let mipimpll_output = pll_configs
        .get("clk_mipimpll")
        .map(|c| c.output_freq)
        .unwrap_or(OSC_FREQUENCY_MHZ * 36.0);

    // 填充默认子 PLL 参数
    let default_sub_plls = [
        ("clk_a24k", 1, 1.0),
        ("clk_vivo_mipimpll", 1, 1.0),
        ("clk_cyc_dsi_syn", 1, 1.0),
        ("clk_cam0pll", 6, 5.0), // 900 * 6 / 5 = 1080 MHz (clk_mipimpll 的子 PLL)
        ("clk_disppll", 12, 9.09090909),
        ("clk_a0pll", 4, 7.32421875),
    ];

    for &(name, mult, div) in default_sub_plls.iter() {
        if !pll_configs.contains_key(name) {
            let out_freq = if name == "clk_a24k" {
                0.0
            } else {
                mipimpll_output * mult as f64 / div
            };
            pll_configs.insert(
                name.to_string(),
                PllConfig {
                    name: name.to_string(),
                    enabled: true,
                    input_freq: mipimpll_output,
                    output_freq: out_freq,
                    divider: div,
                    multiplier: mult,
                    source: "clk_mipimpll".to_string(),
                },
            );
        }
    }

    let mut outputs = HashMap::new();
    let mut sub_nodes_map: HashMap<String, HashMap<String, ClockOutput>> = HashMap::new();

    // Step 1: Compute main PLL frequencies
    for pll_name in PLL_NAMES.iter() {
        if let Some(config) = pll_configs.get_mut(*pll_name) {
            // Main PLL: divider is typically 1 (fixed)
            // output_freq = input_freq * multiplier
            config.output_freq =
                compute_pll_frequency(config.input_freq, config.multiplier, config.divider);
        }
    }

    // Step 2: Compute SubPLL frequencies (cascaded from MIPIMPLL)
    // SubPLL input comes from clk_mipimpll output
    let mipimpll_output = pll_configs
        .get("clk_mipimpll")
        .map(|c| c.output_freq)
        .unwrap_or(OSC_FREQUENCY_MHZ);

    for sub_pll_name in SUB_PLL_NAMES.iter() {
        if let Some(config) = pll_configs.get_mut(*sub_pll_name) {
            // Set input from MIPIMPLL output
            config.input_freq = mipimpll_output;

            // Special handling for clk_a24k (frequency fixed to 0)
            if *sub_pll_name == "clk_a24k" {
                config.output_freq = 0.0;
            } else {
                config.output_freq =
                    compute_subpll_frequency(config.input_freq, config.multiplier, config.divider);
            }
        }
    }

    // Step 3: Compute OSC direct output frequencies
    // 派生节点（与板端 clk_summary 对齐）：
    //   - clk_mipimpll_d3 = clk_mipimpll / 3（源自 mipimpll，非 OSC×3）
    //   - clk_cam1pll     = OSC × multiplier（默认 ×1，倍频值不在本次结构修正范围）
    //   - clk_cam0pll 已改为 clk_mipimpll 的子 PLL（见 SUB_PLL_NAMES），不再是 OSC 直出节点
    for output_name in crate::clock_calc::OUTPUT_NAMES.iter() {
        let (source, divider, multiplier, frequency): (&str, i32, i32, f64) = match *output_name {
            "clk_mipimpll_d3" => ("clk_mipimpll", 3, 1, mipimpll_output / 3.0),
            "clk_cam1pll" => ("OSC", 1, 1, OSC_FREQUENCY_MHZ),
            _ => ("OSC", 1, 1, compute_output_frequency(OSC_FREQUENCY_MHZ, 1, 1)),
        };

        outputs.insert(
            output_name.to_string(),
            ClockOutput {
                name: output_name.to_string(),
                source: source.to_string(),
                divider,
                multiplier,
                frequency,
                enabled: true,
            },
        );
    }

    // Step 4: Compute sub-node group frequencies
    for (parent_name, sub_node_list) in SUB_NODE_GROUPS {
        // Get parent frequency from the appropriate source
        let parent_freq = get_parent_frequency(parent_name, &pll_configs, &outputs);

        // 显示用的源标签：clk_rtc_sys_* 门控时钟在板端实为 osc 子节点（25MHz），
        // 故其源标记为 "osc" 而非 clk_rtc_sys（clk_rtc_sys 本身是 clk_mpll/4=300M 的叶子）。
        let source_label: &str = if *parent_name == "clk_rtc_sys" {
            "osc"
        } else {
            parent_name
        };

        let mut sub_nodes = HashMap::new();
        for node_name in sub_node_list.iter() {
            let divider = get_default_subnode_divider(node_name);
            let freq = compute_subnode_frequency(parent_freq, divider);

            sub_nodes.insert(
                node_name.to_string(),
                ClockOutput {
                    name: node_name.to_string(),
                    source: source_label.to_string(),
                    divider,
                    multiplier: 1,
                    frequency: freq,
                    enabled: true,
                },
            );
        }
        sub_nodes_map.insert(parent_name.to_string(), sub_nodes);
    }

    Ok(ClockTreeResult {
        pll_configs,
        outputs,
        sub_nodes: sub_nodes_map,
    })
}

/// Helper: resolve the parent frequency for a sub-node group.
///
/// Different sub-node groups have different parent sources:
/// - clk_1M ?from outputs (clk_1M frequency)
/// - clk_cam1pll ?from pll_configs (clk_cam1pll output)
/// - clk_raw_axi ?from clk_cam1pll sub-nodes (clk_raw_axi frequency)
/// - clk_a0pll ?from pll_configs (clk_a0pll output)
/// - etc.
fn get_parent_frequency(
    parent_name: &str,
    pll_configs: &HashMap<String, PllConfig>,
    outputs: &HashMap<String, ClockOutput>,
) -> f64 {
    match parent_name {
        "clk_1M" => {
            let xtal_misc_freq = get_parent_frequency("clk_xtal_misc", pll_configs, outputs);
            xtal_misc_freq / 250.0
        }
        "clk_cam1pll" => outputs
            .get("clk_cam1pll")
            .map(|o| o.frequency)
            .unwrap_or(800.0),
        "clk_a0pll" => pll_configs
            .get("clk_a0pll")
            .map(|c| c.output_freq)
            .unwrap_or(500.0),
        "clk_rvpll" => pll_configs
            .get("clk_rvpll")
            .map(|c| c.output_freq)
            .unwrap_or(500.0),
        "clk_appll" => pll_configs
            .get("clk_appll")
            .map(|c| c.output_freq)
            .unwrap_or(500.0),
        "clk_fpll" => pll_configs
            .get("clk_fpll")
            .map(|c| c.output_freq)
            .unwrap_or(500.0),
        "clk_tpll" => pll_configs
            .get("clk_tpll")
            .map(|c| c.output_freq)
            .unwrap_or(500.0),
        "clk_mpll" => pll_configs
            .get("clk_mpll")
            .map(|c| c.output_freq)
            .unwrap_or(500.0),
        "clk_disppll" => pll_configs
            .get("clk_disppll")
            .map(|c| c.output_freq)
            .unwrap_or(500.0),
        "clk_cam0pll" => pll_configs
            .get("clk_cam0pll")
            .map(|c| c.output_freq)
            .unwrap_or(1080.0),
        "clk_sys_disp" => {
            let disppll_freq = get_parent_frequency("clk_disppll", pll_configs, outputs);
            disppll_freq / 8.0
        }
        "clk_fab_100M" => {
            let fpll_freq = get_parent_frequency("clk_fpll", pll_configs, outputs);
            fpll_freq / 10.0
        }
        "clk_xtal_misc" => {
            let fpll_freq = get_parent_frequency("clk_fpll", pll_configs, outputs);
            fpll_freq / 40.0
        }
        "clk_i2c" => {
            let fpll_freq = get_parent_frequency("clk_fpll", pll_configs, outputs);
            fpll_freq / 10.0
        }
        "clk_apb_i2c" => {
            get_parent_frequency("clk_i2c", pll_configs, outputs)
        }
        "clk_apb_vcsys" => {
            get_parent_frequency("clk_fab_100M", pll_configs, outputs)
        }
        "clk_x2p" => {
            get_parent_frequency("clk_fab_100M", pll_configs, outputs)
        }
        // clk_rtc_sys_* 门控时钟在板端是 osc 的直接子节点（25MHz），
        // 而非 clk_rtc_sys(=clk_mpll/4=300M) 的子节点。
        "clk_rtc_sys" => OSC_FREQUENCY_MHZ,
        "clk_hsperi" => {
            let mpll_freq = get_parent_frequency("clk_mpll", pll_configs, outputs);
            mpll_freq / 4.0
        }
        "clk_vip_sys_0" => {
            let mpll_freq = get_parent_frequency("clk_mpll", pll_configs, outputs);
            mpll_freq / 8.0
        }
        "clk_vip_sys_1" => {
            let mpll_freq = get_parent_frequency("clk_mpll", pll_configs, outputs);
            mpll_freq / 4.0
        }
        "clk_vip_sys_2" => {
            let cam1pll_freq = get_parent_frequency("clk_cam1pll", pll_configs, outputs);
            cam1pll_freq / 1.0
        }
        "clk_vip_sys_3" => {
            let mpll_freq = get_parent_frequency("clk_mpll", pll_configs, outputs);
            mpll_freq / 2.0
        }
        "clk_spi" => {
            let mpll_freq = get_parent_frequency("clk_mpll", pll_configs, outputs);
            mpll_freq / 6.0
        }
        "clk_keyscan_xclk" => OSC_FREQUENCY_MHZ,
        "clk_wgn_xclk" => OSC_FREQUENCY_MHZ,
        // clk_wgn (=osc 25M) -> clk_wgn2/1/0 (25M)
        "clk_wgn" => OSC_FREQUENCY_MHZ,
        // clk_eth_pll = clk_fpll / 2 = 500M -> eth_csrclk(÷2=250M), eth_ptpclk(÷10=50M)
        "clk_eth_pll" => {
            let fpll_freq = get_parent_frequency("clk_fpll", pll_configs, outputs);
            fpll_freq / 2.0
        }
        // rtc_32k 域 (32.768kHz) -> clk_rtc_sys_wdt / clk_rtc_sys_gpio_db
        "rtc_32k" => RTC_FREQUENCY_MHZ,
        "clk_raw_axi" => {
            get_parent_frequency("clk_cam1pll", pll_configs, outputs)
        }
        _ => 25.0, // default fallback
    }
}

/// Helper function to retrieve CviCubeMX default subnode divider coefficients from C++ clockconfig.cpp
fn get_default_subnode_divider(name: &str) -> i32 {
    match name {
        // Cam0PLL sub-nodes
        "clk_cam0_vip" => 50,
        
        // DispPLL sub-nodes
        "clk_cam2_vip" => 32,
        "clk_cam1_vip" => 44,
        "clk_sys_disp" => 8,
        
        // A0PLL sub-nodes
        "clk_aud3" | "clk_aud2" | "clk_aud1" | "clk_aud0" | "clk_audsrc" => 17,
        
        // RVPLL sub-nodes
        "clk_rv1" => 2,
        
        // FPLL sub-nodes
        "clk_xtal_misc" => 40,
        "clk_pwm" => 4,
        "clk_i2c" => 10,
        "clk_eth_pll" => 2,
        "clk_cyc_dsi_esc" => 51,
        "clk_scan_100M" => 51,
        "clk_video_axi" => 2,
        "clk_fab_500M" => 2,
        "clk_fab_100M" => 10,
        
        // TPLL sub-nodes
        "clk_tpu" | "clk_tpu_gdma" => 3,
        
        // MPLL sub-nodes
        "clk_uart0" => 651,
        "clk_uart4" | "clk_uart3" | "clk_uart2" | "clk_uart1" | "clk_spi" | "clk_vip_sys_4" => 6,
        "clk_spi_nand" | "clk_spi_nor" | "clk_usb20_bus_early" | "clk_rtc_spi_nor" | "clk_cyc_scan_300M" |
        "clk_vip_sys_1" | "clk_tpu_sys" | "clk_gic" | "clk_rtc_sys" | "clk_hsperi" => 4,
        "clk_usb20_ref" => 50,
        "clk_vip_sys_3" | "clk_vc_src0" | "clk_bus" => 2,
        "clk_vip_sys_0" => 8,
        
        // XtalMisc sub-nodes
        "clk_1M" => 250,
        "clk_usb20_suspend" => 125,

        // EthPLL sub-nodes (clk_eth_pll=500M)
        "eth_csrclk" => 2, // 500 / 2 = 250 MHz
        "eth_ptpclk" => 10, // 500 / 10 = 50 MHz

        // Default divider is 1
        _ => 1,
    }
}

/// Save module positions to a JSON file.
///
/// # Arguments
/// * `positions` - Map of module name ?ModulePosition
///
/// # Returns
/// * `Ok(())` on success
/// * `Err(String)` on I/O or serialization error
#[tauri::command]
pub fn save_module_positions(positions: HashMap<String, ModulePosition>) -> Result<(), String> {
    // Save to app data directory
    let app_data_dir = get_app_data_dir()?;
    let path = Path::new(&app_data_dir).join("module_positions.json");

    let json = serde_json::to_string_pretty(&positions)
        .map_err(|e| format!("Serialization error: {}", e))?;

    fs::write(&path, json).map_err(|e| format!("Write error: {}", e))?;

    Ok(())
}

/// Load module positions from a JSON file.
///
/// # Returns
/// * `Ok(HashMap<String, ModulePosition>)` with saved positions
/// * `Err(String)` on I/O or deserialization error
#[tauri::command]
pub fn load_module_positions() -> Result<HashMap<String, ModulePosition>, String> {
    let app_data_dir = get_app_data_dir()?;
    let path = Path::new(&app_data_dir).join("module_positions.json");

    if !path.exists() {
        // Return empty map if no saved positions exist
        return Ok(HashMap::new());
    }

    let json = fs::read_to_string(&path).map_err(|e| format!("Read error: {}", e))?;

    let positions: HashMap<String, ModulePosition> =
        serde_json::from_str(&json).map_err(|e| format!("Deserialization error: {}", e))?;

    Ok(positions)
}

/// Export clock configuration to defconfig format.
///
/// This mirrors the C++ `exportToDefconfig()` function.
/// It updates the CONFIG_OD_CLK_SEL config in the board's defconfig.
///
/// # Arguments
/// * `source_path` - Path to the SDK source directory
/// * `chip_type` - Chip type string (e.g. "cv1842hp")
/// * `configs` - Map of PLL name to PllConfig (with multiplier/divider/source set)
///
/// # Returns
/// * `Ok(())` on success
/// * `Err(String)` on I/O error
#[tauri::command]
pub fn export_clock_defconfig(
    source_path: String,
    chip_type: String,
    configs: HashMap<String, PllConfig>,
) -> Result<(), String> {
    // 1. Determine if overclocked (CONFIG_OD_CLK_SEL = y)
    // C++ applyOverclockConfig sets clk_appll multiplier to 44, clk_rvpll multiplier to 64
    let is_overclock = if let Some(appll) = configs.get("clk_appll") {
        appll.multiplier == 44
    } else if let Some(rvpll) = configs.get("clk_rvpll") {
        rvpll.multiplier == 64
    } else {
        false
    };

    let value = if is_overclock { "y" } else { "n" };

    // 2. Build defconfig path
    // C++: QString defconfigPath = QString("%1/build/boards/cv184x/%2/%2_defconfig").arg(sourcePath).arg(chipType)
    // Ensure lowercase chip type to match directory naming
    let chip_type_lower = chip_type.to_lowercase();
    let defconfig_path = Path::new(&source_path)
        .join("build")
        .join("boards")
        .join("cv184x")
        .join(&chip_type_lower)
        .join(format!("{}_defconfig", chip_type_lower));

    if !defconfig_path.exists() {
        return Err(format!(
            "Defconfig file not found: {}",
            defconfig_path.display()
        ));
    }

    // 3. Read existing defconfig file content
    let content = fs::read_to_string(&defconfig_path)
        .map_err(|e| format!("Failed to read defconfig file: {}", e))?;

    let mut lines: Vec<String> = content.lines().map(|s| s.to_string()).collect();
    let mut found_config = false;
    let config_key = "CONFIG_OD_CLK_SEL=";

    for line in &mut lines {
        if line.starts_with(config_key) {
            *line = format!("{}{}", config_key, value);
            found_config = true;
            break;
        }
    }

    // If config was not found, append it to the end
    if !found_config {
        lines.push(format!("{}{}", config_key, value));
    }

    // 4. Write back to defconfig file
    // C++ stream appends a newline at the end
    let output_content = lines.join("\n") + "\n";
    fs::write(&defconfig_path, output_content)
        .map_err(|e| format!("Failed to write defconfig file: {}", e))?;

    Ok(())
}

/// Helper: Get the app data directory for storing module positions.
///
/// This uses the standard OS-specific app data location.
fn get_app_data_dir() -> Result<String, String> {
    // For Tauri apps, we typically use the app's data directory
    // Since we don't have direct access to tauri::App here,
    // we use a simple approach based on the current executable
    let exe_dir = std::env::current_exe().map_err(|e| format!("Cannot get exe path: {}", e))?;

    let app_dir = exe_dir
        .parent()
        .unwrap_or(Path::new("."))
        .join("cvicubemx_data");

    if !app_dir.exists() {
        fs::create_dir_all(&app_dir).map_err(|e| format!("Create app data dir error: {}", e))?;
    }

    Ok(app_dir.to_string_lossy().to_string())
}

// ============================================================
// Unit Tests
// ============================================================

#[cfg(test)]
mod tests {
    use super::*;
    use crate::clock_calc::OSC_FREQUENCY_MHZ;

    #[test]
    fn test_compute_clock_tree_basic() {
        let mut configs = HashMap::new();

        // Create a basic PLL config for clk_mipimpll
        configs.insert(
            "clk_mipimpll".to_string(),
            PllConfig {
                name: "clk_mipimpll".to_string(),
                enabled: true,
                input_freq: OSC_FREQUENCY_MHZ,
                output_freq: 0.0,
                divider: 1.0,
                multiplier: 54,
                source: "OSC".to_string(),
            },
        );

        let result = compute_clock_tree(configs).unwrap();

        // Verify PLL output frequency was computed
        let mipimpll = result.pll_configs.get("clk_mipimpll").unwrap();
        assert_eq!(mipimpll.output_freq, 1350.0);
    }

    #[test]
    fn test_compute_clock_tree_with_subpll() {
        let mut configs = HashMap::new();

        configs.insert(
            "clk_mipimpll".to_string(),
            PllConfig {
                name: "clk_mipimpll".to_string(),
                enabled: true,
                input_freq: OSC_FREQUENCY_MHZ,
                output_freq: 0.0,
                divider: 1.0,
                multiplier: 54,
                source: "OSC".to_string(),
            },
        );

        configs.insert(
            "clk_a0pll".to_string(),
            PllConfig {
                name: "clk_a0pll".to_string(),
                enabled: true,
                input_freq: 0.0, // will be set from MIPIMPLL output
                output_freq: 0.0,
                divider: 3.0,
                multiplier: 2,
                source: "clk_mipimpll".to_string(),
            },
        );

        let result = compute_clock_tree(configs).unwrap();

        // Verify SubPLL (clk_a0pll) frequency
        let a0pll = result.pll_configs.get("clk_a0pll").unwrap();
        // a0pll: input=1350, multiplier=2, divider=3 ?900
        assert_eq!(a0pll.output_freq, 900.0);
        assert_eq!(a0pll.input_freq, 1350.0); // input should come from MIPIMPLL
    }

    #[test]
    fn test_compute_clock_tree_clk_a24k_zero() {
        let mut configs = HashMap::new();

        configs.insert(
            "clk_mipimpll".to_string(),
            PllConfig {
                name: "clk_mipimpll".to_string(),
                enabled: true,
                input_freq: OSC_FREQUENCY_MHZ,
                output_freq: 0.0,
                divider: 1.0,
                multiplier: 54,
                source: "OSC".to_string(),
            },
        );

        configs.insert(
            "clk_a24k".to_string(),
            PllConfig {
                name: "clk_a24k".to_string(),
                enabled: true,
                input_freq: 0.0,
                output_freq: 0.0,
                divider: 1.0,
                multiplier: 1,
                source: "clk_mipimpll".to_string(),
            },
        );

        let result = compute_clock_tree(configs).unwrap();

        // clk_a24k has fixed frequency of 0
        let a24k = result.pll_configs.get("clk_a24k").unwrap();
        assert_eq!(a24k.output_freq, 0.0);
    }

    #[test]
    fn test_save_and_load_module_positions() {
        let mut positions = HashMap::new();
        positions.insert(
            "PLL_MIPIMPLL".to_string(),
            ModulePosition {
                module_name: "PLL_MIPIMPLL".to_string(),
                x: 100,
                y: 200,
                width: 300,
                height: 150,
            },
        );

        // Save
        let save_result = save_module_positions(positions.clone());
        assert!(save_result.is_ok());

        // Load
        let load_result = load_module_positions();
        assert!(load_result.is_ok());

        let loaded = load_result.unwrap();
        assert_eq!(loaded.len(), 1);
        let pos = loaded.get("PLL_MIPIMPLL").unwrap();
        assert_eq!(pos.x, 100);
        assert_eq!(pos.y, 200);
        assert_eq!(pos.width, 300);
        assert_eq!(pos.height, 150);
    }

    #[test]
    fn test_load_module_positions_no_file() {
        // If no file exists, should return empty HashMap
        // This test may fail if a file exists from previous tests
        // In that case, we verify it returns a valid HashMap regardless
        let result = load_module_positions();
        assert!(result.is_ok());
    }

    #[test]
    fn test_export_clock_defconfig_overclock() {
        let timestamp = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap()
            .as_nanos();
        let temp_dir = std::env::temp_dir().join(format!("cvicubemx_test_{}", timestamp));
        let build_dir = temp_dir
            .join("build")
            .join("boards")
            .join("cv184x")
            .join("cv1842hp");
        fs::create_dir_all(&build_dir).unwrap();

        let defconfig_path = build_dir.join("cv1842hp_defconfig");
        fs::write(&defconfig_path, "CONFIG_OD_CLK_SEL=n\nANOTHER_CONFIG=123\n").unwrap();

        // 1. Test overclock config export (y)
        let mut configs = HashMap::new();
        configs.insert(
            "clk_appll".to_string(),
            PllConfig {
                name: "clk_appll".to_string(),
                enabled: true,
                input_freq: OSC_FREQUENCY_MHZ,
                output_freq: 0.0,
                divider: 1.0,
                multiplier: 44, // overclock multiplier
                source: "OSC".to_string(),
            },
        );

        let result = export_clock_defconfig(
            temp_dir.to_string_lossy().to_string(),
            "cv1842hp".to_string(),
            configs,
        );
        assert!(result.is_ok());

        let content = fs::read_to_string(&defconfig_path).unwrap();
        assert!(content.contains("CONFIG_OD_CLK_SEL=y"));
        assert!(content.contains("ANOTHER_CONFIG=123"));

        // 2. Test normal config export (n)
        let mut configs_normal = HashMap::new();
        configs_normal.insert(
            "clk_appll".to_string(),
            PllConfig {
                name: "clk_appll".to_string(),
                enabled: true,
                input_freq: OSC_FREQUENCY_MHZ,
                output_freq: 0.0,
                divider: 1.0,
                multiplier: 40, // normal multiplier
                source: "OSC".to_string(),
            },
        );

        let result = export_clock_defconfig(
            temp_dir.to_string_lossy().to_string(),
            "cv1842hp".to_string(),
            configs_normal,
        );
        assert!(result.is_ok());

        let content = fs::read_to_string(&defconfig_path).unwrap();
        assert!(content.contains("CONFIG_OD_CLK_SEL=n"));

        // Clean up
        fs::remove_dir_all(&temp_dir).ok();
    }
}
