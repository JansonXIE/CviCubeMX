/// 代码生成器模块 (对应重构计划 M7)
///
/// 从 C++ codegenerator.cpp/h 提取的代码生成逻辑，实现 cvi_board_init.c 的生成与增量更新。
/// 关键功能:
/// - generate_pinmux_macro: 生成 PINMUX(PAD_name, FUNCTION_name) 宏
/// - generate_code: 生成完整的 cvi_board_init.c 代码
/// - update_existing_code: 增量更新已有文件
/// - generate_eth/mipi/audio_sequence: 特殊寄存器序列
/// - is_gpio_mode: 判断是否为 GPIO 模式

use serde::{Deserialize, Serialize};
use std::collections::{BTreeMap, HashMap};
use std::fs;
use std::path::Path;

/// ETH 相关功能关键词 (对应 C++ generateEthSequence)
pub const ETH_KEYWORDS: [&str; 3] = ["RMII0", "EPHY", "PAD_ETH"];

/// MIPI 相关功能关键词 (对应 C++ generateMipiSequence)
pub const MIPI_KEYWORDS: [&str; 4] = ["MIPI", "VI0_D", "VI1_D", "VI2_D"];

/// Audio 相关功能关键词 (对应 C++ generateAudioSequence)
pub const AUDIO_KEYWORDS: [&str; 4] = ["IIS", "IIC", "AUD", "SPK"];

/// 用户配置的引脚信息 (从前端传入)
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PinConfig {
    /// 引脚 PAD 名称 (如 "PAD_MIPI_TXM4")
    pub pin_name: String,
    /// 当前选择的功能 (如 "XGPIOC_18")
    pub function: String,
    /// 是否已被用户配置 (仅输出 user_configured = true 的引脚)
    pub user_configured: bool,
}

/// 判断某个功能字符串是否表示 GPIO 模式
///
/// 对应 C++ codegenerator.cpp: isGpioMode()
/// 常见 GPIO 表示形式：直接包含 "GPIO"，或以 "XGPIO"/"PWR_GPIO" 等前缀
pub fn is_gpio_mode(func: &str) -> bool {
    if func.is_empty() {
        return false;
    }
    let func_upper = func.to_uppercase();
    func_upper.starts_with("XGPIO")
        || func_upper.starts_with("PWR_GPIO")
        || func_upper.contains("GPIO")
}

/// 生成 PINMUX 宏: PINMUX(PAD_name, FUNCTION_name)
///
/// 对应 C++ codegenerator.cpp: functionToMacro() + getPinMuxName()
/// PINMUX_CONFIG 格式: PINMUX_CONFIG(PAD_name, FUNCTION_name);
pub fn generate_pinmux_macro(pin_name: &str, function: &str) -> String {
    format!("PINMUX({}, {})", pin_name, function)
}

/// 生成 PINMUX_CONFIG 语句: PINMUX_CONFIG(PAD_name, FUNCTION_name);
pub fn generate_pinmux_config_statement(pin_name: &str, function: &str) -> String {
    format!("PINMUX_CONFIG({}, {});", pin_name, function)
}

/// 检查功能是否包含 ETH 关键词
fn has_eth_keyword(function: &str) -> bool {
    ETH_KEYWORDS.iter().any(|k| function.contains(k))
}

/// 检查功能是否包含 MIPI 关键词
fn has_mipi_keyword(function: &str) -> bool {
    MIPI_KEYWORDS.iter().any(|k| function.contains(k))
}

/// 检查功能是否包含 Audio 关键词
fn has_audio_keyword(function: &str) -> bool {
    AUDIO_KEYWORDS.iter().any(|k| function.contains(k))
}

