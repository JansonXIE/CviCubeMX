//! Peripheral information module.
//!
//! Defines the `PeripheralInfo` struct (mirroring the C++ version),
//! the `SysdmaChannelMap` helper for SYSDMA channel constant ↔ number mapping,
//! and the Tauri command handlers for DTS peripheral operations.

use serde::{Deserialize, Serialize};

use crate::dts_parser::DtsParser;
use crate::dts_writer::DtsWriter;

// ─── PeripheralInfo ──────────────────────────────────────────────

/// Information about a single peripheral node in a DTS file.
///
/// Mirrors the C++ `PeripheralInfo` struct from `dtsconfig.h`.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PeripheralInfo {
    /// Node name (e.g. "i2c0", "uart0", "sysdma_remap")
    pub name: String,

    /// Status: "okay", "disabled", or empty (defaults to okay)
    pub status: String,

    /// Clock name extracted from `CV184X_CLK_XXX` pattern
    pub clock_name: String,

    /// Raw clock-frequency string value
    pub clock_freq: String,

    /// Clock frequency numeric value (from `clock-frequency` property)
    pub clock_frequency: i32,

    /// PWM cells count (from `#pwm-cells` property, default 1)
    pub pwm_cells: i32,

    /// UART baud rate (from `current-speed` property, default 115200)
    pub current_speed: i32,

    /// SYSDMA channel mapping (from `ch-remap` property)
    pub sysdma_channels: Vec<String>,

    /// Whether the node has an explicit `status` property
    pub has_status: bool,

    /// Whether the node has a `clocks` property
    pub has_clock: bool,

    /// Whether the node has a `clock-frequency` property
    pub has_clock_freq: bool,

    /// Whether the node has a `#pwm-cells` property
    pub has_pwm_cells: bool,

    /// Whether the node has a `current-speed` property (UART only)
    pub has_current_speed: bool,

    /// Whether the node has a `ch-remap` property (SYSDMA only)
    pub has_sysdma_channels: bool,

    /// 1-based line number in the DTS file (used for line-level modification)
    pub line_number: i32,
}

impl Default for PeripheralInfo {
    fn default() -> Self {
        PeripheralInfo {
            name: String::new(),
            status: "okay".to_string(),
            clock_name: String::new(),
            clock_freq: String::new(),
            clock_frequency: 0,
            pwm_cells: 1,
            current_speed: 115200,
            sysdma_channels: Self::default_sysdma_channels(),
            has_status: false,
            has_clock: false,
            has_clock_freq: false,
            has_pwm_cells: false,
            has_current_speed: false,
            has_sysdma_channels: false,
            line_number: 0,
        }
    }
}

impl PeripheralInfo {
    /// Default SYSDMA channel mapping: ["0","5","12","13","42","42","4","7"]
    pub fn default_sysdma_channels() -> Vec<String> {
        vec!["0", "5", "12", "13", "42", "42", "4", "7"]
            .into_iter()
            .map(|s| s.to_string())
            .collect()
    }
}

// ─── SysdmaChannelMap ────────────────────────────────────────────

/// Helper for SYSDMA channel constant ↔ number conversions.
///
/// Mirrors the C++ `getChannelNumber()` / `getChannelName()` /
/// `getPeripheralNodeFromChannel()` / `isChannelRx()` functions.
pub struct SysdmaChannelMap;

