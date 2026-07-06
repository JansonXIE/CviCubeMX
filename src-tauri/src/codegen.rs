/// 代码生成器模块 (对应重构计划 M7)
///
/// 从 C++ codegenerator.cpp/h 提取的代码生成逻辑，实现 cvi_board_init.c 的生成与增量更新。
/// 关键功能:
/// - generate_pinmux_macro: 生成 PINMUX(PAD_name, FUNCTION_name) 宏
/// - generate_code: 生成完整的 cvi_board_init.c 代码
/// - update_existing_code: 增量更新已有文件
/// - generate_eth/mipi/audio_sequence: 特殊寄存器序列
/// - is_gpio_mode: 判断是否为 GPIO 模式
use handlebars::Handlebars;
use serde::{Deserialize, Serialize};
use std::collections::{BTreeMap, HashMap};
use std::fs;
use std::path::Path;

const TEMPLATE_CONTENT: &str = include_str!("../templates/cvi_board_init.c.hbs");

#[derive(Serialize)]
struct TemplateContext {
    date: String,
    chip_type: String,
    package_type: String,
    pin_mappings: Vec<PinMapping>,
    function_groups: Vec<FunctionGroup>,
    pinmux_configs: Vec<PinmuxConfigGroup>,
    eth_sequence: String,
    mipi_sequence: String,
    audio_sequence: String,
    is_empty: bool,
    incremental_only: bool,
}

#[derive(Serialize)]
struct PinMapping {
    pin_name: String,
    function: String,
}

#[derive(Serialize)]
struct FunctionGroup {
    func_name: String,
    pins: String,
}

#[derive(Serialize)]
struct PinmuxConfigGroup {
    func_name: String,
    pins: Vec<PinmuxConfigItem>,
}

#[derive(Serialize)]
struct PinmuxConfigItem {
    pin_name: String,
    pinmux_macro: String,
}

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
    /// 二级 mux 功能选择 (仅当 function 为 MUX_SPI1_* 时有意义)
    #[serde(default)]
    pub state: Option<String>,
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

