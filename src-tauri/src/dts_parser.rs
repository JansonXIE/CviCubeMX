//! DTS (Device Tree Source) file parser.
//!
//! Parses `.dtsi` file content into structured `PeripheralInfo` data,
//! preserving line number information for precise line-level modifications.

use regex::Regex;
use std::collections::HashMap;

use crate::peripheral::PeripheralInfo;

/// DTS file parser — parses .dtsi file content into structured peripheral data.
pub struct DtsParser {
    /// Raw file content (kept for writer to modify)
    file_content: String,
    /// Parsed peripherals keyed by node name
    peripherals: HashMap<String, PeripheralInfo>,
    /// File path (for write-back)
    file_path: Option<String>,
}

impl Clone for DtsParser {
    fn clone(&self) -> Self {
        DtsParser {
            file_content: self.file_content.clone(),
            peripherals: self.peripherals.clone(),
            file_path: self.file_path.clone(),
        }
    }
}

impl DtsParser {
    pub fn new() -> Self {
        DtsParser {
            file_content: String::new(),
            peripherals: HashMap::new(),
            file_path: None,
        }
    }

    /// Load and parse a .dtsi file from the given path.
    pub fn load_file(&mut self, file_path: &str) -> Result<(), String> {
        let content = std::fs::read_to_string(file_path)
            .map_err(|e| format!("无法打开设备树文件 {}: {}", file_path, e))?;
        self.file_content = content;
        self.file_path = Some(file_path.to_string());
        self.parse_dts_file();
        Ok(())
    }

    /// Load from raw content string (useful for testing / writer re-parse).
    pub fn load_content(&mut self, content: &str) {
        self.file_content = content.to_string();
        self.parse_dts_file();
    }

    /// Get parsed peripherals.
    pub fn get_peripherals(&self) -> &HashMap<String, PeripheralInfo> {
        &self.peripherals
    }

    /// Get a specific peripheral by name.
    pub fn get_peripheral(&self, name: &str) -> Option<&PeripheralInfo> {
        self.peripherals.get(name)
    }

    /// Get the raw file content (for writer to modify).
    pub fn get_file_content(&self) -> &str {
        &self.file_content
    }

    /// Get the file path (for write-back).
    pub fn get_file_path(&self) -> Option<&str> {
        self.file_path.as_deref()
    }

    /// Set peripheral status (in-memory only, no file change).
    pub fn set_peripheral_status(&mut self, peripheral: &str, status: &str) -> Result<(), String> {
        if let Some(info) = self.peripherals.get_mut(peripheral) {
            info.status = status.to_string();
            info.has_status = true;
            Ok(())
        } else {
            Err(format!("未找到外设: {}", peripheral))
        }
    }

    /// Set peripheral clock frequency (in-memory only).
    pub fn set_peripheral_clock_frequency(
        &mut self,
        peripheral: &str,
        frequency: i32,
    ) -> Result<(), String> {
        if let Some(info) = self.peripherals.get_mut(peripheral) {
            info.clock_frequency = frequency;
            info.has_clock_freq = true;
            Ok(())
        } else {
            Err(format!("未找到外设: {}", peripheral))
        }
    }

    /// Set peripheral PWM cells (in-memory only).
    pub fn set_peripheral_pwm_cells(&mut self, peripheral: &str, cells: i32) -> Result<(), String> {
        if let Some(info) = self.peripherals.get_mut(peripheral) {
            info.pwm_cells = cells;
            info.has_pwm_cells = true;
            Ok(())
        } else {
            Err(format!("未找到外设: {}", peripheral))
        }
    }

    /// Set peripheral current speed (in-memory only).
    pub fn set_peripheral_current_speed(
        &mut self,
        peripheral: &str,
        speed: i32,
    ) -> Result<(), String> {
        if let Some(info) = self.peripherals.get_mut(peripheral) {
            info.current_speed = speed;
            info.has_current_speed = true;
            Ok(())
        } else {
            Err(format!("未找到外设: {}", peripheral))
        }
    }

    /// Set peripheral sysdma channels (in-memory only).
    pub fn set_peripheral_sysdma_channels(
        &mut self,
        peripheral: &str,
        channels: Vec<String>,
    ) -> Result<(), String> {
        if let Some(info) = self.peripherals.get_mut(peripheral) {
            info.sysdma_channels = channels;
            info.has_sysdma_channels = true;
            Ok(())
        } else {
            Err(format!("未找到外设: {}", peripheral))
        }
    }

    // ── Internal parsing ──────────────────────────────────────

