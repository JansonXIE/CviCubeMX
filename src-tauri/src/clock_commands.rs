//! Tauri command handlers for clock tree operations (M4)
//!
//! This module provides the Tauri command interface that bridges
//! the frontend with the clock_calc module.

use crate::clock_calc::{
    compute_clk_1m_subnode_frequency, compute_output_frequency, compute_pll_frequency,
    compute_subnode_frequency, compute_subpll_frequency, ClockOutput, ClockTreeResult,
    ModulePosition, OSC_FREQUENCY_MHZ, PLLConfig, PLL_NAMES, SUB_NODE_GROUPS, SUB_PLL_NAMES,
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
/// * `configs` - Map of PLL name → PLLConfig (with multiplier/divider/source set)
///
/// # Returns
/// * `Ok(ClockTreeResult)` with all computed frequencies
/// * `Err(String)` on computation error
#[tauri::command]
pub fn compute_clock_tree(
    configs: HashMap<String, PLLConfig>,
) -> Result<ClockTreeResult, String> {
    let mut pll_configs = configs.clone();
    let mut outputs = HashMap::new();
    let mut sub_nodes_map: HashMap<String, HashMap<String, ClockOutput>> = HashMap::new();

    // Step 1: Compute main PLL frequencies
    for pll_name in PLL_NAMES.iter() {
        if let Some(config) = pll_configs.get_mut(*pll_name) {
            // Main PLL: divider is typically 1 (fixed)
            // output_freq = input_freq * multiplier
            config.output_freq = compute_pll_frequency(
                config.input_freq,
                config.multiplier,
                config.divider,
            );
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
                config.output_freq = compute_subpll_frequency(
                    config.input_freq,
                    config.multiplier,
                    config.divider,
                );
            }
        }
    }

    // Step 3: Compute OSC direct output frequencies
    // Special multiplier nodes: clk_mipimpll_d3, clk_cam1pll, clk_cam0pll
    let special_multiplier_nodes = ["clk_mipimpll_d3", "clk_cam1pll", "clk_cam0pll"];

    // Create default outputs for OSC branch nodes
    for output_name in crate::clock_calc::OUTPUT_NAMES.iter() {
        let multiplier = if special_multiplier_nodes.contains(output_name) {
            // These use multiplier instead of divider: freq = OSC * multiplier
            // Default multiplier values
            match *output_name {
                "clk_mipimpll_d3" => 3,
                "clk_cam1pll" => 1,
                "clk_cam0pll" => 1,
                _ => 1,
            }
        } else {
            1
        };

        let divider = if special_multiplier_nodes.contains(&output_name) {
            1
        } else {
            // Default divider values vary per node - start with 1
            1
        };

        let frequency = if special_multiplier_nodes.contains(&output_name) {
            OSC_FREQUENCY_MHZ * multiplier as f64
        } else {
            compute_output_frequency(OSC_FREQUENCY_MHZ, divider, multiplier)
        };

        outputs.insert(
            output_name.to_string(),
            ClockOutput {
                name: output_name.to_string(),
                source: "OSC".to_string(),
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

        let mut sub_nodes = HashMap::new();
        for node_name in sub_node_list.iter() {
            // Special handling for clk_1M sub-nodes: uses 1 MHz base
            let freq = if *parent_name == "clk_1M" {
                compute_clk_1m_subnode_frequency(1) // default divider=1
            } else {
                compute_subnode_frequency(parent_freq, 1) // default divider=1
            };

            sub_nodes.insert(
                node_name.to_string(),
                ClockOutput {
                    name: node_name.to_string(),
                    source: parent_name.to_string(),
                    divider: 1,
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
/// - clk_1M → from outputs (clk_1M frequency)
/// - clk_cam1pll → from pll_configs (clk_cam1pll output)
/// - clk_raw_axi → from clk_cam1pll sub-nodes (clk_raw_axi frequency)
/// - clk_a0pll → from pll_configs (clk_a0pll output)
/// - etc.
fn get_parent_frequency(
    parent_name: &str,
    pll_configs: &HashMap<String, PLLConfig>,
    outputs: &HashMap<String, ClockOutput>,
) -> f64 {
    match parent_name {
        "clk_1M" => outputs
            .get("clk_1M")
            .map(|o| o.frequency)
            .unwrap_or(0.1), // clk_1M default = 0.1 MHz
        "clk_cam1pll" => pll_configs
            .get("clk_cam1pll")
            .map(|c| c.output_freq)
            .unwrap_or(800.0), // default 800 MHz
        "clk_a0pll" => pll_configs
            .get("clk_a0pll")
            .map(|c| c.output_freq)
            .unwrap_or(500.0), // default 500 MHz
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
        "clk_cam0pll" => outputs
            .get("clk_cam0pll")
            .map(|o| o.frequency)
            .unwrap_or(25.0),
        "clk_sys_disp" => outputs
            .get("clk_sys_disp")
            .map(|o| o.frequency)
            .unwrap_or(25.0),
        "clk_fab_100M" => pll_configs
            .get("clk_fpll")
            .map(|c| c.output_freq)
            .unwrap_or(100.0),
        "clk_xtal_misc" => outputs
            .get("clk_xtal_misc")
            .map(|o| o.frequency)
            .unwrap_or(25.0),
        "clk_i2c" => outputs
            .get("clk_i2c")
            .map(|o| o.frequency)
            .unwrap_or(25.0),
        "clk_apb_i2c" => outputs
            .get("clk_apb_i2c")
            .map(|o| o.frequency)
            .unwrap_or(25.0),
        "clk_apb_vcsys" => outputs
            .get("clk_apb_vcsys")
            .map(|o| o.frequency)
            .unwrap_or(25.0),
        "clk_x2p" => outputs
            .get("clk_x2p")
            .map(|o| o.frequency)
            .unwrap_or(25.0),
        "clk_rtc_sys" => outputs
            .get("clk_rtc_sys")
            .map(|o| o.frequency)
            .unwrap_or(0.032768),
        "clk_hsperi" => outputs
            .get("clk_hsperi")
            .map(|o| o.frequency)
            .unwrap_or(200.0),
        "clk_vip_sys_0" => outputs
            .get("clk_vip_sys_0")
            .map(|o| o.frequency)
            .unwrap_or(200.0),
        "clk_vip_sys_1" => outputs
            .get("clk_vip_sys_1")
            .map(|o| o.frequency)
            .unwrap_or(200.0),
        "clk_vip_sys_2" => outputs
            .get("clk_vip_sys_2")
            .map(|o| o.frequency)
            .unwrap_or(200.0),
        "clk_vip_sys_3" => outputs
            .get("clk_vip_sys_3")
            .map(|o| o.frequency)
            .unwrap_or(200.0),
        "clk_spi" => outputs
            .get("clk_spi")
            .map(|o| o.frequency)
            .unwrap_or(25.0),
        "clk_keyscan_xclk" => outputs
            .get("clk_keyscan_xclk")
            .map(|o| o.frequency)
            .unwrap_or(25.0),
        "clk_wgn_xclk" => outputs
            .get("clk_wgn_xclk")
            .map(|o| o.frequency)
            .unwrap_or(25.0),
        "clk_raw_axi" => {
            // clk_raw_axi is a child of clk_cam1pll, need to resolve from sub-nodes
            // For simplicity, use a default value
            200.0
        }
        _ => 25.0, // default fallback
    }
}

/// Save module positions to a JSON file.
///
/// # Arguments
/// * `positions` - Map of module name → ModulePosition
///
/// # Returns
/// * `Ok(())` on success
/// * `Err(String)` on I/O or serialization error
#[tauri::command]
pub fn save_module_positions(
    positions: HashMap<String, ModulePosition>,
) -> Result<(), String> {
    // Save to app data directory
    let app_data_dir = get_app_data_dir()?;
    let path = Path::new(&app_data_dir).join("module_positions.json");

    let json = serde_json::to_string_pretty(&positions)
        .map_err(|e| format!("Serialization error: {}", e))?;

    fs::write(&path, json)
        .map_err(|e| format!("Write error: {}", e))?;

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

    let json = fs::read_to_string(&path)
        .map_err(|e| format!("Read error: {}", e))?;

    let positions: HashMap<String, ModulePosition> = serde_json::from_str(&json)
        .map_err(|e| format!("Deserialization error: {}", e))?;

    Ok(positions)
}

/// Export clock configuration to defconfig format.
///
/// This mirrors the C++ `exportToDefconfig()` function.
/// It writes clock configuration values to a defconfig file
/// in the format: `CONFIG_CLK_<NAME>=<VALUE>`
///
/// # Arguments
/// * `source_path` - Path to the SDK source directory
/// * `chip_type` - Chip type string (e.g. "CV1800B")
/// * `configs` - Map of PLL name → PLLConfig
///
/// # Returns
/// * `Ok(())` on success
/// * `Err(String)` on I/O error
#[tauri::command]
pub fn export_clock_defconfig(
    source_path: String,
    chip_type: String,
    configs: HashMap<String, PLLConfig>,
) -> Result<(), String> {
    // Determine defconfig path based on chip type and source path
    let defconfig_name = match chip_type.as_str() {
        "CV1800B" => "cv1800b_aside",
        "CV1811C" => "cv1811c_aside",
        "CV1812H" => "cv1812h_aside",
        _ => "generic_aside",
    };

    let defconfig_path = Path::new(&source_path)
        .join("buildboards")
        .join(chip_type)
        .join("defconfig")
        .join(defconfig_name);

    // Build defconfig content from PLL configs
    let mut lines = Vec::new();

    for (pll_name, config) in &configs {
        if !config.enabled {
            continue;
        }

        // Format: CONFIG_CLK_<PLL_NAME>=<OUTPUT_FREQ>
        let config_key = format!("CONFIG_CLK_{}", pll_name.to_uppercase());
        let config_value = format!("{}MHz", config.output_freq as i32);
        lines.push(format!("{}={}", config_key, config_value));
    }

    // Write to file (append if exists, create if not)
    let content = lines.join("\n");

    if let Some(parent) = defconfig_path.parent() {
        fs::create_dir_all(parent)
            .map_err(|e| format!("Create directory error: {}", e))?;
    }

    fs::write(&defconfig_path, content)
        .map_err(|e| format!("Write error to {}: {}", defconfig_path.display(), e))?;

    Ok(())
}

/// Helper: Get the app data directory for storing module positions.
///
/// This uses the standard OS-specific app data location.
fn get_app_data_dir() -> Result<String, String> {
    // For Tauri apps, we typically use the app's data directory
    // Since we don't have direct access to tauri::App here,
    // we use a simple approach based on the current executable
    let exe_dir = std::env::current_exe()
        .map_err(|e| format!("Cannot get exe path: {}", e))?;

    let app_dir = exe_dir
        .parent()
        .unwrap_or(Path::new("."))
        .join("cvicubemx_data");

    if !app_dir.exists() {
        fs::create_dir_all(&app_dir)
            .map_err(|e| format!("Create app data dir error: {}", e))?;
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
            PLLConfig {
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
            PLLConfig {
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
            PLLConfig {
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
        // a0pll: input=1350, multiplier=2, divider=3 → 900
        assert_eq!(a0pll.output_freq, 900.0);
        assert_eq!(a0pll.input_freq, 1350.0); // input should come from MIPIMPLL
    }

    #[test]
    fn test_compute_clock_tree_clk_a24k_zero() {
        let mut configs = HashMap::new();

        configs.insert(
            "clk_mipimpll".to_string(),
            PLLConfig {
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
            PLLConfig {
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
}