/// 从已有的 cvi_board_init.c 内容中解析出「工具生成块」内的引脚复用配置
///
/// 本工具只管理 `// Generated PINMUX configurations` 标记块（该块由
/// generate_code / update_existing_code 写入，位于 `return 0;` 之前）。
/// 回读时也**只解析该块**，绝不解析用户手写代码或 `#if 0` 禁用块——否则再次
/// 生成时会重复写入手写行、甚至把被禁用的配置复活成有效配置。
///
/// 返回的 PinConfig 均标记 user_configured = true；若文件中没有生成块则返回空列表。
///
/// 注意: GPIO 功能在生成时不会写出 PINMUX_CONFIG 行 (见 is_gpio_mode)，
/// 因此无法从文件反向恢复被设为 GPIO 的引脚——这些引脚将回落到默认功能。
pub fn parse_board_init(content: &str) -> Vec<PinConfig> {
    const MARKER: &str = "// Generated PINMUX configurations";

    // 仅在生成块范围内解析：从标记之后到 `return 0;` 之前
    let block = match content.find(MARKER) {
        Some(pos) => {
            let after = &content[pos + MARKER.len()..];
            match after.find("return") {
                Some(end) => &after[..end],
                None => after,
            }
        }
        None => return Vec::new(),
    };

    let re = regex::Regex::new(r"PINMUX_CONFIG\s*\(\s*([A-Za-z0-9_]+)\s*,\s*([A-Za-z0-9_]+)\s*\)")
        .expect("valid PINMUX_CONFIG regex");

    let mut result: Vec<PinConfig> = Vec::new();
    let mut seen = std::collections::HashSet::new();
    // 二级 mux 行 PINMUX_CONFIG(MUX_name, sub) 暂存，循环后回填到对应引脚的 state
    let mut mux_state: HashMap<String, String> = HashMap::new();

    for line in block.lines() {
        // 跳过注释行 (生成块内含 "// xxx pins configuration" 等注释)
        let trimmed = line.trim_start();
        if trimmed.starts_with("//") || trimmed.starts_with('*') || trimmed.starts_with("/*") {
            continue;
        }

        if let Some(caps) = re.captures(line) {
            let first = caps[1].to_string();
            let second = caps[2].to_string();
            if crate::pin_mux::is_mux_function(&first) {
                // 二级行: 第一个参数是 MUX 名称，第二个是二级功能
                mux_state.insert(first, second);
            } else if seen.insert(first.clone()) {
                result.push(PinConfig {
                    pin_name: first,
                    function: second,
                    user_configured: true,
                    state: None,
                });
            }
        }
    }

    // 把二级功能回填到 function 为 MUX 的引脚上
    for cfg in result.iter_mut() {
        if crate::pin_mux::is_mux_function(&cfg.function) {
            cfg.state = mux_state.get(&cfg.function).cloned();
        }
    }

    result
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

/// 生成 ETH pad 解锁寄存器序列
///
/// PAD_ETH_TXP/TXM/RXP/RXM 默认被内部 EPHY 占用 (硬件锁定)。当客户不使用 ETH、
/// 而把这几个 pad 用作其它功能 (UART3/IIC1/GPIO/PWM/CAM/SPI1/IIS2) 时，
/// 需要写入该序列把 pad 从 EPHY 释放出来。
///
/// 触发条件: 这 4 个 pad 中任意一个被用户配置 (无论配成哪种功能)——因为它们的
/// supported_functions 中没有任何 ETH 功能，被配置即意味着要脱离 EPHY。
pub fn generate_eth_sequence(pin_functions: &HashMap<String, String>) -> String {
    let special_eth_pads = ["PAD_ETH_RXM", "PAD_ETH_RXP", "PAD_ETH_TXM", "PAD_ETH_TXP"];

    // 只要有任意 ETH pad 被用户配置 (出现在 map 中) 就需要解锁
    let need = special_eth_pads
        .iter()
        .any(|pad| pin_functions.contains_key(*pad));

    if !need {
        return String::new();
    }

    let mut seq = String::new();
    seq += "/* Unlock ETH pads (release from internal EPHY) for non-ETH pinmux use */\n";
    seq += "/* Note: requires mmio_write_32 and udelay helpers */\n";
    seq += "mmio_write_32(0x03009804, 0x1);\n";
    seq += "mmio_write_32(0x0300907C, 0x0500);\n";
    seq += "mmio_write_32(0x03009078, 0x1000);\n";
    seq += "mmio_write_32(0x03009050, 0x4000);\n";
    seq += "mmio_write_32(0x0300907C, 0x0);\n";
    seq += "mmio_write_32(0x03009804, 0x0);\n";
    seq += "mmio_write_32(0x03009804, 0x1);\n";
    seq += "mmio_write_32(0x03009808, 0x181);\n";
    seq += "mmio_write_32(0x03009800, 0x0905);\n";
    seq += "udelay(10); /* wait 10us */\n";
    seq += "mmio_write_32(0x0300907C, 0x0500);\n";
    seq += "mmio_write_32(0x03009078, 0x1F00);\n";
    seq += "mmio_write_32(0x03009074, 0x606);\n";
    seq += "mmio_write_32(0x03009070, 0x606);\n";
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
        "PAD_MIPI_TXM0",
        "PAD_MIPI_TXP0",
        "PAD_MIPI_TXM1",
        "PAD_MIPI_TXP1",
        "PAD_MIPI_TXM2",
        "PAD_MIPI_TXP2",
        "PAD_MIPI_TXM3",
        "PAD_MIPI_TXP3",
        "PAD_MIPI_TXM4",
        "PAD_MIPI_TXP4",
    ];

    let rx_pads = [
        "PAD_MIPIRX0N",
        "PAD_MIPIRX0P",
        "PAD_MIPIRX1N",
        "PAD_MIPIRX1P",
        "PAD_MIPIRX2N",
        "PAD_MIPIRX2P",
        "PAD_MIPIRX3N",
        "PAD_MIPIRX3P",
        "PAD_MIPIRX4N",
        "PAD_MIPIRX4P",
        "PAD_MIPIRX5N",
        "PAD_MIPIRX5P",
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
            "    mmio_write_32(0x0A098064, (mmio_read(0x0A098064) & ~0x{:X}) | 0x{:X});\n",
            mask_low, val_low
        );
    }

    if val_top != 0 {
        seq += &format!(
            "    mmio_write_32(0x0A098064, (mmio_read(0x0A098064) & ~0x{:X}) | 0x{:X});\n",
            mask_top, val_top
        );
        seq += "\n";
    }

    if val_rx != 0 {
        seq += "    /* MIPI RX: reg_mipirx_pd_rxlp set for GPIO/MIPI */\n";
        seq += &format!(
            "    mmio_write_32(0x0A0A6000, (mmio_read(0x0A0A6000) & ~0x{:X}) | 0x{:X});\n",
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
            "    mmio_write_32(0x03002204, (mmio_read(0x03002204) & ~0x{:X}) | 0x{:X});\n",
            mask, val
        );
        let mask: u32 = 0x3u32 << 2;
        let val: u32 = if is_gpio_mode(&pin_value1) || is_gpio_mode(&pin_value2) {
            0x1u32 << 2
        } else {
            0
        };
        seq += &format!(
            "    mmio_write_32(0x0300212C, (mmio_read(0x0300212C) & ~0x{:X}) | 0x{:X});\n",
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
            "    mmio_write_32(0x03002204, (mmio_read(0x03002204) & ~0x{:X}) | 0x{:X});\n",
            mask, val
        );
        let mask: u32 = 0x3u32;
        let val: u32 = if is_gpio_mode(&pin_value3) || is_gpio_mode(&pin_value4) {
            0x1u32
        } else {
            0
        };
        seq += &format!(
            "    mmio_write_32(0x03002100, (mmio_read(0x03002100) & ~0x{:X}) | 0x{:X});\n",
            mask, val
        );
        need = true;
    }

    if !seq.is_empty() {
        seq = "    /* Audio pad mode adjustments (analog=00, gpio!=00) */\n".to_string()
            + &seq
            + "\n";
    }
    if !need {
        return seq;
    }
    seq += "\t/*PAD_AUD PINMUX extra config END*/\n";
    seq += "\n";
    seq
}