/// 生成 ETH 特殊寄存器序列
///
/// 对应 C++ codegenerator.cpp: generateEthSequence()
/// 当 ETH pads (PAD_ETH_RXM/RXP/TXM/TXP) 配置为 GPIO 时，需要写入特殊寄存器序列
pub fn generate_eth_sequence(pin_functions: &HashMap<String, String>) -> String {
    let special_eth_pads = [
        "PAD_ETH_RXM",
        "PAD_ETH_RXP",
        "PAD_ETH_TXM",
        "PAD_ETH_TXP",
    ];

    // 检查是否有 ETH pad 被配置为 GPIO
    let need = special_eth_pads
        .iter()
        .any(|pad| is_gpio_mode(pin_functions.get(*pad).unwrap_or(&String::new())));

    if !need {
        return String::new();
    }

    let mut seq = String::new();
    seq += "/* Special sequence: configure EPHY for GPIO on ETH pads */\n";
    seq += "/* Note: requires mmio_read/mmio_write and udelay helpers */\n";
    seq += "/* enable apb interface */\n";
    seq += "mmio_write(0x03009804, mmio_read(0x03009804) | 0x1); // rg_ephy_apb_rw_sel = 1\n";
    seq += "/* set pll stable cnt = 1 (10us) */\n";
    seq += "mmio_write(0x03009808, (mmio_read(0x03009808) & ~0x1F) | 0x1);\n";
    seq += "/* release ephy reset */\n";
    seq += "mmio_write(0x03009800, mmio_read(0x03009800) | (1 << 2)); // rg_ephy_dig_rst_n = 1\n";
    seq += "udelay(10); /* wait 10us */\n";
    seq += "/* select page 5 */\n";
    seq += "mmio_write(0x0300987C, (mmio_read(0x0300987C) & ~(0x1F << 8)) | (5 << 8));\n";
    seq += "/* set to gpio from top */\n";
    seq += "mmio_write(0x03009878, (mmio_read(0x03009878) & ~0xFFF) | 0xF00);\n";
    seq += "/* enable ephy rxp&rxm input & output */\n";
    seq += "mmio_write(0x03009874, (mmio_read(0x03009874)| 0x606));\n";
    seq += "mmio_write(0x03009870, (mmio_read(0x03009870)| 0x606));\n";
    seq += "/* back to page 0 */\n";
    seq += "mmio_write(0x0300987C, 0x0);\n";
    seq += "/* set PHY MDI mode to Force MDIX (bits[1:0] = 01) */\n";
    seq += "mmio_write(0x0300984C, (mmio_read(0x0300984C) & ~0x3) | 0x1);\n";
    seq += "\n";
    seq += "/* PAD_ETH PINMUX GPIO extra config END */\n";
    seq += "\n";
    seq
}

/// 生成 MIPI 特殊寄存器序列
///
/// 对应 C++ codegenerator.cpp: generateMipiSequence()
/// 为 MIPI TX/RX pads 生成寄存器配置 (TXM/TXP -> reg_pd_lptrx/reg_pd_txdvr_ldo, RX -> reg_mipirx_pd_rxlp)
pub fn generate_mipi_sequence(pin_functions: &HashMap<String, String>) -> String {
    let txm_pads = [
        "PAD_MIPI_TXM0", "PAD_MIPI_TXP0", "PAD_MIPI_TXM1", "PAD_MIPI_TXP1",
        "PAD_MIPI_TXM2", "PAD_MIPI_TXP2", "PAD_MIPI_TXM3", "PAD_MIPI_TXP3",
        "PAD_MIPI_TXM4", "PAD_MIPI_TXP4",
    ];

    let rx_pads = [
        "PAD_MIPIRX0N", "PAD_MIPIRX0P", "PAD_MIPIRX1N", "PAD_MIPIRX1P",
        "PAD_MIPIRX2N", "PAD_MIPIRX2P", "PAD_MIPIRX3N", "PAD_MIPIRX3P",
        "PAD_MIPIRX4N", "PAD_MIPIRX4P", "PAD_MIPIRX5N", "PAD_MIPIRX5P",
    ];

    let mut need = false;
    let mut val_low: u32 = 0;
    let mut val_top: u32 = 0;
    let mut mask_low: u32 = 0;
    let mut mask_top: u32 = 0;

    // TX pads: i/2 映射到 bit 位
    for (i, pad) in txm_pads.iter().enumerate() {
        let func = pin_functions.get(*pad).map(|s| s.as_str()).unwrap_or("");
        if is_gpio_mode(func) {
            val_low |= 1u32 << (i / 2);
            val_top |= 1u32 << (i / 2 + 8);
            need = true;
        } else if !func.is_empty() {
            mask_low |= 1u32 << (i / 2);
            mask_top |= 1u32 << (i / 2 + 8);
        }
    }

    // RX pads: 16+i/2 映射到 bit 位
    let mut mask_rx: u32 = 0;
    let mut val_rx: u32 = 0;
    for (i, pad) in rx_pads.iter().enumerate() {
        let func = pin_functions.get(*pad).map(|s| s.as_str()).unwrap_or("");
        if is_gpio_mode(func) {
            mask_rx |= 1u32 << (16 + i / 2);
            val_rx |= 1u32 << (16 + i / 2);
            need = true;
        } else if !func.is_empty() {
            mask_rx |= 1u32 << (16 + i / 2);
        }
    }

    if !need {
        return String::new();
    }

    let mut seq = String::new();

    if val_low != 0 {
        seq += "    /* MIPI TX: set reg_pd_lptrx/reg_pd_txdvr_ldo according to GPIO/MIPI selection */\n";
        seq += &format!(
            "    mmio_write(0x0A098064, (mmio_read(0x0A098064) & ~0x{:X}) | 0x{:X});\n",
            mask_low, val_low
        );
    }

    if val_top != 0 {
        seq += &format!(
            "    mmio_write(0x0A098064, (mmio_read(0x0A098064) & ~0x{:X}) | 0x{:X});\n",
            mask_top, val_top
        );
        seq += "\n";
    }

    if val_rx != 0 {
        seq += "    /* MIPI RX: reg_mipirx_pd_rxlp set for GPIO/MIPI */\n";
        seq += &format!(
            "    mmio_write(0x0A0A6000, (mmio_read(0x0A0A6000) & ~0x{:X}) | 0x{:X});\n",
            mask_rx, val_rx
        );
        seq += "\n";
    }

    seq += "\t/* PAD_MIPI PINMUX extra config set END */\n";
    seq += "\n";
    seq
}

