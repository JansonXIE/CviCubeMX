//! DTS file writer module.
//!
//! Modifies DTS properties using line-level positioning — only the
//! target line is replaced, preserving comments and formatting.

use crate::dts_parser::DtsParser;
use crate::peripheral::SysdmaChannelMap;

/// DTS file writer that modifies properties using line-level positioning.
///
/// Only the target line is replaced; all other content (including comments
/// and formatting) is preserved unchanged.
pub struct DtsWriter;

impl DtsWriter {
    /// Update a single peripheral's status in the file content.
    pub fn update_status(parser: &mut DtsParser, peripheral: &str, status: &str) -> Result<(), String> {
        let info = parser.get_peripheral(peripheral)
            .cloned()
            .ok_or_else(|| format!("未找到外设: {}", peripheral))?;

        Self::update_property_in_node(
            parser,
            peripheral,
            r#"\s*status\s*=\s*"[^"]+"\s*;"#,
            format!("\n\t\tstatus = \"{}\";", status),
            info.has_status || status != "okay",
        )?;

        parser.set_peripheral_status(peripheral, status)?;
        Ok(())
    }

    /// Update a peripheral's clock-frequency in the file content.
    pub fn update_clock_frequency(parser: &mut DtsParser, peripheral: &str, frequency: i32) -> Result<(), String> {
        let info = parser.get_peripheral(peripheral)
            .cloned()
            .ok_or_else(|| format!("未找到外设: {}", peripheral))?;

        Self::update_property_in_node(
            parser,
            peripheral,
            r#"\s*clock-frequency\s*=\s*<[^>]+>\s*;"#,
            format!("\n\t\tclock-frequency = <{}>;", frequency),
            info.has_clock_freq && frequency > 0,
        )?;

        parser.set_peripheral_clock_frequency(peripheral, frequency)?;
        Ok(())
    }

    /// Update a peripheral's #pwm-cells in the file content.
    pub fn update_pwm_cells(parser: &mut DtsParser, peripheral: &str, cells: i32) -> Result<(), String> {
        let info = parser.get_peripheral(peripheral)
            .cloned()
            .ok_or_else(|| format!("未找到外设: {}", peripheral))?;

        Self::update_property_in_node(
            parser,
            peripheral,
            r#"\s*#pwm-cells\s*=\s*<[^>]+>\s*;"#,
            format!("\n\t\t#pwm-cells = <{}>;", cells),
            info.has_pwm_cells && cells > 0,
        )?;

        parser.set_peripheral_pwm_cells(peripheral, cells)?;
        Ok(())
    }

    /// Update a peripheral's current-speed in the file content.
    pub fn update_current_speed(parser: &mut DtsParser, peripheral: &str, speed: i32) -> Result<(), String> {
        let info = parser.get_peripheral(peripheral)
            .cloned()
            .ok_or_else(|| format!("未找到外设: {}", peripheral))?;

        Self::update_property_in_node(
            parser,
            peripheral,
            r#"\s*current-speed\s*=\s*<[^>]+>\s*;"#,
            format!("\n\t\tcurrent-speed = <{}>;", speed),
            info.has_current_speed && speed > 0,
        )?;

        parser.set_peripheral_current_speed(peripheral, speed)?;
        Ok(())
    }

    /// Update a peripheral's sysdma channels (ch-remap) in the file content.
    ///
    /// Also handles DMA config cascade updates for affected peripherals.
    pub fn update_sysdma_channels(parser: &mut DtsParser, peripheral: &str, channels: Vec<String>) -> Result<(), String> {
        // Save the previous channels (parsed from current DTS file content)
        let previous_channels = Self::get_previous_sysdma_channels(parser);

        let channel_names: Vec<String> = channels.iter()
            .map(|ch| SysdmaChannelMap::channel_name(ch))
            .collect();

        let ch_remap_value = if channel_names.len() >= 8 {
            let first_line: Vec<&str> = channel_names[0..4].iter().map(|s| s.as_str()).collect();
            let second_line: Vec<&str> = channel_names[4..8].iter().map(|s| s.as_str()).collect();
            format!("{}\n\t\t\t\t\t{}", first_line.join(" "), second_line.join(" "))
        } else {
            channel_names.join(" ")
        };

        // If the node doesn't exist in the file, create it
        let node_pos = DtsParser::find_node_position_in_content(parser.get_file_content(), peripheral);
        if node_pos.is_none() && peripheral == "sysdma_remap" {
            Self::create_sysdma_remap_node(parser, &channels);
            parser.set_peripheral_sysdma_channels(peripheral, channels.clone())?;
            Self::update_peripheral_dma_config(parser, &previous_channels, &channels);
            return Ok(());
        }

        Self::update_property_in_node(
            parser,
            peripheral,
            r#"\s*ch-remap\s*=\s*<[^>]+>\s*;"#,
            format!("\n\t\tch-remap = <{}>;", ch_remap_value),
            true,
        )?;

        parser.set_peripheral_sysdma_channels(peripheral, channels.clone())?;

        // Update DMA configs for affected peripherals
        if peripheral == "sysdma_remap" {
            Self::update_peripheral_dma_config(parser, &previous_channels, &channels);
        }

        Ok(())
    }

