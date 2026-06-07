/// 引脚数据工具模块 (对应重构计划 M9)
///
/// 从 generate_pins.py 和 function_name_remap.json 提取的工具逻辑:
/// - FUNCTION_NAME_REMAP: 8 条功能名称重映射规则
/// - clean_pin_name: Pin Name 中 ___ 分割清理
/// - parse_function_select_cell: Description 列解析
/// - pin_sort_key: 引脚自然排序
/// - BGA 四角剔除

use std::collections::HashMap;

/// 功能名称重映射规则 (8条)
/// 从 generate_pins.py: FUNCTION_NAME_REMAP 提取
pub fn get_function_name_remap() -> HashMap<&'static str, &'static str> {
    let mut map = HashMap::new();
    map.insert("CR_4WTMS", "CV_2WTMS_CR_4WTMS");
    map.insert("CR_4WTCK", "CV_2WTCK_CR_2WTCK");
    map.insert("CR_4WTDI", "CV_SCL0__CR_4WTDI");
    map.insert("CR_4WTDO", "CV_SDA0__CR_4WTDO");
    map.insert("CR_SCL0", "CV_4WTDI_CR_SCL0");
    map.insert("CR_SDA0", "CV_4WTMS_CR_SDA0");
    map.insert("CR_2WTMS", "CV_4WTDO_CR_2WTMS");
    map.insert("CR_2WTCK", "CV_4WTCK_CR_2WTCK");
    map
}

/// Pin Name 清理逻辑
/// Pin Name 含 ___ 时只取第一段
/// 例: "PAD_ETH_RXM___EPHY_TXP" → "PAD_ETH_RXM"
pub fn clean_pin_name(pin_name: &str) -> &str {
    pin_name.split("___").next().unwrap_or(pin_name)
}

/// Description 列解析
/// 从 generate_pins.py: parse_function_select_cell 提取
///
/// 解析格式: "index : function_name [(default)]"
/// - 有 (default) 标记 → 取该功能为默认
/// - 无标记 → 取第一个含 XGPIO 的功能作为默认; 否则取第一个
/// - 应用 FUNCTION_NAME_REMAP 重映射
/// - 括号 [ → _, ] → 空字符串
/// - 空内容返回 ("GPIO", ["GPIO"])
pub fn parse_function_select_cell(content: &str) -> (Vec<String>, String) {
    let remap = get_function_name_remap();
    let mut functions: Vec<String> = Vec::new();
    let mut default_function = "GPIO".to_string();
    let mut found_default = false;

    if content.is_empty() {
        return (vec!["GPIO".to_string()], "GPIO".to_string());
    }

    for line in content.lines() {
        let trimmed = line.trim();
        // 跳过空行、标题行和 "Others" 行
        if trimmed.is_empty()
            || trimmed.contains("function select")
            || trimmed.contains("Others :")
        {
            continue;
        }

        if trimmed.contains(':') {
            let parts: Vec<&str> = trimmed.splitn(2, ':').collect();
            if parts.len() == 2 {
                let func_info = parts[1].trim();

                if func_info.contains("(default)") {
                    // 有 (default) 标记
                    let func_name = func_info
                        .replace("(default)", "")
                        .trim()
                        .replace('[', "_")
                        .replace(']', "");
                    let func_name = apply_remap(&func_name, &remap);
                    default_function = func_name.clone();
                    functions.push(func_name);
                    found_default = true;
                } else {
                    // 非 default 功能
                    let func_name = func_info
                        .trim()
                        .replace('[', "_")
                        .replace(']', "");
                    let func_name = apply_remap(&func_name, &remap);
                    if !func_name.is_empty() {
                        functions.push(func_name);
                    }
                }
            }
        }
    }

    // 无 (default) 标记时的默认功能查找逻辑
    if !found_default && !functions.is_empty() {
        let gpio_fallback: Vec<&String> = functions.iter().filter(|f| f.contains("XGPIO")).collect();
        if !gpio_fallback.is_empty() {
            default_function = gpio_fallback[0].clone();
        } else {
            default_function = functions[0].clone();
        }
    }

    // 空列表回退
    if functions.is_empty() {
        functions.push("GPIO".to_string());
        default_function = "GPIO".to_string();
    }

    // 确保默认功能在列表中
    if !functions.contains(&default_function) {
        functions.push(default_function.clone());
    }

    (functions, default_function)
}