impl SysdmaChannelMap {
    /// Convert a SYSDMA constant name (e.g. "CVI_I2S0_RX") to its channel number.
    pub fn channel_number(channel_name: &str) -> String {
        let map: &[(&str, &str)] = &[
            ("CVI_I2S0_RX", "0"),
            ("CVI_I2S0_TX", "1"),
            ("CVI_I2S1_RX", "2"),
            ("CVI_I2S1_TX", "3"),
            ("CVI_I2S2_RX", "4"),
            ("CVI_I2S2_TX", "5"),
            ("CVI_I2S3_RX", "6"),
            ("CVI_I2S3_TX", "7"),
            ("CVI_UART0_RX", "8"),
            ("CVI_UART0_TX", "9"),
            ("CVI_UART1_RX", "10"),
            ("CVI_UART1_TX", "11"),
            ("CVI_UART2_RX", "12"),
            ("CVI_UART2_TX", "13"),
            ("CVI_UART3_RX", "14"),
            ("CVI_UART3_TX", "15"),
            ("CVI_SPI0_RX", "16"),
            ("CVI_SPI0_TX", "17"),
            ("CVI_SPI1_RX", "18"),
            ("CVI_SPI1_TX", "19"),
            ("CVI_SPI2_RX", "20"),
            ("CVI_SPI2_TX", "21"),
            ("CVI_SPI3_RX", "22"),
            ("CVI_SPI3_TX", "23"),
            ("CVI_I2C0_RX", "24"),
            ("CVI_I2C0_TX", "25"),
            ("CVI_I2C1_RX", "26"),
            ("CVI_I2C1_TX", "27"),
            ("CVI_I2C2_RX", "28"),
            ("CVI_I2C2_TX", "29"),
            ("CVI_I2C3_RX", "30"),
            ("CVI_I2C3_TX", "31"),
            ("CVI_I2C4_RX", "32"),
            ("CVI_I2C4_TX", "33"),
            ("CVI_TDM0_RX", "34"),
            ("CVI_TDM0_TX", "35"),
            ("CVI_TDM1_RX", "36"),
            ("CVI_AUDSRC", "37"),
            ("CVI_SPI_NOR_RX", "38"),
            ("CVI_SPI_NOR_TX", "39"),
            ("CVI_UART4_RX", "40"),
            ("CVI_UART4_TX", "41"),
            ("CVI_SPI_NAND", "42"),
        ];
        for (name, num) in map {
            if channel_name == *name {
                return num.to_string();
            }
        }
        "0".to_string()
    }

    /// Convert a channel number to its SYSDMA constant name.
    pub fn channel_name(channel_number: &str) -> String {
        let map: &[(&str, &str)] = &[
            ("0", "CVI_I2S0_RX"),
            ("1", "CVI_I2S0_TX"),
            ("2", "CVI_I2S1_RX"),
            ("3", "CVI_I2S1_TX"),
            ("4", "CVI_I2S2_RX"),
            ("5", "CVI_I2S2_TX"),
            ("6", "CVI_I2S3_RX"),
            ("7", "CVI_I2S3_TX"),
            ("8", "CVI_UART0_RX"),
            ("9", "CVI_UART0_TX"),
            ("10", "CVI_UART1_RX"),
            ("11", "CVI_UART1_TX"),
            ("12", "CVI_UART2_RX"),
            ("13", "CVI_UART2_TX"),
            ("14", "CVI_UART3_RX"),
            ("15", "CVI_UART3_TX"),
            ("16", "CVI_SPI0_RX"),
            ("17", "CVI_SPI0_TX"),
            ("18", "CVI_SPI1_RX"),
            ("19", "CVI_SPI1_TX"),
            ("20", "CVI_SPI2_RX"),
            ("21", "CVI_SPI2_TX"),
            ("22", "CVI_SPI3_RX"),
            ("23", "CVI_SPI3_TX"),
            ("24", "CVI_I2C0_RX"),
            ("25", "CVI_I2C0_TX"),
            ("26", "CVI_I2C1_RX"),
            ("27", "CVI_I2C1_TX"),
            ("28", "CVI_I2C2_RX"),
            ("29", "CVI_I2C2_TX"),
            ("30", "CVI_I2C3_RX"),
            ("31", "CVI_I2C3_TX"),
            ("32", "CVI_I2C4_RX"),
            ("33", "CVI_I2C4_TX"),
            ("34", "CVI_TDM0_RX"),
            ("35", "CVI_TDM0_TX"),
            ("36", "CVI_TDM1_RX"),
            ("37", "CVI_AUDSRC"),
            ("38", "CVI_SPI_NOR_RX"),
            ("39", "CVI_SPI_NOR_TX"),
            ("40", "CVI_UART4_RX"),
            ("41", "CVI_UART4_TX"),
            ("42", "CVI_SPI_NAND"),
        ];
        for (num, name) in map {
            if channel_number == *num {
                return name.to_string();
            }
        }
        "CVI_I2S0_RX".to_string()
    }

    /// Determine the peripheral node name from a SYSDMA channel number.
    pub fn peripheral_node_from_channel(channel_number: &str) -> Option<String> {
        let ch = channel_number.parse::<i32>().unwrap_or(-1);
        if ch >= 8 && ch <= 15 {
            Some(format!("uart{}", (ch - 8) / 2))
        } else if ch >= 40 && ch <= 41 {
            Some("uart4".to_string())
        } else if ch >= 0 && ch <= 7 {
            Some(format!("i2s{}", ch / 2))
        } else if ch >= 16 && ch <= 23 {
            Some(format!("spi{}", (ch - 16) / 2))
        } else if ch >= 24 && ch <= 33 {
            Some(format!("i2c{}", (ch - 24) / 2))
        } else {
            None
        }
    }