/// 构建 pinmux 配置组
///
/// 普通功能: 每个引脚一行 `PINMUX_CONFIG(pin_name, function)`。
/// 二级 mux 功能 (MUX_SPI1_*): 在同一组内紧接着追加第二行
/// `PINMUX_CONFIG(MUX_name, sub_function)`，两行相邻输出。
/// sub_function 取被配置引脚的 state，缺省回落到该 mux 的默认值。
fn build_pinmux_configs(
    function_groups: &BTreeMap<String, Vec<String>>,
    configured_pins: &[&PinConfig],
) -> Vec<PinmuxConfigGroup> {
    // 收集每个 mux 对应的二级功能
    let mut mux_state: HashMap<String, String> = HashMap::new();
    for pin in configured_pins {
        if crate::pin_mux::is_mux_function(&pin.function) {
            let sub = pin
                .state
                .clone()
                .or_else(|| crate::pin_mux::mux_default(&pin.function))
                .unwrap_or_default();
            if !sub.is_empty() {
                mux_state.insert(pin.function.clone(), sub);
            }
        }
    }

    let mut pinmux_configs = Vec::new();
    for (func_name, pins) in function_groups {
        let mut items: Vec<PinmuxConfigItem> = pins
            .iter()
            .map(|pin_name| PinmuxConfigItem {
                pin_name: pin_name.clone(),
                pinmux_macro: func_name.clone(),
            })
            .collect();

        // 二级 mux: 追加 PINMUX_CONFIG(MUX_name, sub_function)
        if crate::pin_mux::is_mux_function(func_name) {
            if let Some(sub) = mux_state.get(func_name) {
                items.push(PinmuxConfigItem {
                    pin_name: func_name.clone(),
                    pinmux_macro: sub.clone(),
                });
            }
        }

        pinmux_configs.push(PinmuxConfigGroup {
            func_name: func_name.clone(),
            pins: items,
        });
    }
    pinmux_configs
}