    /// Main parse routine — mirrors C++ `parseDtsFile()`.
    fn parse_dts_file(&mut self) {
        self.peripherals.clear();

        // DTS overlay format: &nodeName { ... }; or nodeName: &nodeName { ... };
        // Plain format: nodeName { ... }; (for sysdma_remap)
        // We scan for node references and extract the node name
        let scan_patterns = [
            // DTS reference format: &nodeName followed by whitespace and {
            r"&(pwm[0-9]+)\s*\{",
            r"&(i2c[0-9]+)\s*\{",
            r"&(spi[0-9]+)\s*\{",
            r"&(uart[0-9]+)\s*\{",
            r"&(gpio[0-9]+)\s*\{",
            r"&(saradc[0-9]*)\s*\{",
            // Label format: nodeName: (overlay label)
            r"(pwm[0-9]+):",
            r"(i2c[0-9]+):",
            r"(spi[0-9]+):",
            r"(uart[0-9]+):",
            r"(gpio[0-9]+):",
            r"(saradc[0-9]*):",
            // sysdma_remap special format: sysdma_remap {
            r"(sysdma_remap)\s*\{",
        ];

        for pattern in &scan_patterns {
            let regex = Regex::new(pattern).expect("Invalid peripheral pattern regex");

            for cap in regex.captures_iter(&self.file_content) {
                // The first capture group is the node name
                let node_name = cap[1].to_string();

                if let Some((start, end)) = self.find_node_position(&node_name) {
                    let info = self.parse_node(&node_name, start, end);
                    self.peripherals.insert(node_name, info);
                }
            }
        }

        // Ensure sysdma_remap node exists (even if not in DTS file)
        if !self.peripherals.contains_key("sysdma_remap") {
            let mut sysdma_info = PeripheralInfo::default();
            sysdma_info.name = "sysdma_remap".to_string();
            sysdma_info.status = "okay".to_string();
            sysdma_info.has_status = true;
            sysdma_info.has_sysdma_channels = true;
            self.peripherals
                .insert("sysdma_remap".to_string(), sysdma_info);
        }
    }