/// 生成 Audio 特殊寄存器序列
///
/// 对应 C++ codegenerator.cpp: generateAudioSequence()
/// 为 Audio pads (PAD_AUD_AINL_MIC, PAD_AUD_AINR, PAD_AUD_AOUTL, PAD_AUD_AOUTR)
/// 生成寄存器配置 (analog=00, gpio!=00)
pub fn generate_audio_sequence(pin_functions: &HashMap<String, String>) -> String {
    let mut seq = String::new();
    let mut need = false;

    // PAD_AUD_AINL_MIC, PAD_AUD_AINR -> 0x03002204[23:22], 0x0300212C[3:2]
    let p1 = "PAD_AUD_AINL_MIC";
    let p2 = "PAD_AUD_AINR";
    let pin_value1 = pin_functions.get(p1).map(|s| s.clone()).unwrap_or_default();
    let pin_value2 = pin_functions.get(p2).map(|s| s.clone()).unwrap_or_default();

    if pin_functions.contains_key(p1) || pin_functions.contains_key(p2) {
        let mask: u32 = 0x3u32 << 22;
        let val: u32 = if is_gpio_mode(&pin_value1) || is_gpio_mode(&pin_value2) {
            0x1u32 << 22
        } else {
            0
        };
        seq += &format!(
            "    mmio_write(0x03002204, (mmio_read(0x03002204) & ~0x{:X}) | 0x{:X});\n",
            mask, val
        );
        let mask: u32 = 0x3u32 << 2;
        let val: u32 = if is_gpio_mode(&pin_value1) || is_gpio_mode(&pin_value2) {
            0x1u32 << 2
        } else {
            0
        };
        seq += &format!(
            "    mmio_write(0x0300212C, (mmio_read(0x0300212C) & ~0x{:X}) | 0x{:X});\n",
            mask, val
        );
        need = true;
    }

    // PAD_AUD_AOUTL -> 0x03002204[25:24], PAD_AUD_AOUTR -> 0x03002100[1:0]
    let p3 = "PAD_AUD_AOUTL";
    let p4 = "PAD_AUD_AOUTR";
    let pin_value3 = pin_functions.get(p3).map(|s| s.clone()).unwrap_or_default();
    let pin_value4 = pin_functions.get(p4).map(|s| s.clone()).unwrap_or_default();

    if pin_functions.contains_key(p3) || pin_functions.contains_key(p4) {
        let mask: u32 = 0x3u32 << 24;
        let val: u32 = if is_gpio_mode(&pin_value3) || is_gpio_mode(&pin_value4) {
            0x1u32 << 24
        } else {
            0
        };
        seq += &format!(
            "    mmio_write(0x03002204, (mmio_read(0x03002204) & ~0x{:X}) | 0x{:X});\n",
            mask, val
        );
        let mask: u32 = 0x3u32;
        let val: u32 = if is_gpio_mode(&pin_value3) || is_gpio_mode(&pin_value4) {
            0x1u32
        } else {
            0
        };
        seq += &format!(
            "    mmio_write(0x03002100, (mmio_read(0x03002100) & ~0x{:X}) | 0x{:X});\n",
            mask, val
        );
        need = true;
    }

    if !seq.is_empty() {
        seq = "    /* Audio pad mode adjustments (analog=00, gpio!=00) */\n".to_string() + &seq + "\n";
    }
    if !need {
        return seq;
    }
    seq += "\t/*PAD_AUD PINMUX extra config END*/\n";
    seq += "\n";
    seq
}