/// 应用 FUNCTION_NAME_REMAP 重映射
fn apply_remap(func_name: &str, remap: &HashMap<&str, &'static str>) -> String {
    remap.get(func_name).map(|v| v.to_string()).unwrap_or_else(|| func_name.to_string())
}

/// BGA 四角排除引脚 (针对 C++ 通用循环的 17x15 网格)
/// 实际使用中，四角由 ChipSpec 的 rows/cols 动态计算
pub const BGA_CORNER_EXCLUSIONS: &[&str] = &["A1", "A15", "R1", "R15"];

/// BGA 行字母 (对应 C++ mainwindow.cpp 的 rowLabels)
pub const BGA_ROWS_CV1842HP: &str = "ABCDEFGHJKLMNPR"; // 15行 (A-R跳过I,Q,S..)

/// 判断是否为 BGA 四角引脚
pub fn is_bga_corner(pin_num: &str) -> bool {
    BGA_CORNER_EXCLUSIONS.contains(&pin_num)
}

/// 引脚自然排序键
/// "A2" → ("A", 2), "A10" → ("A", 10)
/// 确保自然排序: A2 < A10 (不是字典序 A10 < A2)
/// 纯数字引脚: "1" → ("", 1), "10" → ("", 10)
pub fn pin_sort_key(pin_num: &str) -> (String, u32) {
    // 匹配字母+数字格式
    if let Some(letter_end) = pin_num.chars().position(|c| c.is_ascii_digit()) {
        if letter_end > 0 {
            let letter_part = &pin_num[..letter_end];
            let number_part = &pin_num[letter_end..];
            let num: u32 = number_part.parse().unwrap_or(0);
            return (letter_part.to_string(), num);
        }
    }

    // 纯数字引脚
    let num: u32 = pin_num.parse().unwrap_or(0);
    ("".to_string(), num)
}

/// basicFunctions 列表 (用于补充没有硬编码数据的引脚)
pub const BASIC_FUNCTIONS: &[&str] = &["GPIO", "ADC", "PWM", "I2C", "UART", "SPI"];

#[cfg(test)]
mod tests {
    use super::*;

    // === FUNCTION_NAME_REMAP 测试 ===

    #[test]
    fn test_remap_has_8_rules() {
        let remap = get_function_name_remap();
        assert_eq!(remap.len(), 8);
    }

    #[test]
    fn test_remap_cr_4wtms() {
        let remap = get_function_name_remap();
        assert_eq!(remap.get("CR_4WTMS").copied(), Some("CV_2WTMS_CR_4WTMS"));
    }

    #[test]
    fn test_remap_cr_4wtck() {
        let remap = get_function_name_remap();
        assert_eq!(remap.get("CR_4WTCK").copied(), Some("CV_2WTCK_CR_2WTCK"));
    }

    #[test]
    fn test_remap_cr_4wtdi() {
        let remap = get_function_name_remap();
        assert_eq!(remap.get("CR_4WTDI").copied(), Some("CV_SCL0__CR_4WTDI"));
    }

    #[test]
    fn test_remap_cr_4wtdo() {
        let remap = get_function_name_remap();
        assert_eq!(remap.get("CR_4WTDO").copied(), Some("CV_SDA0__CR_4WTDO"));
    }

    #[test]
    fn test_remap_cr_scl0() {
        let remap = get_function_name_remap();
        assert_eq!(remap.get("CR_SCL0").copied(), Some("CV_4WTDI_CR_SCL0"));
    }

    #[test]
    fn test_remap_cr_sda0() {
        let remap = get_function_name_remap();
        assert_eq!(remap.get("CR_SDA0").copied(), Some("CV_4WTMS_CR_SDA0"));
    }

    #[test]
    fn test_remap_cr_2wtms() {
        let remap = get_function_name_remap();
        assert_eq!(remap.get("CR_2WTMS").copied(), Some("CV_4WTDO_CR_2WTMS"));
    }

    #[test]
    fn test_remap_cr_2wtck() {
        let remap = get_function_name_remap();
        assert_eq!(remap.get("CR_2WTCK").copied(), Some("CV_4WTCK_CR_2WTCK"));
    }

    #[test]
    fn test_remap_unknown_key() {
        let remap = get_function_name_remap();
        assert_eq!(remap.get("XGPIOA_0"), None);
    }

    // === Pin Name 清理测试 ===

    #[test]
    fn test_clean_pin_name_with_underscore() {
        assert_eq!(clean_pin_name("PAD_ETH_RXM___EPHY_TXP"), "PAD_ETH_RXM");
    }