    // ── Helper methods ──────────────────────────────────────

    /// Core method: find a node, replace or insert a property line.
    fn update_property_in_node(
        parser: &mut DtsParser,
        peripheral: &str,
        property_regex: &str,
        new_line: String,
        should_add: bool,
    ) -> Result<(), String> {
        let content = parser.get_file_content();

        let (start, end) = DtsParser::find_node_position_in_content(content, peripheral)
            .ok_or_else(|| format!("未找到节点 {} 在文件中的位置", peripheral))?;

        let node_content = &content[start..end];
        let re = regex::Regex::new(property_regex)
            .map_err(|e| format!("无效的正则表达式 {}: {}", property_regex, e))?;

        let new_node_content = if re.is_match(node_content) {
            // Replace existing property
            re.replace_all(node_content, new_line.as_str()).to_string()
        } else if should_add {
            // Insert new property after the last semicolon in the node
            let last_semicolon_pos = node_content.rfind(';');
            if let Some(pos) = last_semicolon_pos {
                let mut modified = node_content.to_string();
                modified.insert_str(pos + 1, &new_line);
                modified
            } else {
                node_content.to_string()
            }
        } else {
            // No change needed
            return Ok(());
        };

        // Splice back into file content
        let mut new_content = content.to_string();
        new_content.replace_range(start..end, &new_node_content);

        // Re-parse with the updated content
        parser.load_content(&new_content);
        Ok(())
    }

    /// Get the previous (pre-modification) sysdma channels from the current file content.
    fn get_previous_sysdma_channels(parser: &DtsParser) -> Vec<String> {
        let default_channels = PeripheralInfo::default_sysdma_channels();

        let content = parser.get_file_content();
        let node_pos = DtsParser::find_node_position_in_content(content, "sysdma_remap");

        if let Some((start, end)) = node_pos {
            let node_content = &content[start..end];
            let re = regex::Regex::new(r"ch-remap\s*=\s*<([^>]+)>;").unwrap();

            if let Some(caps) = re.captures(node_content) {
                let ch_content = caps[1].trim();
                let channels: Vec<String> = ch_content
                    .split_whitespace()
                    .map(|ch| {
                        if ch.starts_with("CVI_") {
                            SysdmaChannelMap::channel_number(ch)
                        } else {
                            ch.to_string()
                        }
                    })
                    .collect();

                if !channels.is_empty() {
                    return channels;
                }
            }
        }

        default_channels
    }

    /// Create a new sysdma_remap node in the file content — mirrors C++ `createSysdmaRemapNode`.
    fn create_sysdma_remap_node(parser: &mut DtsParser, channels: &[String]) {
        let channel_names: Vec<String> = channels.iter()
            .map(|ch| SysdmaChannelMap::channel_name(ch))
            .collect();

        let ch_remap_value = if channel_names.len() >= 8 {
            let first_line: Vec<&str> = channel_names[0..4].iter().map(|s| s.as_str()).collect();
            let second_line: Vec<&str> = channel_names[4..8].iter().map(|s| s.as_str()).collect();
            format!("{}\n\t\t\t\t\t{}", first_line.join(" "), second_line.join(" "))
        } else {
            channel_names.join(" ")
        };

        let status = parser.get_peripheral("sysdma_remap")
            .map(|i| i.status.as_str())
            .unwrap_or("okay");

        let new_node = format!(
            "\n\nsysdma_remap {{\n\
             \tcompatible = \"cvitek,sysdma_remap\";\n\
             \treg = <0x0 0x03000154 0x0 0x10>;\n\
             \tstatus = \"{}\";\n\
             \tch-remap = <{}>;\n\
             \tint_mux_base = <0x03000298>;\n\
             \tint_mux = <0x1FF>; /* enable bit [0..8] for CPU0(CA53) */\n\
             }};\n",
            status, ch_remap_value
        );

        let mut content = parser.get_file_content().to_string();
        content.push_str(&new_node);
        parser.load_content(&content);
    }