/// 生成 PINMUX 配置块 (用于增量更新)
///
/// 对应 C++ codegenerator.cpp: generatePinmuxConfig()
/// 仅输出 user_configured = true 的引脚配置，按功能分组
fn generate_pinmux_config_block(pin_configs: &[PinConfig]) -> String {
    // 过滤: 仅输出 user_configured = true 且非 reset_state 的引脚
    let configured_pins: Vec<&PinConfig> = pin_configs
        .iter()
        .filter(|p| p.user_configured && p.function != "reset_state")
        .collect();

    // 按功能分组 (使用 BTreeMap 保证顺序)
    let mut function_groups: BTreeMap<String, Vec<String>> = BTreeMap::new();
    for pin in &configured_pins {
        // GPIO 不需要特殊配置 (对应 C++ 跳过 "GPIO" 功能)
        if !is_gpio_mode(&pin.function) {
            function_groups
                .entry(pin.function.clone())
                .or_default()
                .push(pin.pin_name.clone());
        }
    }

    let mut config_code = String::new();
    let mut first_group = true;

    for (func_name, pins) in &function_groups {
        if !first_group {
            config_code += "\n";
        }
        first_group = false;

        // 使用制表符缩进，与文件中其他行保持一致
        config_code += &format!("\t// {} pins configuration\n", func_name);

        for pin_name in pins {
            config_code += &format!("\tPINMUX_CONFIG({}, {});\n", pin_name, func_name);
        }
    }

    config_code
}

/// 生成完整的 cvi_board_init.c 代码
///
/// 对应 C++ codegenerator.cpp: generateCode() + generatePinmuxFunction()
/// 生成完整的 cvi_board_init.c 文件内容 (用于不存在已有文件时的新建场景)
pub fn generate_code(chip_type: &str, pin_configs: &[PinConfig]) -> String {
    let mut code = String::new();

    // 生成文件头
    code += "/**\n";
    code += " * @file cvi_board_init.c\n";
    code += " * @brief Board initialization file generated by CviCubeMX\n";
    code += " * @author CviCubeMX Tool\n";
    // 不使用 Date.now() (不可用)，使用固定占位符
    code += " * @date <generated_timestamp>\n";
    code += " */\n\n";

    // 生成引脚复用函数
    code += "/**\n";
    code += " * @brief Initialize board pin multiplexing\n";
    code += &format!(" * @note Generated for chip: {}\n", chip_type);
    code += " */\n";
    code += "int cvi_board_init(void)\n";
    code += "{\n";

    // 芯片类型注释
    code += &format!("    // Pin configuration for {}\n", chip_type);
    code += "    // Package type: ";

    let is_qfn = chip_type.ends_with('c') || chip_type.contains("cp_");
    let is_bga = chip_type.ends_with('h') || chip_type.contains("hp_");

    if is_qfn {
        code += "QFN (Quad Flat No-leads)\n";
    } else if is_bga {
        code += "BGA (Ball Grid Array)\n";
    } else {
        code += "Unknown\n";
    }
    code += "\n";

    // 生成引脚复用配置 (仅 user_configured = true)
    let configured_pins: Vec<&PinConfig> = pin_configs
        .iter()
        .filter(|p| p.user_configured && p.function != "reset_state")
        .collect();

    // 按功能分组
    let mut function_groups: BTreeMap<String, Vec<String>> = BTreeMap::new();
    for pin in &configured_pins {
        if !is_gpio_mode(&pin.function) {
            function_groups
                .entry(pin.function.clone())
                .or_default()
                .push(pin.pin_name.clone());
        }
    }

    // 调试注释: pinFunctions mapping
    code += "    /* Debug: pinFunctions mapping (key -> value)\n";
    if configured_pins.is_empty() {
        code += "     * <empty>\n";
    } else {
        for pin in &configured_pins {
            code += &format!("     * {} -> {}\n", pin.pin_name, pin.function);
        }
    }
    code += "     */\n\n";

    // 调试注释: functionGroups
    code += "    /* Debug: functionGroups (function -> pins)\n";
    if function_groups.is_empty() {
        code += "     * <empty>\n";
    } else {
        for (func, pins) in &function_groups {
            code += &format!("     * {} -> {}\n", func, pins.join(", "));
        }
    }
    code += "     */\n\n";

    // 生成每个功能组的配置
    for (func_name, pins) in &function_groups {
        code += &format!("    // {} pins configuration\n", func_name);
        for pin_name in pins {
            code += &format!("    PINMUX_CONFIG({}, {});\n", pin_name, func_name);
        }
        code += "\n";
    }

    // 生成特殊序列 (ETH / MIPI / Audio)
    let pin_functions: HashMap<String, String> = configured_pins
        .iter()
        .map(|p| (p.pin_name.clone(), p.function.clone()))
        .collect();

    code += &generate_eth_sequence(&pin_functions);
    code += &generate_mipi_sequence(&pin_functions);
    code += &generate_audio_sequence(&pin_functions);

    // 如果没有配置任何引脚，添加默认注释
    if function_groups.is_empty() {
        code += "    // No special pin functions configured\n";
        code += "    // All pins are set to GPIO by default\n";
    }

    code += "}\n";
    code
}

