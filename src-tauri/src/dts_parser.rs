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
            // ── Additional peripherals ──────────────────────────
            // Base-dtsi nodes are matched via their label (`name:`); the
            // `&name {` form covers overlay references. Short names use `\b`
            // to avoid matching inside longer identifiers (e.g. `sd` in
            // `wifi-sd`). Wiegand nodes are plain `wiegandN { ... }`.
            r"&(usb)\s*\{",
            r"\b(usb)\s*:",
            r"&(ethernet[0-9]+)\s*\{",
            r"(ethernet[0-9]+)\s*:",
            r"&(emmc)\s*\{",
            r"\b(emmc)\s*:",
            r"&(sd)\s*\{",
            r"\b(sd)\s*:",
            r"&(wifisd)\s*\{",
            r"\b(wifisd)\s*:",
            r"&(watchdog[0-9]+)\s*\{",
            r"(watchdog[0-9]+)\s*:",
            r"&(spinand)\s*\{",
            r"\b(spinand)\s*:",
            r"&(spif)\s*\{",
            r"\b(spif)\s*:",
            r"&(dmac)\s*\{",
            r"\b(dmac)\s*:",
            r"&(thermal)\s*\{",
            r"\b(thermal)\s*:",
            r"&(keyscan)\s*\{",
            r"\b(keyscan)\s*:",
            r"&(irrx)\s*\{",
            r"\b(irrx)\s*:",
            r"(wiegand[0-9]+)\s*\{",
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

    // ── Raw property parsing (generic key/value editor) ───────

    /// Parse ALL raw properties of a node (independent of `parse_node`).
    ///
    /// Returns every property declared directly on the node — regardless of
    /// whether it is one of the 5 "friendly" properties — preserving DTS
    /// declaration order. Nested child-node properties are excluded.
    pub fn get_raw_properties(
        &self,
        node_name: &str,
    ) -> Result<Vec<crate::peripheral::RawProperty>, String> {
        let (start, end) = self
            .find_node_position(node_name)
            .ok_or_else(|| format!("未找到节点: {}", node_name))?;
        let node_content = &self.file_content[start..end];
        Ok(Self::parse_raw_properties_from_node(node_content))
    }

    /// Parse the raw properties out of a single node slice (`... { body }`).
    ///
    /// Steps: strip comments → isolate the outermost `{ }` body → remove
    /// nested child `{ }` blocks (and their labels) → collapse newlines so
    /// multi-line cell values become a single statement → split on `;` →
    /// classify each statement as cell / string / bool.
    pub fn parse_raw_properties_from_node(
        node_content: &str,
    ) -> Vec<crate::peripheral::RawProperty> {
        use crate::peripheral::{is_protected_property, RawProperty};

        // b. Remove comments first — this keeps braces inside comments from
        //    throwing off the brace-counting below.
        let no_block = Regex::new(r"(?s)/\*.*?\*/")
            .unwrap()
            .replace_all(node_content, "")
            .to_string();
        let no_line = Regex::new(r"//[^\n]*")
            .unwrap()
            .replace_all(&no_block, "")
            .to_string();

        // b2. Drop disabled `#if 0 ... #endif` blocks entirely, then strip any
        //     remaining preprocessor directive lines (keeping their body). This
        //     avoids surfacing disabled properties (e.g. an alternate `clocks`)
        //     and prevents a lone `#endif` from swallowing the next property.
        //     `#...-cells` properties are preserved (not directive keywords).
        let no_if0 = Regex::new(r"(?s)[ \t]*#if[ \t]+0\b.*?#endif\b[^\n]*\r?\n?")
            .unwrap()
            .replace_all(&no_line, "")
            .to_string();
        let cleaned = Regex::new(
            r"(?m)^[ \t]*#(?:ifdef|ifndef|elif|else|endif|include|define|undef|pragma|error|warning|if)\b[^\n]*\r?\n?",
        )
        .unwrap()
        .replace_all(&no_if0, "")
        .to_string();

        // a. Isolate the outermost { ... } body via brace counting.
        let body = match extract_outer_body(&cleaned) {
            Some(b) => b,
            None => return Vec::new(),
        };

        // c. Remove nested child-node blocks (and the label preceding them).
        let body = strip_child_blocks(&body);

        // d. Collapse newlines to spaces so multi-line <...> cell values stay
        //    within a single ';'-delimited statement.
        let flattened: String = body
            .chars()
            .map(|c| if c == '\n' || c == '\r' { ' ' } else { c })
            .collect();

        let cell_re = Regex::new(r#"^\s*([#\w\-,]+)\s*=\s*<(.*)>\s*$"#).unwrap();
        let string_re = Regex::new(r#"^\s*([#\w\-,]+)\s*=\s*"(.*)"\s*$"#).unwrap();
        let bool_re = Regex::new(r#"^\s*([#\w\-,]+)\s*$"#).unwrap();

        let mut props: Vec<RawProperty> = Vec::new();
        // e/g. Classify each statement (order: cell → string → bool), keeping
        //       the original declaration order via Vec push order.
        for stmt in flattened.split(';') {
            let s = stmt.trim();
            if s.is_empty() {
                continue;
            }

            if let Some(caps) = cell_re.captures(s) {
                let key = caps[1].to_string();
                let protected = is_protected_property(&key);
                props.push(RawProperty {
                    key,
                    value: caps[2].trim().to_string(),
                    kind: "cell".to_string(),
                    protected,
                });
            } else if let Some(caps) = string_re.captures(s) {
                let key = caps[1].to_string();
                let protected = is_protected_property(&key);
                props.push(RawProperty {
                    key,
                    value: caps[2].to_string(),
                    kind: "string".to_string(),
                    protected,
                });
            } else if let Some(caps) = bool_re.captures(s) {
                let key = caps[1].to_string();
                let protected = is_protected_property(&key);
                props.push(RawProperty {
                    key,
                    value: String::new(),
                    kind: "bool".to_string(),
                    protected,
                });
            }
            // Anything else (e.g. a leftover child-node label) is skipped.
        }

        props
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

/// Extract the content between the first `{` and its matching `}` (exclusive).
///
/// Uses brace counting so nested `{ }` blocks are handled correctly.
fn extract_outer_body(content: &str) -> Option<String> {
    let open = content.find('{')?;
    let bytes = content.as_bytes();
    let mut depth: i32 = 0;
    let mut i = open;

    while i < bytes.len() {
        match bytes[i] {
            b'{' => depth += 1,
            b'}' => {
                depth -= 1;
                if depth == 0 {
                    return Some(content[open + 1..i].to_string());
                }
            }
            _ => {}
        }
        i += 1;
    }

    None
}

/// Remove nested child-node blocks (`label { ... };`) from a node body.
///
/// Both the `{ ... }` block and the label token immediately preceding it are
/// removed, along with a trailing `;`, so child-node properties are never
/// mistaken for properties of the parent node.
fn strip_child_blocks(body: &str) -> String {
    let bytes = body.as_bytes();
    let mut result = String::new();
    let mut segment_start = 0;
    let mut i = 0;

    let is_label_char = |c: u8| {
        c.is_ascii_alphanumeric() || c == b'_' || c == b'@' || c == b'-' || c == b',' || c == b'#'
    };
    let is_ws = |c: u8| matches!(c, b' ' | b'\t' | b'\n' | b'\r');

    while i < bytes.len() {
        if bytes[i] == b'{' {
            // Walk back over whitespace, then over the label token, so the
            // `label` before this `{` is dropped together with the block.
            let mut label_start = i;
            while label_start > segment_start && is_ws(bytes[label_start - 1]) {
                label_start -= 1;
            }
            while label_start > segment_start && is_label_char(bytes[label_start - 1]) {
                label_start -= 1;
            }
            result.push_str(&body[segment_start..label_start]);

            // Find the matching closing brace.
            let mut depth: i32 = 1;
            let mut j = i + 1;
            while j < bytes.len() && depth > 0 {
                match bytes[j] {
                    b'{' => depth += 1,
                    b'}' => depth -= 1,
                    _ => {}
                }
                j += 1;
            }

            // Skip trailing whitespace and one terminating ';' after the block.
            let mut k = j;
            while k < bytes.len() && is_ws(bytes[k]) {
                k += 1;
            }
            if k < bytes.len() && bytes[k] == b';' {
                k += 1;
            }

            segment_start = k;
            i = k;
        } else {
            i += 1;
        }
    }

    result.push_str(&body[segment_start..]);
    result
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

    #[test]
    fn test_discover_added_peripherals() {
        // 各新增外设在 cv184x_base.dtsi 中的真实节点写法（label 形式 + wiegand 纯节点）。
        let content = r#"
	dmac: dma@0x4330000 {
		compatible = "snps,dmac-bm";
	};
	watchdog0: cv-wd@0x3010000 {
		compatible = "snps,dw-wdt";
	};
	spinand:cv-spinf@4060000 {
		compatible = "cvitek,cv1835-spinf";
	};
	spif:cvi-spif@10000000 {
		compatible = "snps,dw-apb-ssi-4.04a";
		spi-nor@0 {
			compatible = "jedec,spi-nor";
		};
	};
	ethernet0: ethernet@4070000 {
		compatible = "cvitek,ethernet";
	};
	emmc:cv-emmc@4300000 {
		compatible = "cvitek,cv184x-emmc";
	};
	sd:cv-sd@4310000 {
		compatible = "cvitek,cv184x-sd";
	};
	wifisd:wifi-sd@4320000 {
		compatible = "cvitek,cv184x-sdio";
		status = "disabled";
	};
	usb: usb@04340000 {
		compatible = "cvitek,cv182x-usb";
		status = "okay";
	};
	thermal:thermal@030E0000 {
		compatible = "cvitek,cv184x-thermal";
	};
	irrx:irrx@0502E000 {
		compatible = "cvitek,irrx";
	};
	keyscan:keyscan@03040000 {
		compatible = "cvitek,keyscan";
	};
	wiegand0 {
		compatible = "cvitek,wiegand";
	};
	wiegand1 {
		compatible = "cvitek,wiegand";
	};
	wiegand2 {
		compatible = "cvitek,wiegand";
	};
"#;
        let mut parser = DtsParser::new();
        parser.load_content(content);

        for name in [
            "dmac",
            "watchdog0",
            "spinand",
            "spif",
            "ethernet0",
            "emmc",
            "sd",
            "wifisd",
            "usb",
            "thermal",
            "irrx",
            "keyscan",
            "wiegand0",
            "wiegand1",
            "wiegand2",
        ] {
            assert!(
                parser.get_peripheral(name).is_some(),
                "未发现外设节点: {}",
                name
            );
        }

        // status 解析正确（wifisd 禁用、usb 使能）
        assert_eq!(parser.get_peripheral("wifisd").unwrap().status, "disabled");
        assert_eq!(parser.get_peripheral("usb").unwrap().status, "okay");

        // 短名不应错配到更长的标识符：sd 与 wifisd 是两个独立节点
        assert!(parser.get_peripheral("sd").is_some());
        assert!(parser.get_peripheral("wifisd").is_some());
    }

    #[test]
    fn test_raw_props_skip_if0_block() {
        // usb 节点内部含 `#if 0 ... #endif`（禁用的备用 clocks/clock-names）。
        // 原始属性解析应忽略被禁用块，且不吞掉紧随 #endif 之后的 vbus-gpio。
        let usb = "usb: usb@04340000 {\n\
			\tcompatible = \"cvitek,cv182x-usb\";\n\
			\tclocks = <&clk A>, <&clk B>;\n\
			\tclock-names = \"clk_a\", \"clk_b\";\n\
			#if 0\n\
			\tclocks = <&clk X>;\n\
			\tclock-names = \"clk_x\";\n\
			#endif\n\
			\tvbus-gpio = <&portb 6 0>;\n\
			\tstatus = \"okay\";\n\
			};";
        let props = DtsParser::parse_raw_properties_from_node(usb);

        // clocks / clock-names 各只出现一次（不含被禁用的重复项）
        assert_eq!(props.iter().filter(|p| p.key == "clocks").count(), 1);
        assert_eq!(props.iter().filter(|p| p.key == "clock-names").count(), 1);
        // vbus-gpio 未被 #endif 吞掉
        assert!(props.iter().any(|p| p.key == "vbus-gpio"));
        // 保留的是启用块的值（A/B），而非被禁用块的值（X）
        let clocks = props.iter().find(|p| p.key == "clocks").unwrap();
        assert!(clocks.value.contains('A') && clocks.value.contains('B'));
        assert!(!clocks.value.contains('X'));
        // status 仍能解析
        assert!(props.iter().any(|p| p.key == "status" && p.value == "okay"));
    }
}