/// 生成完整的 cvi_board_init.c 文件内容 (用于不存在已有文件时的新建场景)
pub fn generate_code(
    chip_type: &str,
    pin_configs: &[PinConfig],
    output_path: Option<&str>,
) -> Result<String, String> {
    let configured_pins: Vec<&PinConfig> = pin_configs
        .iter()
        .filter(|p| p.user_configured && p.function != "reset_state")
        .collect();

    // 按功能分组 (使用 BTreeMap 保证顺序)
    let mut function_groups: BTreeMap<String, Vec<String>> = BTreeMap::new();
    for pin in &configured_pins {
        if !is_gpio_mode(&pin.function) {
            function_groups
                .entry(pin.function.clone())
                .or_default()
                .push(pin.pin_name.clone());
        }
    }

    let is_qfn = chip_type.ends_with('c') || chip_type.contains("cp");
    let is_bga = chip_type.ends_with('h') || chip_type.contains("hp");

    let package_type = if is_qfn {
        "QFN (Quad Flat No-leads)"
    } else if is_bga {
        "BGA (Ball Grid Array)"
    } else {
        "Unknown"
    };

    let pin_mappings: Vec<PinMapping> = configured_pins
        .iter()
        .map(|p| PinMapping {
            pin_name: p.pin_name.clone(),
            function: p.function.clone(),
        })
        .collect();

    let function_groups_vec: Vec<FunctionGroup> = function_groups
        .iter()
        .map(|(func_name, pins)| FunctionGroup {
            func_name: func_name.clone(),
            pins: pins.join(", "),
        })
        .collect();

    let pinmux_configs = build_pinmux_configs(&function_groups, &configured_pins);

    let pin_functions: HashMap<String, String> = configured_pins
        .iter()
        .map(|p| (p.pin_name.clone(), p.function.clone()))
        .collect();

    let eth_sequence = generate_eth_sequence(&pin_functions);
    let mipi_sequence = generate_mipi_sequence(&pin_functions);
    let audio_sequence = generate_audio_sequence(&pin_functions);

    let context = TemplateContext {
        date: "<generated_timestamp>".to_string(),
        chip_type: chip_type.to_string(),
        package_type: package_type.to_string(),
        pin_mappings,
        function_groups: function_groups_vec,
        pinmux_configs,
        eth_sequence,
        mipi_sequence,
        audio_sequence,
        is_empty: function_groups.is_empty(),
        incremental_only: false,
    };

    let mut reg = Handlebars::new();
    reg.register_template_string("cvi_board_init", TEMPLATE_CONTENT)
        .map_err(|e| format!("Template register error: {}", e))?;

    let code = reg
        .render("cvi_board_init", &context)
        .map_err(|e| format!("Rendering error: {}", e))?;

    // 如果指定了输出路径，将代码写入文件
    if let Some(path) = output_path {
        if !path.trim().is_empty() {
            fs::write(path, &code).map_err(|e| format!("Cannot write to file {}: {}", path, e))?;
        }
    }

    Ok(code)
}