/// 增量更新已有 cvi_board_init.c 文件
///
/// 对应 C++ codegenerator.cpp: updateExistingFile()
/// 查找 "// Generated PINMUX configurations" 块并替换为新生成的配置
/// 如果找不到该块，则在 "return 0;" 之前插入
pub fn update_existing_code(file_path: &str, pin_configs: &[PinConfig]) -> Result<String, String> {
    let path = Path::new(file_path);

    // 读取已有文件内容
    let content = fs::read_to_string(path)
        .map_err(|e| format!("Cannot open file {}: {}", file_path, e))?;

    // 构建用户配置的 pin_functions map
    let pin_functions: HashMap<String, String> = pin_configs
        .iter()
        .filter(|p| p.user_configured)
        .map(|p| (p.pin_name.clone(), p.function.clone()))
        .collect();

    // 生成新的 PINMUX 配置
    let pinmux_config = generate_pinmux_config_block(pin_configs);

    // 生成特殊序列 (ETH / MIPI / Audio)
    let mut special_seq = String::new();
    special_seq += &generate_eth_sequence(&pin_functions);
    special_seq += &generate_mipi_sequence(&pin_functions);
    special_seq += &generate_audio_sequence(&pin_functions);

    // 合并 PINMUX 配置 + 特殊序列
    let mut full_config = pinmux_config;
    if !special_seq.is_empty() {
        if !full_config.ends_with('\n') {
            full_config += "\n";
        }
        // 将特殊序列每行前加一个制表符对齐
        for ln in special_seq.lines() {
            if !ln.is_empty() {
                full_config += &format!("\t{}\n", ln);
            }
        }
    }

    // 查找并替换 "Generated PINMUX configurations" 块
    // 对应 C++ 正则: \n*\s*// Generated PINMUX configurations\n.*?(?=\n*\s*return\s+0\s*;)
    let generated_marker = "// Generated PINMUX configurations";

    let result = if content.contains(generated_marker) {
        // 已有生成块: 删除旧块并在 return 0; 前插入新块
        // Step 1: 删除旧的生成块 (从 "// Generated" 到 "return 0;" 之间的内容)
        let mut new_content = content.clone();

        // 使用 regex 删除旧的生成块
        let re = regex::Regex::new(
            r"\n*\s*// Generated PINMUX configurations\n.*?(?=\n*\s*return\s+0\s*;)"
        ).map_err(|e| format!("Regex error: {}", e))?;

        new_content = re.replace(&new_content, "").to_string();

        // 清理多余换行符
        let re_multiline = regex::Regex::new(r"(\n\s*){3,}(\s*return\s+0\s*;)")
            .map_err(|e| format!("Regex error: {}", e))?;
        new_content = re_multiline
            .replace(&new_content, "\n\n\treturn 0;")
            .to_string();

        // Step 2: 在 return 0; 前插入新配置
        let return_re = regex::Regex::new(r"\n*\s*return\s+0\s*;")
            .map_err(|e| format!("Regex error: {}", e))?;

        if let Some(mat) = return_re.find(&new_content) {
            let return_pos = mat.start();

            let mut result = new_content[..return_pos].to_string();
            if !result.ends_with('\n') {
                result += "\n";
            }

            if !full_config.is_empty() {
                result += "\n\t// Generated PINMUX configurations\n";
                result += &full_config;
                result += "\n\treturn 0;\n";
                result += "}";
            } else {
                // 没有配置要添加，保持干净的 return 格式
                result += "\n\treturn 0;\n";
                result += "}";
            }

            // 写回文件
            fs::write(path, &result)
                .map_err(|e| format!("Cannot write to file {}: {}", file_path, e))?;

            if full_config.is_empty() {
                Ok("Existing generated configurations removed".to_string())
            } else {
                Ok("File updated successfully".to_string())
            }
        } else {
            Err("Cannot find 'return 0;' in the existing file".to_string())
        }
    } else {
        // 没有已有生成块: 在 return 0; 前插入新配置
        let return_re = regex::Regex::new(r"return\s+0\s*;")
            .map_err(|e| format!("Regex error: {}", e))?;

        if let Some(mat) = return_re.find(&content) {
            let return_pos = mat.start();

            let mut new_content = content[..return_pos].to_string();
            if !new_content.ends_with('\n') {
                new_content += "\n";
            }

            if !full_config.is_empty() {
                new_content += "\n\t// Generated PINMUX configurations\n";
                new_content += &full_config;
                new_content += "\n\treturn 0;\n";
                new_content += "}";
            } else {
                // 没有配置要添加，保持原样
                new_content += &content[return_pos..];
            }

            // 写回文件
            fs::write(path, &new_content)
                .map_err(|e| format!("Cannot write to file {}: {}", file_path, e))?;

            Ok("File updated successfully".to_string())
        } else {
            Err("Cannot find 'return 0;' in the existing file".to_string())
        }
    };

    result
}

