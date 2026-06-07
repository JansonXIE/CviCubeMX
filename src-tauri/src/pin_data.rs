/// 引脚数据模块 (对应重构计划 M2)
///
/// 从 pinfunction.cpp 提取的引脚功能数据，通过 include_str! 嵌入 JSON。
/// 提供 Tauri commands: load_pin_data, set_pin_function
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::sync::Mutex;

use crate::chip_spec;
use crate::pin_data_tool;

/// 引脚信息结构
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PinInfo {
    /// 引脚编号 (BGA: "A2", QFN: "1")
    pub pin_num: String,
    /// PAD 名称 (已清理, 如 "PAD_MIPI_TXM4")
    pub pin_name: String,
    /// 显示名称 (与 pin_num 保持一致，如 "A2" 或 "1")
    pub display_name: String,
    /// 支持的功能列表
    pub supported_functions: Vec<String>,
    /// 默认功能
    pub default_function: String,
    /// 当前功能 (初始 = default_function)
    pub current_function: String,
    /// 是否已被用户配置 (初始 = false)
    pub user_configured: bool,
}

/// JSON 数据中的原始引脚记录 (不含运行时字段)
#[derive(Debug, Clone, Serialize, Deserialize)]
struct PinDataRaw {
    pin_num: String,
    pin_name: String,
    supported_functions: Vec<String>,
    default_function: String,
}

/// 嵌入 pin_data.json
const PIN_DATA_JSON: &str = include_str!("../pin_data.json");

/// 全局用户配置存储
/// key = (chip_type, pin_name), value = user-selected function
static USER_CONFIG: std::sync::LazyLock<
    Mutex<HashMap<(String, String), (String, Option<String>)>>,
> = std::sync::LazyLock::new(|| Mutex::new(HashMap::new()));

/// 解析嵌入的 JSON 数据
fn load_raw_pin_data() -> Result<Vec<PinDataRaw>, String> {
    serde_json::from_str(PIN_DATA_JSON).map_err(|e| format!("Failed to parse pin_data.json: {}", e))
}