/// 增量更新已有 cvi_board_init.c 文件
///
/// 对应 C++ codegenerator.cpp: updateExistingFile()
pub fn update_existing_code(file_path: &str, pin_configs: &[PinConfig]) -> Result<String, String> {
    let path = Path::new(file_path);

    // 读取已有文件内容
    let content =
        fs::read_to_string(path).map_err(|e| format!("Cannot open file {}: {}", file_path, e))?;

    // 构建用户配置的 pin_functions map
    let pin_functions: HashMap<String, String> = pin_configs
        .iter()
        .filter(|p| p.user_configured)
        .map(|p| (p.pin_name.clone(), p.function.clone()))
        .collect();

    // 过滤配置
    let configured_pins: Vec<&PinConfig> = pin_configs
        .iter()
        .filter(|p| p.user_configured && p.function != "reset_state")
        .collect();

    // 按功能分组 (使用 BTreeMap 保证顺序)
    let mut function_groups: BTreeMap<String, Vec<String>> = BTreeMap::new();
    for pin in &configured_pins {
        if !is_gpio_mode(&pin.function) {
            function_groups
                .entry(pin.function.clone())
                .or_default()
                .push(pin.pin_name.clone());
        }
    }

    let pinmux_configs = build_pinmux_configs(&function_groups, &configured_pins);

    // 生成特殊序列 (ETH / MIPI / Audio)
    let eth_raw = generate_eth_sequence(&pin_functions);
    let mipi_raw = generate_mipi_sequence(&pin_functions);
    let audio_raw = generate_audio_sequence(&pin_functions);

    // 增量更新中，将特殊序列每行前加一个制表符对齐
    let eth_sequence = if !eth_raw.is_empty() {
        let mut out = String::new();
        for ln in eth_raw.lines() {
            if !ln.is_empty() {
                out += &format!("\t{}\n", ln);
            }
        }
        out
    } else {
        String::new()
    };

    let mipi_sequence = if !mipi_raw.is_empty() {
        let mut out = String::new();
        for ln in mipi_raw.lines() {
            if !ln.is_empty() {
                out += &format!("\t{}\n", ln);
            }
        }
        out
    } else {
        String::new()
    };

    let audio_sequence = if !audio_raw.is_empty() {
        let mut out = String::new();
        for ln in audio_raw.lines() {
            if !ln.is_empty() {
                out += &format!("\t{}\n", ln);
            }
        }
        out
    } else {
        String::new()
    };

    let context = TemplateContext {
        date: String::new(),
        chip_type: String::new(),
        package_type: String::new(),
        pin_mappings: Vec::new(),
        function_groups: Vec::new(),
        pinmux_configs,
        eth_sequence,
        mipi_sequence,
        audio_sequence,
        is_empty: function_groups.is_empty(),
        incremental_only: true,
    };

    let mut reg = Handlebars::new();
    reg.register_template_string("cvi_board_init", TEMPLATE_CONTENT)
        .map_err(|e| format!("Template register error: {}", e))?;

    let full_config = reg
        .render("cvi_board_init", &context)
        .map_err(|e| format!("Rendering error: {}", e))?;

    // 查找并替换 "Generated PINMUX configurations" 块
    let generated_marker = "// Generated PINMUX configurations";

    let result = if content.contains(generated_marker) {
        // 已有生成块: 删除旧块并在 return 0; 前插入新块
        let mut new_content = content.clone();

        if let Some(pos1) = new_content.find(generated_marker) {
            if let Some(pos2) = new_content[pos1..].find("return 0;") {
                let actual_pos2 = pos1 + pos2;
                let mut start_pos = pos1;
                while start_pos > 0 {
                    let prev_char = new_content.as_bytes()[start_pos - 1];
                    if prev_char == b'\n'
                        || prev_char == b'\r'
                        || prev_char == b' '
                        || prev_char == b'\t'
                    {
                        start_pos -= 1;
                    } else {
                        break;
                    }
                }
                new_content.replace_range(start_pos..actual_pos2, "");
            }
        }

        // 清理多余换行符
        let re_multiline = regex::Regex::new(r"(\n\s*){3,}(\s*return\s+0\s*;)")
            .map_err(|e| format!("Regex error: {}", e))?;
        new_content = re_multiline
            .replace(&new_content, "\n\n\treturn 0;")
            .to_string();

        // 在 return 0; 前插入新配置
        let return_re = regex::Regex::new(r"\n*\s*return\s+0\s*;")
            .map_err(|e| format!("Regex error: {}", e))?;

        if let Some(mat) = return_re.find(&new_content) {
            let return_pos = mat.start();

            let mut result = new_content[..return_pos].to_string();
            if !result.ends_with('\n') {
                result += "\n";
            }

            if !full_config.trim().is_empty() {
                result += "\n\t// Generated PINMUX configurations\n";
                result += &full_config;
                if !result.ends_with('\n') {
                    result += "\n";
                }
                result += "\treturn 0;\n";
                result += "}";
            } else {
                // 没有配置要添加，保持干净的 return 格式
                result += "\n\treturn 0;\n";
                result += "}";
            }

            // 写回文件
            fs::write(path, &result)
                .map_err(|e| format!("Cannot write to file {}: {}", file_path, e))?;

            if full_config.trim().is_empty() {
                Ok("Existing generated configurations removed".to_string())
            } else {
                Ok("File updated successfully".to_string())
            }
        } else {
            Err("Cannot find 'return 0;' in the existing file".to_string())
        }
    } else {
        // 没有已有生成块: 在 return 0; 前插入新配置
        let return_re =
            regex::Regex::new(r"return\s+0\s*;").map_err(|e| format!("Regex error: {}", e))?;

        if let Some(mat) = return_re.find(&content) {
            let return_pos = mat.start();

            let mut new_content = content[..return_pos].to_string();
            if !new_content.ends_with('\n') {
                new_content += "\n";
            }

            if !full_config.trim().is_empty() {
                new_content += "\n\t// Generated PINMUX configurations\n";
                new_content += &full_config;
                if !new_content.ends_with('\n') {
                    new_content += "\n";
                }
                new_content += "\treturn 0;\n";
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
        assert!(seq.contains("mmio_write_32(0x03009804"));
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
        assert!(seq.contains("mmio_write_32(0x0A098064"));
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
        assert!(seq.contains("mmio_write_32(0x03002204"));
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
        let pin_configs = vec![PinConfig {
            pin_name: "PAD_MIPI_TXM4".to_string(),
            function: "VI0_D_15".to_string(),
            user_configured: true,
            state: None,
        }];
        let code = generate_code("cv1842hp", &pin_configs, None).unwrap();
        assert!(code.contains("cvi_board_init.c"));
        assert!(code.contains("int cvi_board_init(void)"));
        assert!(code.contains("PINMUX_CONFIG(PAD_MIPI_TXM4, VI0_D_15)"));
        assert!(code.contains("BGA"));
    }

    #[test]
    fn test_generate_code_qfn_package() {
        let pin_configs = vec![];
        let code = generate_code("cv1842cp", &pin_configs, None).unwrap();
        assert!(code.contains("QFN"));
    }

    #[test]
    fn test_generate_code_skip_gpio() {
        let pin_configs = vec![PinConfig {
            pin_name: "PAD_MIPI_TXM4".to_string(),
            function: "XGPIOC_18".to_string(),
            user_configured: true,
            state: None,
        }];
        let code = generate_code("cv1842hp", &pin_configs, None).unwrap();
        // GPIO 功能不应生成 PINMUX_CONFIG (但应出现在调试注释中)
        assert!(code.contains("PAD_MIPI_TXM4 -> XGPIOC_18"));
        assert!(!code.contains("PINMUX_CONFIG(PAD_MIPI_TXM4, XGPIOC_18)"));
    }

    #[test]
    fn test_generate_code_skip_reset_state() {
        let pin_configs = vec![PinConfig {
            pin_name: "PAD_MIPI_TXM4".to_string(),
            function: "reset_state".to_string(),
            user_configured: true,
            state: None,
        }];
        let code = generate_code("cv1842hp", &pin_configs, None).unwrap();
        // reset_state 不应生成 any 配置
        assert!(!code.contains("PINMUX_CONFIG"));
    }

    #[test]
    fn test_generate_code_skip_non_user_configured() {
        let pin_configs = vec![PinConfig {
            pin_name: "PAD_MIPI_TXM4".to_string(),
            function: "VI0_D_15".to_string(),
            user_configured: false,
            state: None,
        }];
        let code = generate_code("cv1842hp", &pin_configs, None).unwrap();
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

        let pin_configs = vec![PinConfig {
            pin_name: "PAD_MIPI_TXM4".to_string(),
            function: "VI0_D_15".to_string(),
            user_configured: true,
            state: None,
        }];

        let result = update_existing_code(test_file.to_str().unwrap(), &pin_configs);
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

        let pin_configs = vec![PinConfig {
            pin_name: "PAD_MIPI_TXM4".to_string(),
            function: "VI0_D_15".to_string(),
            user_configured: true,
            state: None,
        }];

        let result = update_existing_code(test_file.to_str().unwrap(), &pin_configs);
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
        let result = update_existing_code(test_file.to_str().unwrap(), &pin_configs);
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

        let pin_configs = vec![PinConfig {
            pin_name: "PAD_MIPI_TXM4".to_string(),
            function: "VI0_D_15".to_string(),
            user_configured: true,
            state: None,
        }];

        let result = update_existing_code(test_file.to_str().unwrap(), &pin_configs);
        assert!(result.is_err());
        assert!(result.unwrap_err().contains("return 0"));

        // 清理
        fs::remove_file(&test_file).ok();
    }

    // ---- parse_board_init ----

    #[test]
    fn test_parse_board_init_basic() {
        let content = "int cvi_board_init(void) {\n    // Generated PINMUX configurations\n    // IIC1 pins configuration\n    PINMUX_CONFIG(PAD_MIPI_TXM4, VI0_D_15);\n    PINMUX_CONFIG(UART0_TX, UART0_TX);\n    return 0;\n}";
        let configs = parse_board_init(content);
        assert_eq!(configs.len(), 2);
        let p = configs
            .iter()
            .find(|c| c.pin_name == "PAD_MIPI_TXM4")
            .unwrap();
        assert_eq!(p.function, "VI0_D_15");
        assert!(p.user_configured);
    }

    #[test]
    fn test_parse_board_init_ignores_out_of_block_lines() {
        // 只解析生成块内的行: #if 0 禁用块 + 手写行都在生成块之外，必须忽略
        let content = "int cvi_board_init(void)\n{\n#if 0 /* pinmux set in alios */\n\tPINMUX_CONFIG(CAM_MCLK0, CAM_MCLK0);\n\tPINMUX_CONFIG(IIC2_SCL, IIC2_SCL);\n#endif\n\n\tPINMUX_CONFIG(JTAG_CPU_TMS, UART1_TX);\n\tPINMUX_CONFIG(UART2_TX, UART2_TX);\n\tmmio_setbits_32(0x030002d0, 1 << 9);\n\n\t// Generated PINMUX configurations\n\t// IIC1_SCL pins configuration\n\tPINMUX_CONFIG(PAD_MIPIRX0N, IIC1_SCL);\n\t// IIC1_SDA pins configuration\n\tPINMUX_CONFIG(PAD_MIPI_TXM4, IIC1_SDA);\n\n\treturn 0;\n}";
        let configs = parse_board_init(content);
        // 只应解析出生成块内配置的两个引脚，禁用块与手写行都被忽略
        assert_eq!(configs.len(), 2);
        assert!(configs
            .iter()
            .any(|c| c.pin_name == "PAD_MIPIRX0N" && c.function == "IIC1_SCL"));
        assert!(configs
            .iter()
            .any(|c| c.pin_name == "PAD_MIPI_TXM4" && c.function == "IIC1_SDA"));
        // 禁用块 / 手写行不得出现
        assert!(!configs.iter().any(|c| c.pin_name == "CAM_MCLK0"));
        assert!(!configs.iter().any(|c| c.pin_name == "IIC2_SCL"));
        assert!(!configs.iter().any(|c| c.pin_name == "JTAG_CPU_TMS"));
        assert!(!configs.iter().any(|c| c.pin_name == "UART2_TX"));
    }

    #[test]
    fn test_parse_board_init_no_marker_returns_empty() {
        // 没有生成块标记的文件 (全部手写) 不应解析出任何配置
        let content =
            "int cvi_board_init(void) {\n\tPINMUX_CONFIG(UART2_TX, UART2_TX);\n\treturn 0;\n}";
        let configs = parse_board_init(content);
        assert!(configs.is_empty());
    }

    #[test]
    fn test_parse_board_init_empty() {
        let configs = parse_board_init("int cvi_board_init(void) { return 0; }");
        assert!(configs.is_empty());
    }

    #[test]
    fn test_parse_board_init_dedup() {
        let content = "\t// Generated PINMUX configurations\n\tPINMUX_CONFIG(UART0_TX, UART0_TX);\n\tPINMUX_CONFIG(UART0_TX, UART0_TX);\n\treturn 0;\n";
        let configs = parse_board_init(content);
        assert_eq!(configs.len(), 1);
    }

    #[test]
    fn test_generate_then_parse_roundtrip() {
        // 生成的非 GPIO 引脚配置应能被完整解析回来
        let pin_configs = vec![
            PinConfig {
                pin_name: "PAD_MIPI_TXM4".to_string(),
                function: "VI0_D_15".to_string(),
                user_configured: true,
                state: None,
            },
            PinConfig {
                pin_name: "UART0_TX".to_string(),
                function: "UART0_TX".to_string(),
                user_configured: true,
                state: None,
            },
        ];
        let code = generate_code("cv1842hp", &pin_configs, None).unwrap();
        let parsed = parse_board_init(&code);
        assert_eq!(parsed.len(), 2);
        assert!(parsed
            .iter()
            .any(|c| c.pin_name == "PAD_MIPI_TXM4" && c.function == "VI0_D_15"));
        assert!(parsed
            .iter()
            .any(|c| c.pin_name == "UART0_TX" && c.function == "UART0_TX"));
    }

    #[test]
    fn test_update_then_parse_roundtrip_preserves_handwritten() {
        // 模拟用户提供的文件: 含 #if 0 禁用块 + 手写行，再增量写入配置后回读
        let dir = std::env::temp_dir();
        let test_file = dir.join("test_cvi_board_init_roundtrip.c");
        let content = "int cvi_board_init(void)\n{\n#if 0 /* pinmux set in alios */\n\tPINMUX_CONFIG(CAM_MCLK0, CAM_MCLK0);\n\tPINMUX_CONFIG(IIC2_SCL, IIC2_SCL);\n#endif\n\n\tPINMUX_CONFIG(JTAG_CPU_TMS, UART1_TX);\n\tPINMUX_CONFIG(UART2_TX, UART2_TX);\n\tmmio_setbits_32(0x030002d0, 1 << 9);\n\n\treturn 0;\n}\n";
        fs::write(&test_file, content).unwrap();

        let pin_configs = vec![
            PinConfig {
                pin_name: "PAD_MIPIRX0N".to_string(),
                function: "IIC1_SCL".to_string(),
                user_configured: true,
                state: None,
            },
            PinConfig {
                pin_name: "PAD_MIPI_TXM4".to_string(),
                function: "IIC1_SDA".to_string(),
                user_configured: true,
                state: None,
            },
        ];

        update_existing_code(test_file.to_str().unwrap(), &pin_configs).unwrap();
        let written = fs::read_to_string(&test_file).unwrap();

        // 手写行 / 禁用块只应出现一次 (不被重复写入生成块)
        assert_eq!(
            written.matches("PINMUX_CONFIG(UART2_TX, UART2_TX)").count(),
            1
        );
        assert_eq!(
            written
                .matches("PINMUX_CONFIG(JTAG_CPU_TMS, UART1_TX)")
                .count(),
            1
        );
        assert_eq!(
            written
                .matches("PINMUX_CONFIG(CAM_MCLK0, CAM_MCLK0)")
                .count(),
            1
        );

        // 回读只得到工具生成的两个引脚
        let parsed = parse_board_init(&written);
        assert_eq!(parsed.len(), 2);
        assert!(parsed.iter().any(|c| c.pin_name == "PAD_MIPIRX0N"));
        assert!(parsed.iter().any(|c| c.pin_name == "PAD_MIPI_TXM4"));

        fs::remove_file(&test_file).ok();
    }

    // ---- 二级 mux (MUX_SPI1_*) ----

    #[test]
    fn test_generate_code_mux_two_lines() {
        // 选择 MUX_SPI1_MISO 并指定二级功能 PWM_9，应生成相邻两行
        let pin_configs = vec![PinConfig {
            pin_name: "PAD_MIPIRX3N".to_string(),
            function: "MUX_SPI1_MISO".to_string(),
            user_configured: true,
            state: Some("PWM_9".to_string()),
        }];
        let code = generate_code("cv1842hp", &pin_configs, None).unwrap();
        assert!(code.contains("PINMUX_CONFIG(PAD_MIPIRX3N, MUX_SPI1_MISO)"));
        assert!(code.contains("PINMUX_CONFIG(MUX_SPI1_MISO, PWM_9)"));
        // 两行相邻: 一级行紧接着二级行
        let one = code
            .find("PINMUX_CONFIG(PAD_MIPIRX3N, MUX_SPI1_MISO)")
            .unwrap();
        let two = code.find("PINMUX_CONFIG(MUX_SPI1_MISO, PWM_9)").unwrap();
        assert!(two > one);
    }

    #[test]
    fn test_generate_code_mux_defaults_state() {
        // 未指定 state 时，二级功能回落到该 mux 的默认值
        let pin_configs = vec![PinConfig {
            pin_name: "PAD_MIPIRX3N".to_string(),
            function: "MUX_SPI1_MISO".to_string(),
            user_configured: true,
            state: None,
        }];
        let code = generate_code("cv1842hp", &pin_configs, None).unwrap();
        assert!(code.contains("PINMUX_CONFIG(MUX_SPI1_MISO, XGPIOB_8)"));
    }

    #[test]
    fn test_generate_then_parse_mux_roundtrip() {
        let pin_configs = vec![PinConfig {
            pin_name: "PAD_MIPIRX3N".to_string(),
            function: "MUX_SPI1_MISO".to_string(),
            user_configured: true,
            state: Some("PWM_9".to_string()),
        }];
        let code = generate_code("cv1842hp", &pin_configs, None).unwrap();
        let parsed = parse_board_init(&code);
        // 二级行不应被当成独立引脚: 只解析出 1 个引脚
        assert_eq!(parsed.len(), 1);
        let p = &parsed[0];
        assert_eq!(p.pin_name, "PAD_MIPIRX3N");
        assert_eq!(p.function, "MUX_SPI1_MISO");
        assert_eq!(p.state.as_deref(), Some("PWM_9"));
    }

    #[test]
    fn test_update_existing_code_mux_two_lines() {
        let dir = std::env::temp_dir();
        let test_file = dir.join("test_cvi_board_init_mux.c");
        let content = "int cvi_board_init(void)\n{\n\treturn 0;\n}\n";
        fs::write(&test_file, content).unwrap();

        let pin_configs = vec![PinConfig {
            pin_name: "PAD_MIPIRX3N".to_string(),
            function: "MUX_SPI1_MISO".to_string(),
            user_configured: true,
            state: Some("PWM_9".to_string()),
        }];

        update_existing_code(test_file.to_str().unwrap(), &pin_configs).unwrap();
        let written = fs::read_to_string(&test_file).unwrap();
        assert!(written.contains("PINMUX_CONFIG(PAD_MIPIRX3N, MUX_SPI1_MISO)"));
        assert!(written.contains("PINMUX_CONFIG(MUX_SPI1_MISO, PWM_9)"));

        let parsed = parse_board_init(&written);
        assert_eq!(parsed.len(), 1);
        assert_eq!(parsed[0].state.as_deref(), Some("PWM_9"));

        fs::remove_file(&test_file).ok();
    }
}