    /// Update DMA configurations for peripherals affected by sysdma channel changes.
    fn update_peripheral_dma_config(
        parser: &mut DtsParser,
        previous_channels: &[String],
        new_channels: &[String],
    ) {
        let default_channels = PeripheralInfo::default_sysdma_channels();

        // Step 1: Clear DMA configs for peripherals whose channel mapping changed
        Self::clear_changed_peripheral_dma_configs(parser, previous_channels, new_channels);

        // Step 2: Add DMA configs for newly mapped peripherals
        let mut peripheral_channels: std::collections::HashMap<String, Vec<(usize, String)>> =
            std::collections::HashMap::new();

        for (i, channel_number) in new_channels.iter().enumerate().take(8) {
            let default_channel = default_channels.get(i).cloned().unwrap_or_default();

            if channel_number != &default_channel {
                let peripheral_node = SysdmaChannelMap::peripheral_node_from_channel(channel_number);
                if let Some(node) = peripheral_node {
                    peripheral_channels.entry(node)
                        .or_default()
                        .push((i, channel_number.clone()));
                }
            }
        }

        // For each peripheral with modified channels, add DMA config
        for (peripheral_node, channels) in peripheral_channels {
            if channels.len() == 1 {
                let (channel_idx, _) = channels[0];
                let is_rx = channel_idx % 2 == 0;
                let channel_type = if is_rx { "rx" } else { "tx" };
                Self::add_dma_config_to_peripheral(parser, &peripheral_node, &channel_idx.to_string(), "", channel_type);
            } else if channels.len() == 2 {
                let (idx1, num1) = channels[0].clone();
                let (idx2, num2) = channels[1].clone();

                let is_rx1 = SysdmaChannelMap::is_channel_rx(&num1);
                let is_rx2 = SysdmaChannelMap::is_channel_rx(&num2);

                let (rx_index, tx_index) = if is_rx1 && !is_rx2 {
                    (idx1.to_string(), idx2.to_string())
                } else if !is_rx1 && is_rx2 {
                    (idx2.to_string(), idx1.to_string())
                } else {
                    if idx1 % 2 == 0 {
                        (idx1.to_string(), idx2.to_string())
                    } else {
                        (idx2.to_string(), idx1.to_string())
                    }
                };

                Self::add_dma_config_to_peripheral(parser, &peripheral_node, &rx_index, &tx_index, "txrx");
            }
        }
    }

    /// Clear DMA configs for peripherals whose sysdma channel mapping changed.
    fn clear_changed_peripheral_dma_configs(
        parser: &mut DtsParser,
        previous_channels: &[String],
        new_channels: &[String],
    ) {
        let mut peripherals_to_clean: std::collections::HashSet<String> = std::collections::HashSet::new();

        for i in 0..8 {
            let prev = previous_channels.get(i).cloned().unwrap_or_default();
            let new_ch = new_channels.get(i).cloned().unwrap_or_default();

            if prev != new_ch && !prev.is_empty() {
                let peripheral_node = SysdmaChannelMap::peripheral_node_from_channel(&prev);
                if let Some(node) = peripheral_node {
                    peripherals_to_clean.insert(node);
                }
            }
        }

        for peripheral in peripherals_to_clean {
            Self::remove_dma_config_from_peripheral(parser, &peripheral);
        }
    }