/// 加载指定芯片类型的引脚数据
///
/// 根据 package 类型 (QFN/BGA) 返回对应的引脚列表:
/// - BGA: 返回字母+数字编号的引脚 (排除四角)，不足的位置用 basicFunctions 补充
/// - QFN: 返回纯数字编号的引脚 (1-pin_count)，不足的用 basicFunctions 补充
#[tauri::command]
pub fn load_pin_data(chip_type: String) -> Result<Vec<PinInfo>, String> {
    let spec = chip_spec::get_chip_spec(&chip_type)?;
    let raw_data = load_raw_pin_data()?;
    let user_config = USER_CONFIG.lock().unwrap();

    // 从 raw data 中查找匹配的引脚
    let mut result: Vec<PinInfo> = Vec::new();

    if spec.package == "BGA" {
        // BGA: 返回字母+数字编号的引脚 (排除四角)
        let rows = spec.rows.as_ref().ok_or("BGA chip missing rows")?;
        let cols = spec.cols.ok_or("BGA chip missing cols")?;

        // 动态计算四角: 第一行首列、第一行末列、最后一行首列、最后一行末列
        let first_row = rows.chars().next().unwrap();
        let last_row = rows.chars().last().unwrap();
        let corner_exclusions = [
            format!("{}{}", first_row, 1),
            format!("{}{}", first_row, cols),
            format!("{}{}", last_row, 1),
            format!("{}{}", last_row, cols),
        ];

        // 从 JSON 中查找 BGA 引脚 (pin_num 包含字母且不在四角)
        let bga_pins: HashMap<String, PinDataRaw> = raw_data
            .iter()
            .filter(|p| {
                p.pin_num.chars().any(|c| c.is_alphabetic())
                    && !corner_exclusions.contains(&p.pin_num)
            })
            .map(|p| (p.pin_num.clone(), p.clone()))
            .collect();

        // 为所有非四角位置生成 PinInfo
        for (_row_idx, row_char) in rows.chars().enumerate() {
            for col in 1..=cols {
                let pin_num = format!("{}{}", row_char, col);

                // 跳过四角
                if corner_exclusions.contains(&pin_num) {
                    continue;
                }

                if let Some(raw) = bga_pins.get(&pin_num) {
                    // 有硬编码数据
                    let (current, user_configured) = user_config
                        .get(&(chip_type.clone(), raw.pin_name.clone()))
                        .map(|(func, _state)| (func.clone(), true))
                        .unwrap_or_else(|| (raw.default_function.clone(), false));

                    result.push(PinInfo {
                        pin_num: raw.pin_num.clone(),
                        pin_name: raw.pin_name.clone(),
                        display_name: raw.pin_num.clone(),
                        supported_functions: raw.supported_functions.clone(),
                        default_function: raw.default_function.clone(),
                        current_function: current,
                        user_configured,
                    });
                }
            }
        }
    } else {
        // QFN: 返回纯数字编号的引脚 (1-pin_count)
        let qfn_pins: HashMap<String, PinDataRaw> = raw_data
            .iter()
            .filter(|p| p.pin_num.chars().all(|c| c.is_ascii_digit()))
            .map(|p| (p.pin_num.clone(), p.clone()))
            .collect();

        for i in 1..=spec.pin_count {
            let pin_num = i.to_string();

            if let Some(raw) = qfn_pins.get(&pin_num) {
                let (current, user_configured) = user_config
                    .get(&(chip_type.clone(), raw.pin_name.clone()))
                    .map(|(func, _state)| (func.clone(), true))
                    .unwrap_or_else(|| (raw.default_function.clone(), false));

                result.push(PinInfo {
                    pin_num: raw.pin_num.clone(),
                    pin_name: raw.pin_name.clone(),
                    display_name: raw.pin_num.clone(),
                    supported_functions: raw.supported_functions.clone(),
                    default_function: raw.default_function.clone(),
                    current_function: current,
                    user_configured,
                });
            }
        }
    }

    Ok(result)
}

/// 设置引脚功能 (用户配置)
///
/// 对应 C++ chipconfig.cpp: setPinFunction()
#[tauri::command]
pub fn set_pin_function(
    chip_type: String,
    pin_name: String,
    function: String,
    state: Option<String>,
) -> Result<(), String> {
    let mut config = USER_CONFIG.lock().unwrap();
    config.insert((chip_type, pin_name), (function, state));
    Ok(())
}