    #[test]
    fn test_clean_pin_name_without_underscore() {
        assert_eq!(clean_pin_name("PAD_MIPI_TXM4"), "PAD_MIPI_TXM4");
    }

    #[test]
    fn test_clean_pin_name_simple() {
        assert_eq!(clean_pin_name("RSTN"), "RSTN");
    }

    // === Description 解析测试 ===

    #[test]
    fn test_parse_default_function() {
        let content = "0 : UART0_TX (default)\n3 : XGPIOA_16";
        let (functions, default) = parse_function_select_cell(content);
        assert_eq!(default, "UART0_TX");
        assert!(functions.contains(&"UART0_TX".to_string()));
    }

    #[test]
    fn test_parse_multi_functions() {
        let content = "0 : VI0_D_15\n1 : SD1_CLK\n2 : VO_D_24\n3 : XGPIOC_18 (default)\n4 : CAM_MCLK1\n5 : PWM_12\n6 : IIC1_SDA\n7 : DBG_18";
        let (functions, default) = parse_function_select_cell(content);
        assert_eq!(functions.len(), 8);
        assert_eq!(default, "XGPIOC_18");
    }

    #[test]
    fn test_parse_empty_content() {
        let (functions, default) = parse_function_select_cell("");
        assert_eq!(functions, vec!["GPIO".to_string()]);
        assert_eq!(default, "GPIO");
    }

    #[test]
    fn test_parse_remap_in_default() {
        let content = "0 : CR_4WTMS (default)";
        let (functions, default) = parse_function_select_cell(content);
        assert_eq!(default, "CV_2WTMS_CR_4WTMS");
        assert!(functions.contains(&"CV_2WTMS_CR_4WTMS".to_string()));
    }

    #[test]
    fn test_parse_no_default_fallback_gpio() {
        let content = "0 : UART0_TX\n3 : XGPIOA_16";
        let (_functions, default) = parse_function_select_cell(content);
        // 无 (default) 标记, 取第一个含 XGPIO 的
        assert_eq!(default, "XGPIOA_16");
    }

    #[test]
    fn test_parse_no_default_no_gpio() {
        let content = "0 : CAM_MCLK0\n1 : AUX1";
        let (_functions, default) = parse_function_select_cell(content);
        // 无 (default), 无 XGPIO, 取第一个
        assert_eq!(default, "CAM_MCLK0");
    }

    // === 引脚自然排序测试 ===

    #[test]
    fn test_sort_a2_lt_a10() {
        let key_a2 = pin_sort_key("A2");
        let key_a10 = pin_sort_key("A10");
        assert!(key_a2 < key_a10); // ("A", 2) < ("A", 10)
    }

    #[test]
    fn test_sort_b1_lt_b10() {
        let key_b1 = pin_sort_key("B1");
        let key_b10 = pin_sort_key("B10");
        assert!(key_b1 < key_b10);
    }

    #[test]
    fn test_sort_a_lt_b() {
        let key_a2 = pin_sort_key("A2");
        let key_b1 = pin_sort_key("B1");
        assert!(key_a2 < key_b1); // ("A", 2) < ("B", 1)
    }

    #[test]
    fn test_sort_pure_numbers() {
        let key_1 = pin_sort_key("1");
        let key_10 = pin_sort_key("10");
        assert!(key_1 < key_10);
    }

    // === BGA 四角测试 ===

    #[test]
    fn test_bga_corner_exclusions_count() {
        assert_eq!(BGA_CORNER_EXCLUSIONS.len(), 4);
    }

    #[test]
    fn test_bga_corner_a1() {
        assert!(is_bga_corner("A1"));
    }

    #[test]
    fn test_bga_corner_a15() {
        assert!(is_bga_corner("A15"));
    }

    #[test]
    fn test_bga_corner_r1() {
        assert!(is_bga_corner("R1"));
    }

    #[test]
    fn test_bga_corner_r15() {
        assert!(is_bga_corner("R15"));
    }

    #[test]
    fn test_bga_not_corner_a2() {
        assert!(!is_bga_corner("A2"));
    }

    #[test]
    fn test_bga_rows_skip_i() {
        assert!(!BGA_ROWS_CV1842HP.contains('I'));
        assert_eq!(BGA_ROWS_CV1842HP.len(), 15); // 15行 (A-O, 跳过I)
    }
}