    /// Parse a single peripheral node — mirrors C++ `parseNode()`.
    fn parse_node(&self, node_name: &str, start: usize, end: usize) -> PeripheralInfo {
        let node_content = &self.file_content[start..end];

        let mut info = PeripheralInfo::default();
        info.name = node_name.to_string();

        // Parse status
        if let Some(caps) = Regex::new(r#"status\s*=\s*"([^"]+)";"#)
            .unwrap()
            .captures(node_content)
        {
            info.status = caps[1].to_string();
            info.has_status = true;
        } else {
            info.status = "okay".to_string();
            info.has_status = false;
        }

        // Parse clocks
        if let Some(caps) = Regex::new(r"clocks\s*=\s*<([^>]+)>;")
            .unwrap()
            .captures(node_content)
        {
            let clock_content = &caps[1];
            info.has_clock = true;
            if let Some(clock_caps) = Regex::new(r"CV184X_CLK_(\w+)")
                .unwrap()
                .captures(clock_content)
            {
                info.clock_name = clock_caps[1].to_string();
            }
        } else {
            info.has_clock = false;
        }

        // Parse clock-frequency
        if let Some(caps) = Regex::new(r"clock-frequency\s*=\s*<([^>]+)>;")
            .unwrap()
            .captures(node_content)
        {
            info.clock_freq = caps[1].to_string();
            info.has_clock_freq = true;
            info.clock_frequency = caps[1].trim().parse::<i32>().unwrap_or(0);
        } else {
            info.has_clock_freq = false;
            info.clock_frequency = 0;
        }

        // Parse #pwm-cells
        if let Some(caps) = Regex::new(r"#pwm-cells\s*=\s*<([^>]+)>;")
            .unwrap()
            .captures(node_content)
        {
            info.has_pwm_cells = true;
            info.pwm_cells = caps[1].trim().parse::<i32>().unwrap_or(1);
        } else {
            info.has_pwm_cells = false;
            info.pwm_cells = 1;
        }

        // Parse current-speed (UART only)
        if let Some(caps) = Regex::new(r"current-speed\s*=\s*<([^>]+)>;")
            .unwrap()
            .captures(node_content)
        {
            info.has_current_speed = true;
            info.current_speed = caps[1].trim().parse::<i32>().unwrap_or(115200);
        } else {
            info.has_current_speed = false;
            info.current_speed = 115200;
        }

        // Parse ch-remap (SYSDMA only)
        if let Some(caps) = Regex::new(r"ch-remap\s*=\s*<([^>]+)>;")
            .unwrap()
            .captures(node_content)
        {
            info.has_sysdma_channels = true;
            let ch_remap_content = caps[1].trim();
            let channels: Vec<String> = ch_remap_content
                .split_whitespace()
                .map(|ch| {
                    if ch.starts_with("CVI_") {
                        crate::peripheral::SysdmaChannelMap::channel_number(ch)
                    } else {
                        ch.to_string()
                    }
                })
                .collect();
            info.sysdma_channels = channels;
        } else {
            info.has_sysdma_channels = false;
            info.sysdma_channels = PeripheralInfo::default_sysdma_channels();
        }

        // Calculate line number (1-based)
        let before_node = &self.file_content[..start];
        info.line_number = before_node.matches('\n').count() as i32 + 1;

        info
    }

    /// Find node position in file content.
    ///
    /// Supports multiple DTS formats:
    /// 1. `&nodeName { ... }` — DTS overlay reference
    /// 2. `nodeName: &nodeName { ... }` — overlay with label
    /// 3. `nodeName { ... }` — plain node (e.g. sysdma_remap)
    fn find_node_position(&self, node_name: &str) -> Option<(usize, usize)> {
        find_node_position_in_str(&self.file_content, node_name)
    }

    /// Find node position in arbitrary content string (used by DtsWriter).
    pub fn find_node_position_in_content(content: &str, node_name: &str) -> Option<(usize, usize)> {
        find_node_position_in_str(content, node_name)
    }
}

/// Shared helper: find node position in a string.
///
/// Tries overlay format first, then &reference format, then plain format.
fn find_node_position_in_str(content: &str, node_name: &str) -> Option<(usize, usize)> {
    let escaped = regex::escape(node_name);

    // Try overlay format first: nodeName: ... {
    // We construct the regex as a raw string to avoid format!() brace escaping issues.
    let re1_str = format!("\\b{}\\s*:\\s*[\\s\\S]*?\\u007B", escaped);
    let re1 = Regex::new(&re1_str).ok();

    // Try &reference format: &nodeName { or &nodeName\s*{
    let re2_str = format!("\\b&{}\\s*\\u007B", escaped);
    let re2 = Regex::new(&re2_str).ok();

    // Try plain format: nodeName {
    let re3_str = format!("\\b{}\\s+\\u007B", escaped);
    let re3 = Regex::new(&re3_str).ok();

    // Try each format in priority order
    let match_opt = re1
        .as_ref()
        .and_then(|r| r.find(content))
        .or_else(|| re2.as_ref().and_then(|r| r.find(content)))
        .or_else(|| re3.as_ref().and_then(|r| r.find(content)));

    let m = match_opt?;
    let start = m.start();
    let brace_start = m.end() - 1; // byte position of '{'

    // Find matching closing brace (byte-level scan)
    let bytes = content.as_bytes();
    let mut brace_count: i32 = 1;
    let mut pos = brace_start + 1;

    while pos < bytes.len() && brace_count > 0 {
        if bytes[pos] == b'{' {
            brace_count += 1;
        } else if bytes[pos] == b'}' {
            brace_count -= 1;
        }
        pos += 1;
    }

    if brace_count == 0 {
        Some((start, pos))
    } else {
        None
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_simple_i2c_node() {
        let content = "&i2c0 {\n\tstatus = \"okay\";\n\tclock-frequency = <100000>;\n};";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        let peripherals = parser.get_peripherals();
        assert!(peripherals.contains_key("i2c0"));

        let info = peripherals.get("i2c0").unwrap();
        assert_eq!(info.name, "i2c0");
        assert_eq!(info.status, "okay");
        assert_eq!(info.clock_frequency, 100000);
        assert_eq!(info.clock_freq, "100000");
    }

    #[test]
    fn test_parse_pwm_node() {
        let content = "&pwm0 {\n\tstatus = \"disabled\";\n\t#pwm-cells = <3>;\n};";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        let info = parser.get_peripheral("pwm0").unwrap();
        assert_eq!(info.status, "disabled");
        assert_eq!(info.pwm_cells, 3);
        assert!(info.has_pwm_cells);
    }

    #[test]
    fn test_parse_uart_node() {
        let content = "&uart0 {\n\tstatus = \"okay\";\n\tcurrent-speed = <9600>;\n};";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        let info = parser.get_peripheral("uart0").unwrap();
        assert_eq!(info.status, "okay");
        assert_eq!(info.current_speed, 9600);
        assert!(info.has_current_speed);
    }

    #[test]
    fn test_parse_sysdma_remap_node() {
        let content = "sysdma_remap {\n\tch-remap = <CVI_I2S0_RX CVI_I2S2_TX CVI_I2S1_RX CVI_I2S1_TX CVI_SPI_NAND CVI_SPI_NAND CVI_I2S2_RX CVI_I2S3_TX>;\n};";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        let info = parser.get_peripheral("sysdma_remap").unwrap();
        assert!(info.has_sysdma_channels);
        // CVI_I2S0_RX->0, CVI_I2S2_TX->5, CVI_I2S1_RX->2, CVI_I2S1_TX->3
        // CVI_SPI_NAND->42, CVI_SPI_NAND->42, CVI_I2S2_RX->4, CVI_I2S3_TX->7
        assert_eq!(
            info.sysdma_channels,
            vec!["0", "5", "2", "3", "42", "42", "4", "7"]
        );
    }

    #[test]
    fn test_default_sysdma_when_missing() {
        let content = "&i2c0 {\n\tstatus = \"okay\";\n};";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        let info = parser.get_peripheral("sysdma_remap").unwrap();
        assert_eq!(
            info.sysdma_channels,
            PeripheralInfo::default_sysdma_channels()
        );
        assert_eq!(
            info.sysdma_channels,
            vec!["0", "5", "2", "3", "42", "42", "4", "7"]
        );
    }

    #[test]
    fn test_line_number_calculation() {
        let content =
            "/* comment line 1 */\n/* comment line 2 */\n&i2c0 {\n\tstatus = \"okay\";\n};";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        let info = parser.get_peripheral("i2c0").unwrap();
        assert_eq!(info.line_number, 3);
    }

    #[test]
    fn test_node_without_status() {
        let content = "&spi0 {\n\tclock-frequency = <50000000>;\n};";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        let info = parser.get_peripheral("spi0").unwrap();
        assert_eq!(info.status, "okay");
        assert!(!info.has_status);
    }

    #[test]
    fn test_parse_clocks_property() {
        let content = "&i2c0 {\n\tclocks = <&clk CV184X_CLK_I2C0>;\n};";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        let info = parser.get_peripheral("i2c0").unwrap();
        assert!(info.has_clock);
        assert_eq!(info.clock_name, "I2C0");
    }

    #[test]
    fn test_sysdma_channels_numeric_form() {
        let content =
            "sysdma_remap {\n\tch-remap = <0 5 12 13 42 42 4 7>;\n\tstatus = \"okay\";\n};\n";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        let info = parser.get_peripheral("sysdma_remap").unwrap();
        assert_eq!(
            info.sysdma_channels,
            vec!["0", "5", "12", "13", "42", "42", "4", "7"]
        );
    }

    #[test]
    fn test_parse_with_comments() {
        let content = "&i2c0 {\n\t/* this is a comment */\n\tstatus = \"okay\"; /* inline comment */\n\tclock-frequency = <100000>;\n};";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        let info = parser.get_peripheral("i2c0").unwrap();
        assert_eq!(info.status, "okay");
        assert_eq!(info.clock_frequency, 100000);
    }

    #[test]
    fn test_parse_multiple_peripherals() {
        let content = "\
&pwm0 {\n\tstatus = \"okay\";\n\t#pwm-cells = <3>;\n};\n\
&i2c0 {\n\tstatus = \"disabled\";\n\tclock-frequency = <100000>;\n};\n\
&uart0 {\n\tstatus = \"okay\";\n\tcurrent-speed = <9600>;\n};\n";

        let mut parser = DtsParser::new();
        parser.load_content(content);

        assert!(parser.get_peripherals().contains_key("pwm0"));
        assert!(parser.get_peripherals().contains_key("i2c0"));
        assert!(parser.get_peripherals().contains_key("uart0"));
        assert!(parser.get_peripherals().contains_key("sysdma_remap"));
    }

    #[test]
    fn test_find_node_position_overlay_format() {
        let content = "i2c0: &i2c0 {\n\tstatus = \"okay\";\n};\n";
        let pos = DtsParser::find_node_position_in_content(content, "i2c0");
        assert!(pos.is_some());
        let (start, end) = pos.unwrap();
        let node = &content[start..end];
        assert!(node.contains("status = \"okay\";"));
    }

    #[test]
    fn test_find_node_position_plain_format() {
        let content = "sysdma_remap {\n\tstatus = \"okay\";\n};\n";
        let pos = DtsParser::find_node_position_in_content(content, "sysdma_remap");
        assert!(pos.is_some());
    }

    #[test]
    fn test_find_node_position_reference_format() {
        let content = "&uart0 {\n\tstatus = \"disabled\";\n};\n";
        let pos = DtsParser::find_node_position_in_content(content, "uart0");
        assert!(pos.is_some());
        let (start, end) = pos.unwrap();
        let node = &content[start..end];
        assert!(node.contains("status = \"disabled\";"));
    }
}