#[cfg(test)]
mod tests {
    use super::*;

    // ---- M7-T1: PINMUX 宏格式 ----

    #[test]
    fn test_pinmux_macro_format() {
        assert_eq!(
            generate_pinmux_macro("PAD_MIPI_TXM4", "XGPIOC_18"),
            "PINMUX(PAD_MIPI_TXM4, XGPIOC_18)"
        );
    }

    #[test]
    fn test_pinmux_macro_uart() {
        assert_eq!(
            generate_pinmux_macro("UART0_TX", "UART0_TX"),
            "PINMUX(UART0_TX, UART0_TX)"
        );
    }

    #[test]
    fn test_pinmux_config_statement() {
        assert_eq!(
            generate_pinmux_config_statement("PAD_MIPI_TXM4", "XGPIOC_18"),
            "PINMUX_CONFIG(PAD_MIPI_TXM4, XGPIOC_18);"
        );
    }

    // ---- is_gpio_mode ----

    #[test]
    fn test_is_gpio_mode_xgpio() {
        assert!(is_gpio_mode("XGPIOC_18"));
    }

    #[test]
    fn test_is_gpio_mode_pwr_gpio() {
        assert!(is_gpio_mode("PWR_GPIO_0"));
    }

    #[test]
    fn test_is_gpio_mode_gpio() {
        assert!(is_gpio_mode("GPIO"));
    }

    #[test]
    fn test_is_gpio_mode_empty() {
        assert!(!is_gpio_mode(""));
    }

    #[test]
    fn test_is_gpio_mode_uart() {
        assert!(!is_gpio_mode("UART0_TX"));
    }

    #[test]
    fn test_is_gpio_mode_iic() {
        assert!(!is_gpio_mode("IIC1_SDA"));
    }

    // ---- ETH 关键词识别 ----

    #[test]
    fn test_eth_keyword_rmii0() {
        assert!(has_eth_keyword("RMII0_TXD1"));
    }

    #[test]
    fn test_eth_keyword_ephy() {
        assert!(has_eth_keyword("EPHY_SPD_LED"));
    }

    #[test]
    fn test_eth_keyword_pad_eth() {
        assert!(has_eth_keyword("PAD_ETH_RXM"));
    }

    #[test]
    fn test_eth_keyword_uart() {
        assert!(!has_eth_keyword("UART0_TX"));
    }

    // ---- MIPI 关键词识别 ----

    #[test]
    fn test_mipi_keyword_mipi() {
        assert!(has_mipi_keyword("PAD_MIPI_TXM4"));
    }

    #[test]
    fn test_mipi_keyword_vi0_d() {
        assert!(has_mipi_keyword("VI0_D_15"));
    }

    #[test]
    fn test_mipi_keyword_uart() {
        assert!(!has_mipi_keyword("UART0_TX"));
    }

    // ---- Audio 关键词识别 ----

    #[test]
    fn test_audio_keyword_iis() {
        assert!(has_audio_keyword("IIS1_BCLK"));
    }