    /// Check whether a SYSDMA channel number is a RX (vs TX) channel.
    pub fn is_channel_rx(channel_number: &str) -> bool {
        let ch = channel_number.parse::<i32>().unwrap_or(-1);
        if (ch >= 0 && ch <= 7)
            || (ch >= 8 && ch <= 15)
            || (ch >= 16 && ch <= 23)
            || (ch >= 24 && ch <= 33)
            || (ch >= 34 && ch <= 36)
            || (ch >= 38 && ch <= 41)
        {
            ch % 2 == 0
        } else {
            true // AUDSRC(37) and SPI_NAND(42) default to RX
        }
    }
}

// ─── Global state ────────────────────────────────────────────────

/// Global state held by the Tauri app.
///
/// Stores a single `DtsParser` instance that holds both the parsed
/// peripheral data and the raw file content (for writer modifications).
pub struct DtsState {
    pub parser: std::sync::Mutex<DtsParser>,
}

impl DtsState {
    pub fn new() -> Self {
        DtsState {
            parser: std::sync::Mutex::new(DtsParser::new()),
        }
    }
}

// ─── Tauri Commands ──────────────────────────────────────────────

/// Load a DTS file and return all parsed peripherals.
#[tauri::command]
pub fn load_dts_peripherals(
    state: tauri::State<'_, DtsState>,
    file_path: String,
) -> Result<Vec<PeripheralInfo>, String> {
    let mut parser = state.parser.lock().map_err(|e| e.to_string())?;
    parser.load_file(&file_path)?;
    let peripherals: Vec<PeripheralInfo> = parser.get_peripherals().values().cloned().collect();
    Ok(peripherals)
}

/// Set a peripheral's status ("okay" ↔ "disabled").
#[tauri::command]
pub fn set_peripheral_status(
    state: tauri::State<'_, DtsState>,
    peripheral: String,
    status: String,
) -> Result<(), String> {
    let mut parser = state.parser.lock().map_err(|e| e.to_string())?;
    DtsWriter::update_status(&mut parser, &peripheral, &status)?;

    // Write back to file
    if let Some(path) = parser.get_file_path() {
        let content = parser.get_file_content().to_string();
        std::fs::write(path, content)
            .map_err(|e| format!("无法写入文件: {}", e))?;
    }

    Ok(())
}

/// Set a peripheral's clock-frequency value.
#[tauri::command]
pub fn set_peripheral_clock_frequency(
    state: tauri::State<'_, DtsState>,
    peripheral: String,
    frequency: i32,
) -> Result<(), String> {
    let mut parser = state.parser.lock().map_err(|e| e.to_string())?;
    DtsWriter::update_clock_frequency(&mut parser, &peripheral, frequency)?;

    // Write back to file
    if let Some(path) = parser.get_file_path() {
        let content = parser.get_file_content().to_string();
        std::fs::write(path, content)
            .map_err(|e| format!("无法写入文件: {}", e))?;
    }

    Ok(())
}

/// Set a peripheral's #pwm-cells value.
#[tauri::command]
pub fn set_peripheral_pwm_cells(
    state: tauri::State<'_, DtsState>,
    peripheral: String,
    cells: i32,
) -> Result<(), String> {
    let mut parser = state.parser.lock().map_err(|e| e.to_string())?;
    DtsWriter::update_pwm_cells(&mut parser, &peripheral, cells)?;

    // Write back to file
    if let Some(path) = parser.get_file_path() {
        let content = parser.get_file_content().to_string();
        std::fs::write(path, content)
            .map_err(|e| format!("无法写入文件: {}", e))?;
    }

    Ok(())
}

/// Set a peripheral's UART current-speed (baud rate).
#[tauri::command]
pub fn set_peripheral_current_speed(
    state: tauri::State<'_, DtsState>,
    peripheral: String,
    speed: i32,
) -> Result<(), String> {
    let mut parser = state.parser.lock().map_err(|e| e.to_string())?;
    DtsWriter::update_current_speed(&mut parser, &peripheral, speed)?;

    // Write back to file
    if let Some(path) = parser.get_file_path() {
        let content = parser.get_file_content().to_string();
        std::fs::write(path, content)
            .map_err(|e| format!("无法写入文件: {}", e))?;
    }

    Ok(())
}

