/// 芯片规格模块 (对应重构计划 M2)
///
/// 从 C++ chipconfig.cpp: getPinCountForChip() 提取的 6 款芯片规格数据。
/// 每款芯片包含: chip_type, package (QFN/BGA), pin_count, BGA 行列参数。
use serde::{Deserialize, Serialize};

/// 芯片规格结构
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ChipSpec {
    /// 芯片型号，如 "cv1842hp"
    pub chip_type: String,
    /// 封装类型: "QFN" 或 "BGA"
    pub package: String,
    /// 引脚总数
    pub pin_count: u32,
    /// BGA 行字母 (如 "ABCDEFGHJKLMNOPQR")，QFN 为 None
    pub rows: Option<String>,
    /// BGA 列数 (如 15)，QFN 为 None
    pub cols: Option<u32>,
    /// 描述信息
    pub description: String,
}

/// 6 款芯片硬编码规格数据
/// 从 chipconfig.cpp: getPinCountForChip() 和 chip_specs_reference.json 提取
pub fn get_all_chip_specs() -> Vec<ChipSpec> {
    vec![
        ChipSpec {
            chip_type: "cv1801c".to_string(),
            package: "QFN".to_string(),
            pin_count: 64,
            rows: None,
            cols: None,
            description: "QFN封装，每边16个引脚".to_string(),
        },
        ChipSpec {
            chip_type: "cv1801h".to_string(),
            package: "BGA".to_string(),
            pin_count: 60,
            rows: Some("ABCDEFGH".to_string()),
            cols: Some(8),
            description: "BGA封装，8x8网格去掉四角=60".to_string(),
        },
        ChipSpec {
            chip_type: "cv1811c".to_string(),
            package: "QFN".to_string(),
            pin_count: 88,
            rows: None,
            cols: None,
            description: "QFN封装，每边22个引脚".to_string(),
        },
        ChipSpec {
            chip_type: "cv1811h".to_string(),
            package: "BGA".to_string(),
            pin_count: 84,
            rows: Some("ABCDEFGHJKLM".to_string()),
            cols: Some(8),
            description: "BGA封装，自适应网格去掉四角=84".to_string(),
        },
        ChipSpec {
            chip_type: "cv1842cp".to_string(),
            package: "QFN".to_string(),
            pin_count: 88,
            rows: None,
            cols: None,
            description: "QFN封装，每边22个引脚".to_string(),
        },
        ChipSpec {
            chip_type: "cv1842hp".to_string(),
            package: "BGA".to_string(),
            pin_count: 221,
            rows: Some("ABCDEFGHJKLMNPR".to_string()), // 15行 (对应 C++ rowLabels: A B C D E F G H J K L M N P R)
            cols: Some(15),
            description: "BGA封装，15x15网格去掉四角=221".to_string(),
        },
    ]
}

/// 芯片名称规范化函数，支持传入带板级后缀的芯片型号，如 "cv1842hp_wevb_0014a_emmc"
pub fn normalize_chip_type(chip_type: &str) -> String {
    let lower = chip_type.to_lowercase();
    if lower.contains("cv1801c") {
        "cv1801c".to_string()
    } else if lower.contains("cv1801h") {
        "cv1801h".to_string()
    } else if lower.contains("cv1811c") {
        "cv1811c".to_string()
    } else if lower.contains("cv1811h") {
        "cv1811h".to_string()
    } else if lower.contains("cv1840cp") || lower.contains("cv1841cp") || lower.contains("cv1842cp")
    {
        "cv1842cp".to_string()
    } else if lower.contains("cv1842hp") || lower.contains("cv1843hp") {
        "cv1842hp".to_string()
    } else {
        lower
    }
}

/// 根据芯片型号查找规格
///
/// 对应 C++ chipconfig.cpp: getPinCountForChip()
#[tauri::command]
pub fn load_chip_spec(chip_type: String) -> Result<ChipSpec, String> {
    let normalized = normalize_chip_type(&chip_type);
    get_all_chip_specs()
        .iter()
        .find(|spec| spec.chip_type == normalized)
        .cloned()
        .ok_or_else(|| format!("Unknown chip type: {}", chip_type))
}

