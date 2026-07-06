/// 二级引脚复用 (MUX) 数据模块
///
/// CV181x/SG200x 上部分引脚经 SPI1 二级 IO mux (MUX_SPI1_MISO/MOSI/CS/SCK) 后，
/// 还可再选择实际输出的功能 (function select 1..7)。本模块是这些二级 mux 的唯一数据源，
/// 供 codegen (生成/解析两行 PINMUX_CONFIG) 与前端 (渲染二级菜单) 共用。
use serde::Serialize;

/// 一个二级 mux 的定义
#[derive(Debug, Clone, Serialize)]
pub struct MuxDef {
    /// mux 名称 (如 "MUX_SPI1_MISO")，也是一级功能名
    pub name: String,
    /// 默认二级功能 (function select 3)
    pub default: String,
    /// 可选二级功能，按 function select 1..7 顺序排列
    pub options: Vec<String>,
}

fn def(name: &str, default: &str, options: &[&str]) -> MuxDef {
    MuxDef {
        name: name.to_string(),
        default: default.to_string(),
        options: options.iter().map(|s| s.to_string()).collect(),
    }
}

/// 全部二级 mux 定义
///
/// 方括号形式的功能按代码库惯例转下划线 (XGPIOB[8] -> XGPIOB_8)，
/// options 顺序即 function select 1..7，default 为第 3 项。
pub fn mux_definitions() -> Vec<MuxDef> {
    vec![
        def(
            "MUX_SPI1_MISO",
            "XGPIOB_8",
            &[
                "UART3_RTS",
                "IIC1_SDA",
                "XGPIOB_8",
                "PWM_9",
                "KEY_COL1",
                "SPI1_SDI",
                "DBG_14",
            ],
        ),
        def(
            "MUX_SPI1_MOSI",
            "XGPIOB_7",
            &[
                "UART3_RX", "IIC1_SCL", "XGPIOB_7", "PWM_8", "KEY_COL0", "SPI1_SDO", "DBG_13",
            ],
        ),
        def(
            "MUX_SPI1_CS",
            "XGPIOB_10",
            &[
                "UART3_CTS",
                "CAM_MCLK0",
                "XGPIOB_10",
                "PWM_11",
                "KEY_ROW3",
                "SPI1_CS_X",
                "DBG_16",
            ],
        ),
        def(
            "MUX_SPI1_SCK",
            "XGPIOB_9",
            &[
                "UART3_TX",
                "CAM_MCLK1",
                "XGPIOB_9",
                "PWM_10",
                "KEY_ROW2",
                "SPI1_SCK",
                "DBG_15",
            ],
        ),
    ]
}

/// 判断某个功能名是否为二级 mux 网关
pub fn is_mux_function(name: &str) -> bool {
    mux_definitions().iter().any(|m| m.name == name)
}

/// 返回某个二级 mux 的默认功能
pub fn mux_default(name: &str) -> Option<String> {
    mux_definitions()
        .into_iter()
        .find(|m| m.name == name)
        .map(|m| m.default)
}

/// Tauri 命令: 返回全部二级 mux 定义，供前端渲染二级功能菜单
#[tauri::command]
pub fn get_mux_functions() -> Vec<MuxDef> {
    mux_definitions()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_mux_definitions_count() {
        assert_eq!(mux_definitions().len(), 4);
    }

    #[test]
    fn test_each_mux_has_seven_options_with_default_at_index_2() {
        for m in mux_definitions() {
            assert_eq!(m.options.len(), 7, "{} should have 7 options", m.name);
            assert_eq!(
                m.options[2], m.default,
                "{} default should be function select 3",
                m.name
            );
        }
    }

    #[test]
    fn test_is_mux_function() {
        assert!(is_mux_function("MUX_SPI1_MISO"));
        assert!(is_mux_function("MUX_SPI1_SCK"));
        assert!(!is_mux_function("UART0_TX"));
        assert!(!is_mux_function("XGPIOB_8"));
    }

    #[test]
    fn test_mux_default() {
        assert_eq!(mux_default("MUX_SPI1_MISO").as_deref(), Some("XGPIOB_8"));
        assert_eq!(mux_default("MUX_SPI1_CS").as_deref(), Some("XGPIOB_10"));
        assert_eq!(mux_default("UART0_TX"), None);
    }

    #[test]
    fn test_mux_miso_options() {
        let m = mux_definitions()
            .into_iter()
            .find(|m| m.name == "MUX_SPI1_MISO")
            .unwrap();
        assert_eq!(
            m.options,
            vec![
                "UART3_RTS",
                "IIC1_SDA",
                "XGPIOB_8",
                "PWM_9",
                "KEY_COL1",
                "SPI1_SDI",
                "DBG_14"
            ]
        );
    }
}