/// Set a peripheral's SYSDMA channel mapping (ch-remap).
#[tauri::command]
pub fn set_peripheral_sysdma_channels(
    state: tauri::State<'_, DtsState>,
    peripheral: String,
    channels: Vec<String>,
) -> Result<(), String> {
    let mut parser = state.parser.lock().map_err(|e| e.to_string())?;
    DtsWriter::update_sysdma_channels(&mut parser, &peripheral, channels)?;

    // Write back to file
    if let Some(path) = parser.get_file_path() {
        let content = parser.get_file_content().to_string();
        std::fs::write(path, content)
            .map_err(|e| format!("无法写入文件: {}", e))?;
    }

    Ok(())
}

// ─── Unit tests ──────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_default_peripheral_info() {
        let info = PeripheralInfo::default();
        assert_eq!(info.status, "okay");
        assert_eq!(info.pwm_cells, 1);
        assert_eq!(info.current_speed, 115200);
        assert_eq!(info.clock_frequency, 0);
        assert_eq!(info.sysdma_channels, PeripheralInfo::default_sysdma_channels());
    }

    #[test]
    fn test_default_sysdma_channels() {
        let channels = PeripheralInfo::default_sysdma_channels();
        assert_eq!(channels, vec!["0", "5", "12", "13", "42", "42", "4", "7"]);
    }

    #[test]
    fn test_channel_number_name_round_trip() {
        for num in &["0", "5", "12", "42"] {
            let name = SysdmaChannelMap::channel_name(num);
            let back = SysdmaChannelMap::channel_number(&name);
            assert_eq!(&back, num);
        }
    }

    #[test]
    fn test_channel_number_specific_values() {
        assert_eq!(SysdmaChannelMap::channel_number("CVI_I2S0_RX"), "0");
        assert_eq!(SysdmaChannelMap::channel_number("CVI_UART2_RX"), "12");
        assert_eq!(SysdmaChannelMap::channel_number("CVI_SPI_NAND"), "42");
    }

    #[test]
    fn test_channel_name_specific_values() {
        assert_eq!(SysdmaChannelMap::channel_name("5"), "CVI_I2S2_TX");
        assert_eq!(SysdmaChannelMap::channel_name("13"), "CVI_UART2_TX");
        assert_eq!(SysdmaChannelMap::channel_name("42"), "CVI_SPI_NAND");
    }

    #[test]
    fn test_peripheral_node_from_channel() {
        assert_eq!(SysdmaChannelMap::peripheral_node_from_channel("8"), Some("uart0".to_string()));
        assert_eq!(SysdmaChannelMap::peripheral_node_from_channel("12"), Some("uart2".to_string()));
        assert_eq!(SysdmaChannelMap::peripheral_node_from_channel("40"), Some("uart4".to_string()));
        assert_eq!(SysdmaChannelMap::peripheral_node_from_channel("0"), Some("i2s0".to_string()));
        assert_eq!(SysdmaChannelMap::peripheral_node_from_channel("16"), Some("spi0".to_string()));
        assert_eq!(SysdmaChannelMap::peripheral_node_from_channel("24"), Some("i2c0".to_string()));
        assert_eq!(SysdmaChannelMap::peripheral_node_from_channel("43"), None); // out of range
    }

    #[test]
    fn test_is_channel_rx() {
        assert!(SysdmaChannelMap::is_channel_rx("0"));   // CVI_I2S0_RX
        assert!(!SysdmaChannelMap::is_channel_rx("1"));  // CVI_I2S0_TX
        assert!(SysdmaChannelMap::is_channel_rx("12"));  // CVI_UART2_RX
        assert!(!SysdmaChannelMap::is_channel_rx("13")); // CVI_UART2_TX
        assert!(SysdmaChannelMap::is_channel_rx("37"));  // CVI_AUDSRC — special
        assert!(SysdmaChannelMap::is_channel_rx("42"));  // CVI_SPI_NAND — special
    }

    #[test]
    fn test_peripheral_info_serialization() {
        let info = PeripheralInfo::default();
        let json = serde_json::to_string(&info).unwrap();
        let deserialized: PeripheralInfo = serde_json::from_str(&json).unwrap();
        assert_eq!(info.name, deserialized.name);
        assert_eq!(info.status, deserialized.status);
        assert_eq!(info.sysdma_channels, deserialized.sysdma_channels);
    }
}