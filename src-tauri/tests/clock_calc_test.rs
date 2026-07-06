use cvicubemx_lib::clock_calc::{PllConfig, OSC_FREQUENCY_MHZ};
use cvicubemx_lib::clock_commands::{compute_clock_tree, export_clock_defconfig};
use std::collections::HashMap;
use std::fs;

#[test]
fn test_clock_tree_calculation_integration() {
    let mut configs = HashMap::new();

    // 1. 添加 clk_mipimpll 节点
    configs.insert(
        "clk_mipimpll".to_string(),
        PllConfig {
            name: "clk_mipimpll".to_string(),
            enabled: true,
            input_freq: OSC_FREQUENCY_MHZ,
            output_freq: 0.0,
            divider: 1.0,
            multiplier: 54, // 25 * 54 = 1350 MHz
            source: "OSC".to_string(),
        },
    );

    // 2. 添加 clk_a0pll 节点 (级联自 clk_mipimpll)
    configs.insert(
        "clk_a0pll".to_string(),
        PllConfig {
            name: "clk_a0pll".to_string(),
            enabled: true,
            input_freq: 0.0, // 将被自动填充为 clk_mipimpll 的 output_freq
            output_freq: 0.0,
            divider: 3.0,
            multiplier: 2, // 1350 * 2 / 3 = 900 MHz
            source: "clk_mipimpll".to_string(),
        },
    );

    let result = compute_clock_tree(configs).unwrap();

    let mipimpll = result.pll_configs.get("clk_mipimpll").unwrap();
    assert_eq!(mipimpll.output_freq, 1350.0);

    let a0pll = result.pll_configs.get("clk_a0pll").unwrap();
    assert_eq!(a0pll.input_freq, 1350.0);
    assert_eq!(a0pll.output_freq, 900.0);
}

#[test]
fn test_export_clock_defconfig_integration() {
    let timestamp = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let temp_dir = std::env::temp_dir().join(format!("cvicubemx_clock_test_{}", timestamp));
    let build_dir = temp_dir
        .join("build")
        .join("boards")
        .join("cv184x")
        .join("cv1842hp");
    fs::create_dir_all(&build_dir).unwrap();

    let defconfig_path = build_dir.join("cv1842hp_defconfig");
    fs::write(&defconfig_path, "CONFIG_OD_CLK_SEL=n\n").unwrap();

    let mut configs = HashMap::new();
    configs.insert(
        "clk_appll".to_string(),
        PllConfig {
            name: "clk_appll".to_string(),
            enabled: true,
            input_freq: OSC_FREQUENCY_MHZ,
            output_freq: 0.0,
            divider: 1.0,
            multiplier: 44, // 触发超频配置 CONFIG_OD_CLK_SEL = y
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

    fs::remove_dir_all(&temp_dir).ok();
}

#[test]
fn test_default_subnode_dividers() {
    let result = compute_clock_tree(HashMap::new()).unwrap();
    let fpll_subnodes = result
        .sub_nodes
        .get("clk_fpll")
        .expect("No clk_fpll subnodes");
    let xtal_misc = fpll_subnodes
        .get("clk_xtal_misc")
        .expect("No clk_xtal_misc");
    assert_eq!(xtal_misc.divider, 40);
}