    #[test]
    fn test_audio_keyword_iic() {
        assert!(has_audio_keyword("IIC1_SDA"));
    }

    #[test]
    fn test_audio_keyword_aud() {
        assert!(has_audio_keyword("PAD_AUD_AOUTL"));
    }

    #[test]
    fn test_audio_keyword_spk() {
        assert!(has_audio_keyword("SPK_OUT"));
    }

    // ---- ETH 序列生成 ----

    #[test]
    fn test_eth_sequence_with_gpio() {
        let mut pin_functions = HashMap::new();
        pin_functions.insert("PAD_ETH_RXM".to_string(), "XGPIOB_26".to_string());
        let seq = generate_eth_sequence(&pin_functions);
        assert!(seq.contains("mmio_write(0x03009804"));
        assert!(seq.contains("PAD_ETH PINMUX GPIO extra config END"));
    }

    #[test]
    fn test_eth_sequence_without_gpio() {
        let pin_functions = HashMap::new();
        let seq = generate_eth_sequence(&pin_functions);
        assert!(seq.is_empty());
    }

    // ---- MIPI 序列生成 ----

    #[test]
    fn test_mipi_sequence_with_gpio() {
        let mut pin_functions = HashMap::new();
        pin_functions.insert("PAD_MIPI_TXM4".to_string(), "XGPIOC_18".to_string());
        let seq = generate_mipi_sequence(&pin_functions);
        assert!(seq.contains("mmio_write(0x0A098064"));
        assert!(seq.contains("PAD_MIPI PINMUX extra config set END"));
    }

    #[test]
    fn test_mipi_sequence_without_gpio() {
        let pin_functions = HashMap::new();
        let seq = generate_mipi_sequence(&pin_functions);
        assert!(seq.is_empty());
    }

    // ---- Audio 序列生成 ----

    #[test]
    fn test_audio_sequence_with_gpio() {
        let mut pin_functions = HashMap::new();
        pin_functions.insert("PAD_AUD_AINL_MIC".to_string(), "XGPIOA_0".to_string());
        let seq = generate_audio_sequence(&pin_functions);
        assert!(seq.contains("mmio_write(0x03002204"));
        assert!(seq.contains("PAD_AUD PINMUX extra config END"));
    }

    #[test]
    fn test_audio_sequence_without_gpio() {
        let pin_functions = HashMap::new();
        let seq = generate_audio_sequence(&pin_functions);
        assert!(seq.is_empty());
    }

    // ---- generate_code ----

    #[test]
    fn test_generate_code_basic() {
        let pin_configs = vec![
            PinConfig {
                pin_name: "PAD_MIPI_TXM4".to_string(),
                function: "VI0_D_15".to_string(),
                user_configured: true,
            },
        ];
        let code = generate_code("cv1842hp", &pin_configs);
        assert!(code.contains("cvi_board_init.c"));
        assert!(code.contains("int cvi_board_init(void)"));
        assert!(code.contains("PINMUX_CONFIG(PAD_MIPI_TXM4, VI0_D_15)"));
        assert!(code.contains("BGA"));
    }

    #[test]
    fn test_generate_code_qfn_package() {
        let pin_configs = vec![];
        let code = generate_code("cv1842cp", &pin_configs);
        assert!(code.contains("QFN"));
    }

    #[test]
    fn test_generate_code_skip_gpio() {
        let pin_configs = vec![
            PinConfig {
                pin_name: "PAD_MIPI_TXM4".to_string(),
                function: "XGPIOC_18".to_string(),
                user_configured: true,
            },
        ];
        let code = generate_code("cv1842hp", &pin_configs);
        // GPIO 功能不应生成 PINMUX_CONFIG (但应出现在调试注释中)
        assert!(code.contains("PAD_MIPI_TXM4 -> XGPIOC_18"));
        assert!(!code.contains("PINMUX_CONFIG(PAD_MIPI_TXM4, XGPIOC_18)"));
    }

    #[test]
    fn test_generate_code_skip_reset_state() {
        let pin_configs = vec![
            PinConfig {
                pin_name: "PAD_MIPI_TXM4".to_string(),
                function: "reset_state".to_string(),
                user_configured: true,
            },
        ];
        let code = generate_code("cv1842hp", &pin_configs);
        // reset_state 不应生成任何配置
        assert!(!code.contains("PINMUX_CONFIG"));
    }