/// 清除所有用户配置
#[tauri::command]
pub fn clear_pin_functions(chip_type: String) -> Result<(), String> {
    let mut config = USER_CONFIG.lock().unwrap();
    config.retain(|(ct, _), _| ct != &chip_type);
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_load_raw_pin_data() {
        let data = load_raw_pin_data().unwrap();
        assert_eq!(data.len(), 110); // 从 pinfunction.cpp 提取的唯一引脚
    }

    #[test]
    fn test_pad_mipi_txm4_data() {
        let data = load_raw_pin_data().unwrap();
        let pin = data.iter().find(|p| p.pin_name == "PAD_MIPI_TXM4").unwrap();
        assert_eq!(pin.supported_functions.len(), 8);
        assert_eq!(pin.default_function, "XGPIOC_18");
        assert!(pin.supported_functions.contains(&"VI0_D_15".to_string()));
        assert!(pin.supported_functions.contains(&"SD1_CLK".to_string()));
        assert!(pin.supported_functions.contains(&"XGPIOC_18".to_string()));
    }

    #[test]
    fn test_rstn_data() {
        let data = load_raw_pin_data().unwrap();
        let pin = data.iter().find(|p| p.pin_name == "RSTN").unwrap();
        assert_eq!(pin.supported_functions.len(), 1);
        assert_eq!(pin.default_function, "RSTN");
    }

    #[test]
    fn test_pwr_vbat_det_data() {
        let data = load_raw_pin_data().unwrap();
        let pin = data.iter().find(|p| p.pin_name == "PWR_VBAT_DET").unwrap();
        assert_eq!(pin.supported_functions.len(), 1);
        assert_eq!(pin.default_function, "PWR_VBAT_DET");
    }

    #[test]
    fn test_usb_vbus_det_data() {
        let data = load_raw_pin_data().unwrap();
        let pin = data.iter().find(|p| p.pin_name == "USB_VBUS_DET").unwrap();
        assert_eq!(pin.supported_functions[0], "USB_VBUS_DET");
        assert_eq!(pin.default_function, "USB_VBUS_DET");
    }

    #[test]
    fn test_sd0_d1_data() {
        let data = load_raw_pin_data().unwrap();
        let pin = data.iter().find(|p| p.pin_name == "SD0_D1").unwrap();
        assert_eq!(pin.default_function, "SDIO0_D_1");
    }

    #[test]
    fn test_cam_mclk0_data() {
        let data = load_raw_pin_data().unwrap();
        let pin = data.iter().find(|p| p.pin_name == "CAM_MCLK0").unwrap();
        assert_eq!(pin.default_function, "XGPIOA_0");
    }

    #[test]
    fn test_uart0_tx_data() {
        let data = load_raw_pin_data().unwrap();
        let pin = data.iter().find(|p| p.pin_name == "UART0_TX").unwrap();
        assert_eq!(pin.default_function, "UART0_TX");
    }

    #[test]
    fn test_pad_eth_rxm_cleaned() {
        let data = load_raw_pin_data().unwrap();
        // PAD_ETH_RXM 是从 PAD_ETH_RXM___EPHY_TXP 清理而来
        let pin = data.iter().find(|p| p.pin_name == "PAD_ETH_RXM").unwrap();
        assert_eq!(pin.default_function, "XGPIOB_26");
    }

    #[test]
    fn test_cv1842hp_pin_count() {
        // cv1842hp 的实际映射引脚少于物理最大引脚数 221
        let result = load_pin_data("cv1842hp".to_string()).unwrap();
        assert!(result.len() > 30);
    }

    #[test]
    fn test_cv1842hp_no_corner_pins() {
        let result = load_pin_data("cv1842hp".to_string()).unwrap();
        let corner_nums: Vec<&str> = result.iter().map(|p| p.pin_num.as_str()).collect();
        assert!(!corner_nums.contains(&"A1"));
        assert!(!corner_nums.contains(&"A15"));
        assert!(!corner_nums.contains(&"R1"));
        assert!(!corner_nums.contains(&"R15"));
    }

    #[test]
    fn test_cv1842cp_pin_count() {
        // cv1842cp 的实际映射引脚少于物理引脚数 88
        let result = load_pin_data("cv1842cp".to_string()).unwrap();
        assert!(result.len() > 50);
    }

    #[test]
    fn test_cv1801c_pin_count() {
        // cv1801c 的实际映射引脚少于物理引脚数 64
        let result = load_pin_data("cv1801c".to_string()).unwrap();
        assert!(result.len() > 30);
    }

    #[test]
    fn test_set_and_get_pin_function() {
        set_pin_function(
            "cv1842hp".to_string(),
            "PAD_MIPI_TXM4".to_string(),
            "UART0_TX".to_string(),
            None,
        )
        .unwrap();

        let result = load_pin_data("cv1842hp".to_string()).unwrap();
        let pin = result
            .iter()
            .find(|p| p.pin_name == "PAD_MIPI_TXM4")
            .unwrap();
        assert_eq!(pin.current_function, "UART0_TX");
        assert!(pin.user_configured);

        // 清除
        clear_pin_functions("cv1842hp".to_string()).unwrap();
        let result = load_pin_data("cv1842hp".to_string()).unwrap();
        let pin = result
            .iter()
            .find(|p| p.pin_name == "PAD_MIPI_TXM4")
            .unwrap();
        assert_eq!(pin.current_function, "XGPIOC_18");
        assert!(!pin.user_configured);
    }

    #[test]
    fn test_unknown_chip_type() {
        let result = load_pin_data("unknown".to_string());
        assert!(result.is_err());
    }
}