    /// Add DMA configuration properties to a peripheral node.
    fn add_dma_config_to_peripheral(
        parser: &mut DtsParser,
        peripheral_node: &str,
        channel_index1: &str,
        channel_index2: &str,
        capability: &str,
    ) {
        let (dmas_line, dma_names_line, capability_line) = match capability {
            "txrx" => (
                format!("\n\t\tdmas = <&dmac {} 1 1\n\t\t\t&dmac {} 1 1>;", channel_index1, channel_index2),
                "\n\t\tdma-names = \"rx\", \"tx\";".to_string(),
                "\n\t\tcapability = \"txrx\";".to_string(),
            ),
            "rx" => (
                format!("\n\t\tdmas = <&dmac {} 1 1>;", channel_index1),
                "\n\t\tdma-names = \"rx\";".to_string(),
                "\n\t\tcapability = \"rx\";".to_string(),
            ),
            "tx" => (
                format!("\n\t\tdmas = <&dmac {} 1 1>;", channel_index1),
                "\n\t\tdma-names = \"tx\";".to_string(),
                "\n\t\tcapability = \"tx\";".to_string(),
            ),
            _ => return,
        };

        Self::update_or_add_property(parser, peripheral_node, r#"\s*dmas\s*=\s*<[^>]*>\s*;"#, dmas_line);
        Self::update_or_add_property(parser, peripheral_node, r#"\s*dma-names\s*=\s*[^;]+;"#, dma_names_line);
        Self::update_or_add_property(parser, peripheral_node, r#"\s*capability\s*=\s*[^;]+;"#, capability_line);
    }

    /// Remove DMA configuration from a peripheral node.
    fn remove_dma_config_from_peripheral(parser: &mut DtsParser, peripheral_node: &str) {
        Self::remove_property(parser, peripheral_node, r"\s*dmas\s*=\s*<[^>]*>\s*;");
        Self::remove_property(parser, peripheral_node, r#"\s*dma-names\s*=\s*[^;]+;"#);
        Self::remove_property(parser, peripheral_node, r#"\s*capability\s*=\s*[^;]+;"#);
    }

    /// Update or add a property line within a node.
    fn update_or_add_property(parser: &mut DtsParser, peripheral: &str, property_regex: &str, new_line: String) {
        let content = parser.get_file_content();

        if let Some((start, end)) = DtsParser::find_node_position_in_content(content, peripheral) {
            let node_content = &content[start..end];
            let re = regex::Regex::new(property_regex).unwrap();

            let new_node_content = if re.is_match(node_content) {
                re.replace_all(node_content, new_line.as_str()).to_string()
            } else {
                let last_semicolon_pos = node_content.rfind(';');
                if let Some(pos) = last_semicolon_pos {
                    let mut modified = node_content.to_string();
                    modified.insert_str(pos + 1, &new_line);
                    modified
                } else {
                    node_content.to_string()
                }
            };

            let mut new_content = content.to_string();
            new_content.replace_range(start..end, &new_node_content);
            parser.load_content(&new_content);
        }
    }

    /// Remove a property from a node.
    fn remove_property(parser: &mut DtsParser, peripheral: &str, property_regex: &str) {
        let content = parser.get_file_content();

        if let Some((start, end)) = DtsParser::find_node_position_in_content(content, peripheral) {
            let node_content = &content[start..end];
            let re = regex::Regex::new(property_regex).unwrap();

            if re.is_match(node_content) {
                let new_node_content = re.replace_all(node_content, "").to_string();
                let mut new_content = content.to_string();
                new_content.replace_range(start..end, &new_node_content);
                parser.load_content(&new_content);
            }
        }
    }
}

// Need this import for the default_sysdma_channels() call
use crate::peripheral::PeripheralInfo;

#[cfg(test)]
mod tests {
    use super::*;
    use crate::dts_parser::DtsParser;

    #[test]
    fn test_update_status_okay_to_disabled() {
        let content = "&i2c0 {\n\tstatus = \"okay\";\n\tclock-frequency = <100000>;\n};\n";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        DtsWriter::update_status(&mut parser, "i2c0", "disabled").unwrap();

        let new_content = parser.get_file_content();
        assert!(new_content.contains("status = \"disabled\";"));
        assert!(new_content.contains("clock-frequency = <100000>;")); // unchanged
    }

    #[test]
    fn test_update_status_disabled_to_okay() {
        let content = "&uart0 {\n\tstatus = \"disabled\";\n};\n";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        DtsWriter::update_status(&mut parser, "uart0", "okay").unwrap();

        let new_content = parser.get_file_content();
        assert!(new_content.contains("status = \"okay\";"));
    }

    #[test]
    fn test_update_clock_frequency() {
        let content = "&i2c0 {\n\tstatus = \"okay\";\n\tclock-frequency = <100000>;\n};\n";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        DtsWriter::update_clock_frequency(&mut parser, "i2c0", 400000).unwrap();

        let new_content = parser.get_file_content();
        assert!(new_content.contains("clock-frequency = <400000>;"));
        assert!(new_content.contains("status = \"okay\";")); // unchanged
    }

    #[test]
    fn test_update_pwm_cells() {
        let content = "&pwm0 {\n\tstatus = \"okay\";\n\t#pwm-cells = <3>;\n};\n";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        DtsWriter::update_pwm_cells(&mut parser, "pwm0", 2).unwrap();

        let new_content = parser.get_file_content();
        assert!(new_content.contains("#pwm-cells = <2>;"));
    }

    #[test]
    fn test_update_current_speed() {
        let content = "&uart0 {\n\tstatus = \"okay\";\n\tcurrent-speed = <115200>;\n};\n";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        DtsWriter::update_current_speed(&mut parser, "uart0", 9600).unwrap();

        let new_content = parser.get_file_content();
        assert!(new_content.contains("current-speed = <9600>;"));
        assert!(new_content.contains("status = \"okay\";")); // unchanged
    }

    #[test]
    fn test_write_preserves_comments() {
        let content = "&i2c0 {\n\t/* important comment */\n\tstatus = \"okay\";\n\tclock-frequency = <100000>; /* frequency */\n};\n";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        DtsWriter::update_status(&mut parser, "i2c0", "disabled").unwrap();

        let new_content = parser.get_file_content();
        assert!(new_content.contains("/* important comment */"));
        assert!(new_content.contains("/* frequency */"));
        assert!(new_content.contains("status = \"disabled\";"));
    }

    #[test]
    fn test_write_preserves_unrelated_nodes() {
        let content = "\
&i2c0 {
\tstatus = \"okay\";
\tclock-frequency = <100000>;
};
&uart0 {
\tstatus = \"disabled\";
\tcurrent-speed = <9600>;
};\n";

        let mut parser = DtsParser::new();
        parser.load_content(content);

        DtsWriter::update_status(&mut parser, "i2c0", "disabled").unwrap();

        let new_content = parser.get_file_content();
        assert!(new_content.contains("status = \"disabled\";")); // i2c0 changed
        assert!(new_content.contains("current-speed = <9600>;")); // uart0 unchanged
    }

    #[test]
    fn test_sysdma_channel_update() {
        let content = "\
sysdma_remap {
\tch-remap = <0 5 12 13 42 42 4 7>;
\tstatus = \"okay\";
};\n";

        let mut parser = DtsParser::new();
        parser.load_content(content);

        let new_channels: Vec<String> = vec![
            "0", "5", "12", "13", "42", "42", "4", "7"
        ].into_iter().map(|s| s.to_string()).collect();

        DtsWriter::update_sysdma_channels(&mut parser, "sysdma_remap", new_channels).unwrap();

        let new_content = parser.get_file_content();
        assert!(new_content.contains("ch-remap"));
    }

    #[test]
    fn test_update_nonexistent_peripheral_returns_error() {
        let content = "&i2c0 {\n\tstatus = \"okay\";\n};\n";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        let result = DtsWriter::update_status(&mut parser, "nonexistent", "disabled");
        assert!(result.is_err());
    }

    #[test]
    fn test_sysdma_channel_update_and_dma_cascade() {
        let content = "\
&uart0 {
	status = \"okay\";
	current-speed = <115200>;
};
&i2c0 {
	status = \"okay\";
	clock-frequency = <100000>;
};
sysdma_remap {
	ch-remap = <0 5 12 13 42 42 4 7>;
	status = \"okay\";
};
";
        let mut parser = DtsParser::new();
        parser.load_content(content);

        // 修改通道：将 12 更改为 8（这是 uart0_rx）
        // 这样会导致 uart0 (因为映射了 8) 添加 DMA 配置。
        // 原本默认的 12 不再是 12，那么之前映射了 12 的 uart2 应该被清除 DMA 配置。
        let new_channels: Vec<String> = vec![
            "0", "5", "8", "13", "42", "42", "4", "7"
        ].into_iter().map(|s| s.to_string()).collect();

        DtsWriter::update_sysdma_channels(&mut parser, "sysdma_remap", new_channels).unwrap();

        let new_content = parser.get_file_content();
        
        // 检查 uart0 应该被加入了单 rx 的 DMA 配置
        assert!(new_content.contains("&uart0 {"));
        assert!(new_content.contains("dmas = <&dmac 2 1 1>;")); // 对应 channel_index1 = 2 (因为通道 8 在 new_channels 的索引是 2)
        assert!(new_content.contains("dma-names = \"rx\";"));
        assert!(new_content.contains("capability = \"rx\";"));
    }
}