    #[test]
    fn test_generate_code_skip_non_user_configured() {
        let pin_configs = vec![
            PinConfig {
                pin_name: "PAD_MIPI_TXM4".to_string(),
                function: "VI0_D_15".to_string(),
                user_configured: false,
            },
        ];
        let code = generate_code("cv1842hp", &pin_configs);
        // 非 user_configured 的引脚不应出现在配置中
        assert!(!code.contains("PINMUX_CONFIG(PAD_MIPI_TXM4, VI0_D_15)"));
    }

    // ---- update_existing_code ----

    #[test]
    fn test_update_existing_code_with_generated_block() {
        let dir = std::env::temp_dir();
        let test_file = dir.join("test_cvi_board_init_generated.c");
        let content = "int board_init(void) {\n    // Generated PINMUX configurations\n    PINMUX(PAD_MIPI_TXM4, XGPIOC_18);\n    return 0;\n}\n";
        fs::write(&test_file, content).unwrap();

        let pin_configs = vec![
            PinConfig {
                pin_name: "PAD_MIPI_TXM4".to_string(),
                function: "VI0_D_15".to_string(),
                user_configured: true,
            },
        ];

        let result = update_existing_code(
            test_file.to_str().unwrap(),
            &pin_configs,
        );
        assert!(result.is_ok());
        assert_eq!(result.unwrap(), "File updated successfully");

        let updated = fs::read_to_string(&test_file).unwrap();
        assert!(updated.contains("// Generated PINMUX configurations"));
        assert!(updated.contains("PINMUX_CONFIG(PAD_MIPI_TXM4, VI0_D_15)"));
        assert!(updated.contains("return 0;"));

        // 清理
        fs::remove_file(&test_file).ok();
    }

    #[test]
    fn test_update_existing_code_without_generated_block() {
        let dir = std::env::temp_dir();
        let test_file = dir.join("test_cvi_board_init_no_generated.c");
        let content = "int board_init(void) {\n    // no generated block\n    return 0;\n}\n";
        fs::write(&test_file, content).unwrap();

        let pin_configs = vec![
            PinConfig {
                pin_name: "PAD_MIPI_TXM4".to_string(),
                function: "VI0_D_15".to_string(),
                user_configured: true,
            },
        ];

        let result = update_existing_code(
            test_file.to_str().unwrap(),
            &pin_configs,
        );
        assert!(result.is_ok());

        let updated = fs::read_to_string(&test_file).unwrap();
        assert!(updated.contains("// Generated PINMUX configurations"));
        assert!(updated.contains("PINMUX_CONFIG(PAD_MIPI_TXM4, VI0_D_15)"));

        // 清理
        fs::remove_file(&test_file).ok();
    }

    #[test]
    fn test_update_existing_code_no_configs() {
        let dir = std::env::temp_dir();
        let test_file = dir.join("test_cvi_board_init_empty.c");
        let content = "int board_init(void) {\n    // Generated PINMUX configurations\n    PINMUX(PAD_MIPI_TXM4, XGPIOC_18);\n    return 0;\n}\n";
        fs::write(&test_file, content).unwrap();

        let pin_configs = vec![];
        let result = update_existing_code(
            test_file.to_str().unwrap(),
            &pin_configs,
        );
        assert!(result.is_ok());

        let updated = fs::read_to_string(&test_file).unwrap();
        // 应删除旧的生成块
        assert!(!updated.contains("PINMUX_CONFIG"));
        assert!(updated.contains("return 0;"));

        // 清理
        fs::remove_file(&test_file).ok();
    }

    #[test]
    fn test_update_existing_code_file_not_found() {
        let pin_configs = vec![];
        let result = update_existing_code("/nonexistent/file.c", &pin_configs);
        assert!(result.is_err());
    }

    #[test]
    fn test_update_existing_code_no_return_zero() {
        let dir = std::env::temp_dir();
        let test_file = dir.join("test_cvi_board_init_no_return.c");
        let content = "int board_init(void) {\n    // some code\n}\n";
        fs::write(&test_file, content).unwrap();

        let pin_configs = vec![
            PinConfig {
                pin_name: "PAD_MIPI_TXM4".to_string(),
                function: "VI0_D_15".to_string(),
                user_configured: true,
            },
        ];

        let result = update_existing_code(
            test_file.to_str().unwrap(),
            &pin_configs,
        );
        assert!(result.is_err());
        assert!(result.unwrap_err().contains("return 0"));

        // 清理
        fs::remove_file(&test_file).ok();
    }
}