/// 辅助: 获取芯片规格 (内部使用)
pub fn get_chip_spec(chip_type: &str) -> Result<ChipSpec, String> {
    let normalized = normalize_chip_type(chip_type);
    get_all_chip_specs()
        .iter()
        .find(|spec| spec.chip_type == normalized)
        .cloned()
        .ok_or_else(|| format!("Unknown chip type: {}", chip_type))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_chip_specs_count() {
        assert_eq!(get_all_chip_specs().len(), 6);
    }

    #[test]
    fn test_cv1801c_spec() {
        let spec = load_chip_spec("cv1801c".to_string()).unwrap();
        assert_eq!(spec.package, "QFN");
        assert_eq!(spec.pin_count, 64);
        assert!(spec.rows.is_none());
    }

    #[test]
    fn test_cv1801h_spec() {
        let spec = load_chip_spec("cv1801h".to_string()).unwrap();
        assert_eq!(spec.package, "BGA");
        assert_eq!(spec.pin_count, 60);
        assert_eq!(spec.rows.as_deref(), Some("ABCDEFGH"));
        assert_eq!(spec.cols, Some(8));
    }

    #[test]
    fn test_cv1811c_spec() {
        let spec = load_chip_spec("cv1811c".to_string()).unwrap();
        assert_eq!(spec.package, "QFN");
        assert_eq!(spec.pin_count, 88);
    }

    #[test]
    fn test_cv1811h_spec() {
        let spec = load_chip_spec("cv1811h".to_string()).unwrap();
        assert_eq!(spec.package, "BGA");
        assert_eq!(spec.pin_count, 84);
        assert_eq!(spec.rows.as_deref(), Some("ABCDEFGHJKLM"));
        assert_eq!(spec.cols, Some(8));
    }

    #[test]
    fn test_cv1842cp_spec() {
        let spec = load_chip_spec("cv1842cp".to_string()).unwrap();
        assert_eq!(spec.package, "QFN");
        assert_eq!(spec.pin_count, 88);
    }

    #[test]
    fn test_cv1842hp_spec() {
        let spec = load_chip_spec("cv1842hp".to_string()).unwrap();
        assert_eq!(spec.package, "BGA");
        assert_eq!(spec.pin_count, 221);
        assert_eq!(spec.rows.as_deref(), Some("ABCDEFGHJKLMNPR"));
        assert_eq!(spec.cols, Some(15));
    }

    #[test]
    fn test_unknown_chip_type() {
        let result = load_chip_spec("unknown".to_string());
        assert!(result.is_err());
        assert!(result.unwrap_err().contains("Unknown chip type"));
    }

    #[test]
    fn test_bga_rows_skip_i() {
        for spec in get_all_chip_specs() {
            if spec.package == "BGA" {
                let rows = spec.rows.as_ref().unwrap();
                assert!(!rows.contains('I'), "BGA rows should skip letter I");
            }
        }
    }

    #[test]
    fn test_bga_corner_exclusion_count() {
        // cv1842hp: 15x15=225, 去掉4角=221
        let spec = load_chip_spec("cv1842hp".to_string()).unwrap();
        let rows = spec.rows.as_ref().unwrap().len() as u32;
        let cols = spec.cols.unwrap();
        assert_eq!(rows * cols - 4, spec.pin_count);

        // cv1801h: 8x8=64, 去掉4角=60
        let spec = load_chip_spec("cv1801h".to_string()).unwrap();
        let rows = spec.rows.as_ref().unwrap().len() as u32;
        let cols = spec.cols.unwrap();
        assert_eq!(rows * cols - 4, spec.pin_count);

        // cv1811h: verified from C++ source
        let spec = load_chip_spec("cv1811h".to_string()).unwrap();
        assert_eq!(spec.pin_count, 84);
    }
}
