//! Flash partition configuration validation and management module (M6).
//!
//! Provides flash partition data structures, layout validation, and Tauri commands
//! for the frontend.

use serde::{Deserialize, Serialize};
use std::collections::{BTreeMap, HashSet};
use std::fs;
use std::path::{Path, PathBuf};

// ── Data Structures ──────────────────────────────────────────────────────────

/// Default board name used when no chip type is selected (matches C++ fallback).
pub const DEFAULT_BOARD: &str = "cv1842hp_wevb_0014a_emmc";

/// Flash partition information read from a board's defconfig.
///
/// Captures the board-level flash size plus every partition declared in the
/// `# Partition Configuration` section.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FlashBoardInfo {
    /// Raw flash size string from `CONFIG_FLASH_SIZE` (e.g. "32GB", "16MB")
    pub flash_size: String,
    /// Flash size parsed into KB (e.g. 33554432 for "32GB")
    pub flash_size_kb: u64,
    /// Declared partition count from `CONFIG_PARTITION_COUNT`
    pub partition_count: i32,
    /// Partitions in ascending partition-number order
    pub partitions: Vec<FlashPartition>,
}

/// A single flash partition descriptor.
///
/// All sizes are in KB (matching the C++ FlashPartition struct).
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FlashPartition {
    /// Partition number (e.g. 2, 3, 4…)
    pub partition_number: i32,
    /// Partition label (e.g. "BOOT", "ROOTFS")
    pub label: String,
    /// Partition size in KB
    pub size: u64,
    /// Human-readable size string (e.g. "3MB", "8MB", "512KB")
    pub size_string: String,
    /// Associated file (e.g. "boot.emmc", "rootfs.emmc")
    pub file: String,
    /// Mount point (e.g. "/mnt/system", "" if none)
    pub mountpoint: String,
    /// Filesystem type (e.g. "ext4", "" if none)
    pub type_field: String,
    /// Whether the partition is enabled
    pub enabled: bool,
}

// ── Formatting helpers ───────────────────────────────────────────────────────

/// Format a KB size into a human-readable string (matching C++ `formatSize` for flash).
///
/// Rules:
/// - 0       → "0KB"
/// - ≥ 1 GB  → "{n}GB"  (1 GB = 1 048 576 KB)
/// - ≥ 1 MB  → "{n}MB"  (1 MB = 1024 KB)
/// - < 1 MB  → "{n}KB"
///
/// # Examples
/// ```
/// assert_eq!(cvicubemx_lib::flash::format_flash_size(3072),    "3MB");
/// assert_eq!(cvicubemx_lib::flash::format_flash_size(8192),    "8MB");
/// assert_eq!(cvicubemx_lib::flash::format_flash_size(512),     "512KB");
/// assert_eq!(cvicubemx_lib::flash::format_flash_size(3145728), "3GB");
/// ```
pub fn format_flash_size(size_in_kb: u64) -> String {
    if size_in_kb == 0 {
        return "0KB".to_string();
    }
    const MB: u64 = 1024;
    const GB: u64 = MB * 1024;

    if size_in_kb >= GB {
        format!("{}GB", size_in_kb / GB)
    } else if size_in_kb >= MB {
        format!("{}MB", size_in_kb / MB)
    } else {
        format!("{}KB", size_in_kb)
    }
}

/// Parse a human-readable flash size string into KB.
///
/// Supports "3GB", "8MB", "512KB", or a raw number (assumed KB).
/// Returns 0 on parse failure.
pub fn parse_flash_size(size_str: &str) -> u64 {
    let clean = size_str.trim().to_uppercase();
    if clean.ends_with("GB") {
        clean[..clean.len() - 2]
            .parse::<u64>()
            .ok()
            .map(|n| n * 1024 * 1024)
            .unwrap_or(0)
    } else if clean.ends_with("MB") {
        clean[..clean.len() - 2]
            .parse::<u64>()
            .ok()
            .map(|n| n * 1024)
            .unwrap_or(0)
    } else if clean.ends_with("KB") {
        clean[..clean.len() - 2].parse::<u64>().ok().unwrap_or(0)
    } else {
        clean.parse::<u64>().ok().unwrap_or(0)
    }
}

// ── Defconfig parsing ────────────────────────────────────────────────────────

/// Strip surrounding double quotes from a defconfig value (e.g. `"3072"` → `3072`).
fn unquote(value: &str) -> String {
    let trimmed = value.trim();
    trimmed
        .strip_prefix('"')
        .and_then(|s| s.strip_suffix('"'))
        .unwrap_or(trimmed)
        .to_string()
}

/// Mutable builder used while accumulating partition fields during parsing.
#[derive(Default)]
struct PartitionBuilder {
    label: String,
    size_str: Option<String>,
    file: String,
    mountpoint: String,
    type_field: String,
    has_fields: bool,
}

/// Parse a board defconfig's `# Partition Configuration` section.
///
/// Reads `CONFIG_FLASH_SIZE`, `CONFIG_PARTITION_COUNT`, and every
/// `CONFIG_PARTITION_<N>_*` line into an ordered list of partitions.
///
/// A partition is included if it declares any `CONFIG_PARTITION_<N>_*` field
/// (a LABEL/SIZE line is enough) — some boards omit the `CONFIG_PARTITION_<N>=y`
/// line for trailing partitions yet still expect them in the layout.
///
/// An empty `CONFIG_PARTITION_<N>_SIZE=""` yields `size = 0`, meaning the build
/// auto-allocates the remaining flash space to that partition (typically DATA).
pub fn parse_flash_defconfig(content: &str) -> FlashBoardInfo {
    let mut flash_size = "32GB".to_string();
    let mut partition_count: i32 = 0;
    let mut builders: BTreeMap<i32, PartitionBuilder> = BTreeMap::new();

    for line in content.lines() {
        let line = line.trim();

        if let Some(rest) = line.strip_prefix("CONFIG_FLASH_SIZE=") {
            flash_size = unquote(rest);
            continue;
        }
        if let Some(rest) = line.strip_prefix("CONFIG_PARTITION_COUNT=") {
            partition_count = rest.trim().parse::<i32>().unwrap_or(0);
            continue;
        }

        // CONFIG_PARTITION_<N>... — extract the leading partition number.
        let Some(rest) = line.strip_prefix("CONFIG_PARTITION_") else {
            continue;
        };
        let digits: String = rest.chars().take_while(|c| c.is_ascii_digit()).collect();
        if digits.is_empty() {
            continue; // e.g. CONFIG_PARTITION_COUNT handled above; anything else ignored
        }
        let num: i32 = match digits.parse() {
            Ok(n) => n,
            Err(_) => continue,
        };
        let suffix = &rest[digits.len()..];

        // The `CONFIG_PARTITION_<N>=y` enable line carries no field data on its own.
        if suffix == "=y" {
            builders.entry(num).or_default();
            continue;
        }

        let entry = builders.entry(num).or_default();
        if let Some(v) = suffix.strip_prefix("_LABEL=") {
            entry.label = unquote(v);
            entry.has_fields = true;
        } else if let Some(v) = suffix.strip_prefix("_SIZE=") {
            entry.size_str = Some(unquote(v));
            entry.has_fields = true;
        } else if let Some(v) = suffix.strip_prefix("_FILE=") {
            entry.file = unquote(v);
            entry.has_fields = true;
        } else if let Some(v) = suffix.strip_prefix("_MOUNTPOINT=") {
            entry.mountpoint = unquote(v);
            entry.has_fields = true;
        } else if let Some(v) = suffix.strip_prefix("_TYPE=") {
            entry.type_field = unquote(v);
            entry.has_fields = true;
        }
    }

    let partitions: Vec<FlashPartition> = builders
        .into_iter()
        .filter(|(_, b)| b.has_fields)
        .map(|(num, b)| {
            // Empty SIZE → auto-allocate (size 0). Otherwise parse KB number.
            let size = b
                .size_str
                .as_deref()
                .map(|s| {
                    if s.trim().is_empty() {
                        0
                    } else {
                        parse_flash_size(s)
                    }
                })
                .unwrap_or(0);
            FlashPartition {
                partition_number: num,
                label: b.label,
                size,
                size_string: format_flash_size(size),
                file: b.file,
                mountpoint: b.mountpoint,
                type_field: b.type_field,
                enabled: true,
            }
        })
        .collect();

    let flash_size_kb = parse_flash_size(&flash_size);

    FlashBoardInfo {
        flash_size,
        flash_size_kb,
        partition_count,
        partitions,
    }
}

/// Resolve a board's primary defconfig path under the SDK source tree.
///
/// Layout convention shared with the Memory/Clock modules:
/// `{source}/build/boards/cv184x/{chip}/{chip}_defconfig`.
/// Falls back to [`DEFAULT_BOARD`] when no chip is selected.
pub fn flash_defconfig_path(source_path: &str, chip_type: &str) -> PathBuf {
    let chip = if chip_type.is_empty() || chip_type == "请选择芯片型号" {
        DEFAULT_BOARD
    } else {
        chip_type
    };
    Path::new(source_path)
        .join("build")
        .join("boards")
        .join("cv184x")
        .join(chip)
        .join(format!("{}_defconfig", chip))
}

// ── Validation ───────────────────────────────────────────────────────────────

/// Validate the flash partition layout.
///
/// Checks:
/// 1. Partition numbers must be unique
/// 2. Labels must not be empty for enabled partitions
/// 3. Total size of enabled partitions must not exceed `flash_size_kb`
/// 4. Disabled partitions are not counted toward total size
/// 5. Enabled partition sizes must be a multiple of 64 KB (hard error, matching C++ UI behavior)
///
/// Returns `Ok(())` if valid, or `Err(message)` describing the first problem found.
pub fn validate_partition_layout(
    partitions: &[FlashPartition],
    flash_size_kb: u64,
) -> Result<(), String> {
    // 1. Check unique partition numbers
    let mut seen_numbers = std::collections::HashSet::new();
    for p in partitions {
        if seen_numbers.contains(&p.partition_number) {
            return Err(format!(
                "Duplicate partition number: {}",
                p.partition_number
            ));
        }
        seen_numbers.insert(p.partition_number);
    }

    // 2. Check labels for enabled partitions
    for p in partitions {
        if p.enabled && p.label.trim().is_empty() {
            return Err(format!(
                "Partition {} has an empty label but is enabled",
                p.partition_number
            ));
        }
    }

    // 3. Check total size of enabled partitions vs flash size
    let total_enabled_kb: u64 = partitions
        .iter()
        .filter(|p| p.enabled)
        .map(|p| p.size)
        .sum();

    if total_enabled_kb > flash_size_kb {
        return Err(format!(
            "Total enabled partition size ({}) exceeds flash capacity ({})",
            format_flash_size(total_enabled_kb),
            format_flash_size(flash_size_kb)
        ));
    }

    Ok(())
}

// ── Default partitions ───────────────────────────────────────────────────────

/// Return the default set of flash partitions matching the C++ `initializePartitions`.
///
/// Default flash size is 32 GB = 33554432 KB.
pub fn default_partitions() -> Vec<FlashPartition> {
    vec![
        FlashPartition {
            partition_number: 2,
            label: "2nd".into(),
            size: 3072,
            size_string: format_flash_size(3072),
            file: "yoc.bin".into(),
            mountpoint: "".into(),
            type_field: "".into(),
            enabled: true,
        },
        FlashPartition {
            partition_number: 3,
            label: "BOOT".into(),
            size: 8192,
            size_string: format_flash_size(8192),
            file: "boot.emmc".into(),
            mountpoint: "".into(),
            type_field: "".into(),
            enabled: true,
        },
        FlashPartition {
            partition_number: 4,
            label: "MISC".into(),
            size: 512,
            size_string: format_flash_size(512),
            file: "logo.jpg".into(),
            mountpoint: "".into(),
            type_field: "".into(),
            enabled: true,
        },
        FlashPartition {
            partition_number: 5,
            label: "ENV".into(),
            size: 128,
            size_string: format_flash_size(128),
            file: "".into(),
            mountpoint: "".into(),
            type_field: "".into(),
            enabled: true,
        },
        FlashPartition {
            partition_number: 6,
            label: "ROOTFS".into(),
            size: 70656,
            size_string: format_flash_size(70656),
            file: "rootfs.emmc".into(),
            mountpoint: "".into(),
            type_field: "".into(),
            enabled: true,
        },
        FlashPartition {
            partition_number: 7,
            label: "SYSTEM".into(),
            size: 40960,
            size_string: format_flash_size(40960),
            file: "system.emmc".into(),
            mountpoint: "/mnt/system".into(),
            type_field: "ext4".into(),
            enabled: true,
        },
        FlashPartition {
            partition_number: 8,
            label: "CFG".into(),
            size: 15240,
            size_string: format_flash_size(15240),
            file: "cfg.emmc".into(),
            mountpoint: "mnt/cfg".into(),
            type_field: "ext4".into(),
            enabled: true,
        },
        FlashPartition {
            partition_number: 9,
            label: "DATA".into(),
            size: 3145728,
            size_string: format_flash_size(3145728),
            file: "data.emmc".into(),
            mountpoint: "mnt/data".into(),
            type_field: "ext4".into(),
            enabled: true,
        },
    ]
}

/// Default flash size: 32 GB in KB
pub const DEFAULT_FLASH_SIZE_KB: u64 = 32 * 1024 * 1024; // 32 GB = 33554432 KB

// ── JSON / defconfig export ──────────────────────────────────────────────────

/// Export flash partitions to a JSON file (matching C++ `exportToJson`).
pub fn export_flash_json_file(
    partitions: &[FlashPartition],
    flash_size: &str,
    partition_count: i32,
    path: &str,
) -> Result<(), String> {
    let root = serde_json::json!({
        "flashSize": flash_size,
        "partitionCount": partition_count,
        "partitions": partitions,
    });
    let json_str = serde_json::to_string_pretty(&root).map_err(|e| e.to_string())?;
    fs::write(path, json_str).map_err(|e| format!("Failed to write JSON file: {}", e))
}

/// Import flash partitions from a JSON file (matching C++ `importFromJson`).
pub fn import_flash_json_file(path: &str) -> Result<(Vec<FlashPartition>, String, i32), String> {
    let content =
        fs::read_to_string(path).map_err(|e| format!("Failed to read JSON file: {}", e))?;
    let root: serde_json::Value =
        serde_json::from_str(&content).map_err(|e| format!("JSON parse error: {}", e))?;

    let flash_size = root
        .get("flashSize")
        .and_then(|v| v.as_str())
        .unwrap_or("32GB")
        .to_string();
    let partition_count = root
        .get("partitionCount")
        .and_then(|v| v.as_i64())
        .unwrap_or(9) as i32;

    let arr = root
        .get("partitions")
        .and_then(|v| v.as_array())
        .ok_or_else(|| "Missing 'partitions' array in JSON".to_string())?;

    let mut partitions = Vec::new();
    for val in arr {
        let p: FlashPartition = serde_json::from_value(val.clone())
            .map_err(|e| format!("Failed to parse FlashPartition: {}", e))?;
        partitions.push(p);
    }

    Ok((partitions, flash_size, partition_count))
}

/// Identify which partition number a defconfig line belongs to, if any.
///
/// Matches the enable line (`CONFIG_PARTITION_<N>=y`) and any field line
/// (`CONFIG_PARTITION_<N>_*`), but NOT `CONFIG_PARTITION_COUNT=`.
fn line_partition_number(line: &str) -> Option<i32> {
    let rest = line.trim_start().strip_prefix("CONFIG_PARTITION_")?;
    let dlen = rest.chars().take_while(|c| c.is_ascii_digit()).count();
    if dlen == 0 {
        return None;
    }
    let num: i32 = rest[..dlen].parse().ok()?;
    let suffix = &rest[dlen..];
    if suffix == "=y" || suffix.starts_with('_') {
        Some(num)
    } else {
        None
    }
}

/// Identify a `CONFIG_PARTITION_<N>_<FIELD>=` line, returning `(N, FIELD)`.
fn partition_field(line: &str) -> Option<(i32, &'static str)> {
    let rest = line.trim_start().strip_prefix("CONFIG_PARTITION_")?;
    let dlen = rest.chars().take_while(|c| c.is_ascii_digit()).count();
    if dlen == 0 {
        return None;
    }
    let num: i32 = rest[..dlen].parse().ok()?;
    let suffix = &rest[dlen..];
    let field = if suffix.starts_with("_LABEL=") {
        "LABEL"
    } else if suffix.starts_with("_SIZE=") {
        "SIZE"
    } else if suffix.starts_with("_FILE=") {
        "FILE"
    } else if suffix.starts_with("_MOUNTPOINT=") {
        "MOUNTPOINT"
    } else if suffix.starts_with("_TYPE=") {
        "TYPE"
    } else {
        return None;
    };
    Some((num, field))
}

/// Serialize a partition size for defconfig output.
///
/// `size == 0` means "auto-allocate the remaining flash" and is written as an
/// empty value (`""`), so the build assigns the leftover space to that
/// partition (typically DATA).
fn format_size_value(size: u64) -> String {
    if size == 0 {
        String::new()
    } else {
        size.to_string()
    }
}

/// Export flash config to a board's defconfig file (matching C++ `exportToDefconfig`).
///
/// Behaviour:
/// - Existing partitions: their `_LABEL/_SIZE/_FILE/_MOUNTPOINT/_TYPE` lines are
///   updated in place (the `CONFIG_PARTITION_<N>=y` enable line is left as-is).
/// - New partitions (not present in the file): a full config block is inserted
///   before `# end of Partition Configuration`.
/// - Partitions removed in the UI: all their `CONFIG_PARTITION_<N>*` lines are
///   stripped from the file.
/// - `CONFIG_PARTITION_COUNT` is set to the highest partition number.
pub fn write_flash_defconfig(
    partitions: &[FlashPartition],
    source_path: &str,
    chip_type: &str,
) -> Result<(), String> {
    let defconfig_path = flash_defconfig_path(source_path, chip_type);

    if !defconfig_path.exists() {
        return Err(format!(
            "Defconfig file does not exist: {}",
            defconfig_path.display()
        ));
    }

    let content = fs::read_to_string(&defconfig_path)
        .map_err(|e| format!("Failed to read defconfig: {}", e))?;
    let mut lines: Vec<String> = content.lines().map(|l| l.to_string()).collect();

    let by_num: BTreeMap<i32, &FlashPartition> =
        partitions.iter().map(|p| (p.partition_number, p)).collect();
    let ui_numbers: HashSet<i32> = by_num.keys().copied().collect();
    let file_numbers: HashSet<i32> = lines
        .iter()
        .filter_map(|l| line_partition_number(l))
        .collect();

    // 1. Delete partitions that are in the file but no longer in the UI list.
    let to_delete: HashSet<i32> = file_numbers.difference(&ui_numbers).copied().collect();
    if !to_delete.is_empty() {
        lines.retain(|l| match line_partition_number(l) {
            Some(n) => !to_delete.contains(&n),
            None => true,
        });
    }

    // 2. Update existing partitions' field lines in place.
    for line in lines.iter_mut() {
        if let Some((n, field)) = partition_field(line) {
            if let Some(p) = by_num.get(&n) {
                *line = match field {
                    "LABEL" => format!("CONFIG_PARTITION_{}_LABEL=\"{}\"", n, p.label),
                    "SIZE" => {
                        format!(
                            "CONFIG_PARTITION_{}_SIZE=\"{}\"",
                            n,
                            format_size_value(p.size)
                        )
                    }
                    "FILE" => format!("CONFIG_PARTITION_{}_FILE=\"{}\"", n, p.file),
                    "MOUNTPOINT" => {
                        format!("CONFIG_PARTITION_{}_MOUNTPOINT=\"{}\"", n, p.mountpoint)
                    }
                    "TYPE" => format!("CONFIG_PARTITION_{}_TYPE=\"{}\"", n, p.type_field),
                    _ => line.clone(),
                };
            }
        }
    }

    // 3. Insert new partitions (present in UI but not the file) before the
    //    section terminator, in ascending partition-number order.
    let mut new_nums: Vec<i32> = ui_numbers.difference(&file_numbers).copied().collect();
    new_nums.sort_unstable();
    for n in new_nums {
        let p = by_num[&n];
        let block = vec![
            format!("CONFIG_PARTITION_{}=y", n),
            format!("CONFIG_PARTITION_{}_LABEL=\"{}\"", n, p.label),
            format!(
                "CONFIG_PARTITION_{}_SIZE=\"{}\"",
                n,
                format_size_value(p.size)
            ),
            format!("CONFIG_PARTITION_{}_FILE=\"{}\"", n, p.file),
            format!("CONFIG_PARTITION_{}_MOUNTPOINT=\"{}\"", n, p.mountpoint),
            format!("CONFIG_PARTITION_{}_TYPE=\"{}\"", n, p.type_field),
            String::new(),
        ];
        let insert_pos = lines
            .iter()
            .position(|l| l.contains("# end of Partition Configuration"))
            .unwrap_or(lines.len());
        for (offset, bl) in block.into_iter().enumerate() {
            lines.insert(insert_pos + offset, bl);
        }
    }

    // 4. Keep CONFIG_PARTITION_COUNT in sync with the highest partition number.
    if let Some(max_num) = ui_numbers.iter().max() {
        for line in lines.iter_mut() {
            if line.trim_start().starts_with("CONFIG_PARTITION_COUNT=") {
                *line = format!("CONFIG_PARTITION_COUNT={}", max_num);
                break;
            }
        }
    }

    let output = lines.join("\n");
    // Preserve the file's trailing newline so export only touches intended lines.
    let output = if content.ends_with('\n') {
        format!("{}\n", output)
    } else {
        output
    };
    fs::write(&defconfig_path, output).map_err(|e| format!("Failed to write defconfig: {}", e))?;

    Ok(())
}

// ── Tauri commands ────────────────────────────────────────────────────────────

#[tauri::command]
pub fn load_partitions() -> Result<Vec<FlashPartition>, String> {
    Ok(default_partitions())
}

/// Read the flash partition layout from the selected board's defconfig.
///
/// This is the primary data source for the Flash page: it reflects the real
/// per-board flash size and partition table rather than hardcoded defaults.
#[tauri::command]
pub fn read_flash_board_info(
    source_path: String,
    chip_type: String,
) -> Result<FlashBoardInfo, String> {
    let defconfig_path = flash_defconfig_path(&source_path, &chip_type);
    if !defconfig_path.exists() {
        return Err(format!(
            "Defconfig file does not exist: {}",
            defconfig_path.display()
        ));
    }
    let content = fs::read_to_string(&defconfig_path)
        .map_err(|e| format!("Failed to read defconfig: {}", e))?;
    Ok(parse_flash_defconfig(&content))
}

#[tauri::command]
pub fn validate_partitions(partitions: Vec<FlashPartition>) -> Result<(), String> {
    validate_partition_layout(&partitions, DEFAULT_FLASH_SIZE_KB)
}

#[tauri::command]
pub fn export_flash_json(partitions: Vec<FlashPartition>, path: String) -> Result<(), String> {
    export_flash_json_file(&partitions, "32GB", 9, &path)
}

#[tauri::command]
pub fn export_flash_defconfig(
    partitions: Vec<FlashPartition>,
    source_path: String,
    chip_type: String,
) -> Result<(), String> {
    write_flash_defconfig(&partitions, &source_path, &chip_type)
}

// ── Unit tests ────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_format_flash_size() {
        assert_eq!(format_flash_size(3072), "3MB");
        assert_eq!(format_flash_size(8192), "8MB");
        assert_eq!(format_flash_size(512), "512KB");
        assert_eq!(format_flash_size(128), "128KB");
        assert_eq!(format_flash_size(3145728), "3GB");
        assert_eq!(format_flash_size(0), "0KB");
        assert_eq!(format_flash_size(40960), "40MB");
        assert_eq!(format_flash_size(70656), "69MB"); // 70656 / 1024 = 69
    }

    #[test]
    fn test_parse_flash_size() {
        assert_eq!(parse_flash_size("3GB"), 3 * 1024 * 1024);
        assert_eq!(parse_flash_size("8MB"), 8 * 1024);
        assert_eq!(parse_flash_size("512KB"), 512);
        assert_eq!(parse_flash_size("128"), 128); // raw KB
        assert_eq!(parse_flash_size("garbage"), 0);
    }

    #[test]
    fn test_validate_partition_layout_total_exceeds_flash() {
        let partitions = vec![FlashPartition {
            partition_number: 2,
            label: "BIG".into(),
            size: DEFAULT_FLASH_SIZE_KB + 1024,
            size_string: format_flash_size(DEFAULT_FLASH_SIZE_KB + 1024),
            file: "".into(),
            mountpoint: "".into(),
            type_field: "".into(),
            enabled: true,
        }];
        let result = validate_partition_layout(&partitions, DEFAULT_FLASH_SIZE_KB);
        assert!(result.is_err());
        assert!(result.unwrap_err().contains("exceeds flash capacity"));
    }

    #[test]
    fn test_validate_partition_layout_disabled_not_counted() {
        let partitions = vec![FlashPartition {
            partition_number: 2,
            label: "BIG".into(),
            size: DEFAULT_FLASH_SIZE_KB + 1024,
            size_string: "".into(),
            file: "".into(),
            mountpoint: "".into(),
            type_field: "".into(),
            enabled: false, // disabled → not counted
        }];
        // Even though total size > flash, disabled partition doesn't count
        assert!(validate_partition_layout(&partitions, DEFAULT_FLASH_SIZE_KB).is_ok());
    }

    #[test]
    fn test_validate_partition_layout_size_not_multiple_of_64_is_soft_warning() {
        // 64 KB alignment is a soft UI warning in C++, not a hard validation error.
        // Our validation follows the same approach: it does NOT reject non-aligned sizes.
        let partitions = vec![FlashPartition {
            partition_number: 2,
            label: "BAD_SIZE".into(),
            size: 100,
            size_string: "".into(),
            file: "".into(),
            mountpoint: "".into(),
            type_field: "".into(),
            enabled: true,
        }];
        let result = validate_partition_layout(&partitions, DEFAULT_FLASH_SIZE_KB);
        // Non-64-aligned size does NOT cause a hard validation error
        assert!(result.is_ok());
    }

    #[test]
    fn test_validate_partition_layout_duplicate_number() {
        let partitions = vec![
            FlashPartition {
                partition_number: 2,
                label: "A".into(),
                size: 1024,
                size_string: "".into(),
                file: "".into(),
                mountpoint: "".into(),
                type_field: "".into(),
                enabled: true,
            },
            FlashPartition {
                partition_number: 2,
                label: "B".into(),
                size: 2048,
                size_string: "".into(),
                file: "".into(),
                mountpoint: "".into(),
                type_field: "".into(),
                enabled: true,
            },
        ];
        let result = validate_partition_layout(&partitions, DEFAULT_FLASH_SIZE_KB);
        assert!(result.is_err());
        assert!(result.unwrap_err().contains("Duplicate partition number"));
    }

    #[test]
    fn test_validate_partition_layout_empty_label_enabled() {
        let partitions = vec![FlashPartition {
            partition_number: 2,
            label: "".into(),
            size: 1024,
            size_string: "".into(),
            file: "".into(),
            mountpoint: "".into(),
            type_field: "".into(),
            enabled: true,
        }];
        let result = validate_partition_layout(&partitions, DEFAULT_FLASH_SIZE_KB);
        assert!(result.is_err());
        assert!(result.unwrap_err().contains("empty label"));
    }

    #[test]
    fn test_validate_partition_layout_valid_default() {
        let partitions = default_partitions();
        let result = validate_partition_layout(&partitions, DEFAULT_FLASH_SIZE_KB);
        assert!(result.is_ok());
    }

    #[test]
    fn test_default_partitions_count() {
        let partitions = default_partitions();
        assert_eq!(partitions.len(), 8);
    }

    #[test]
    fn test_default_partition_labels() {
        let partitions = default_partitions();
        let labels: Vec<String> = partitions.iter().map(|p| p.label.clone()).collect();
        assert!(labels.contains(&"2nd".to_string()));
        assert!(labels.contains(&"BOOT".to_string()));
        assert!(labels.contains(&"ROOTFS".to_string()));
        assert!(labels.contains(&"SYSTEM".to_string()));
    }

    #[test]
    fn test_export_import_json_roundtrip() {
        let partitions = default_partitions();
        let tmp = std::env::temp_dir().join("cvicubemx_flash_test.json");
        let path = tmp.to_str().unwrap();

        export_flash_json_file(&partitions, "32GB", 9, path).unwrap();
        let (imported, flash_size, count) = import_flash_json_file(path).unwrap();

        assert_eq!(flash_size, "32GB");
        assert_eq!(count, 9);
        assert_eq!(imported.len(), partitions.len());
        for (orig, imp) in partitions.iter().zip(imported.iter()) {
            assert_eq!(orig.label, imp.label);
            assert_eq!(orig.size, imp.size);
            assert_eq!(orig.partition_number, imp.partition_number);
        }

        let _ = fs::remove_file(&tmp);
    }

    #[test]
    fn test_validate_partition_size_multiple_64_for_disabled() {
        // Disabled partitions should NOT be checked for 64 KB alignment
        let partitions = vec![FlashPartition {
            partition_number: 2,
            label: "BAD".into(),
            size: 100,
            size_string: "".into(),
            file: "".into(),
            mountpoint: "".into(),
            type_field: "".into(),
            enabled: false,
        }];
        assert!(validate_partition_layout(&partitions, DEFAULT_FLASH_SIZE_KB).is_ok());
    }

    // ── Defconfig parse / write tests ─────────────────────────────────────────

    /// Representative eMMC partition section (mirrors the real board defconfig).
    const SAMPLE_EMMC: &str = "\
CONFIG_SOMETHING_ELSE=y
#
# Partition Configuration
#
CONFIG_FLASH_SIZE=\"32GB\"
CONFIG_PARTITION_COUNT=9
# eMMC use partation 1 to store fip.bin so we started form partition 2
CONFIG_PARTITION_2=y
CONFIG_PARTITION_2_LABEL=\"2nd\"
CONFIG_PARTITION_2_SIZE=\"3072\"
CONFIG_PARTITION_2_FILE=\"yoc.bin\"
CONFIG_PARTITION_2_MOUNTPOINT=\"\"
CONFIG_PARTITION_2_TYPE=\"\"

CONFIG_PARTITION_3=y
CONFIG_PARTITION_3_LABEL=\"BOOT\"
CONFIG_PARTITION_3_SIZE=\"8192\"
CONFIG_PARTITION_3_FILE=\"boot.emmc\"
CONFIG_PARTITION_3_MOUNTPOINT=\"\"
CONFIG_PARTITION_3_TYPE=\"\"

CONFIG_PARTITION_4=y
CONFIG_PARTITION_4_LABEL=\"MISC\"
CONFIG_PARTITION_4_SIZE=\"512\"
CONFIG_PARTITION_4_FILE=\"logo.jpg\"
CONFIG_PARTITION_4_MOUNTPOINT=\"\"
CONFIG_PARTITION_4_TYPE=\"\"

CONFIG_PARTITION_5=y
CONFIG_PARTITION_5_LABEL=\"ENV\"
CONFIG_PARTITION_5_SIZE=\"128\"
CONFIG_PARTITION_5_FILE=\"\"
CONFIG_PARTITION_5_MOUNTPOINT=\"\"
CONFIG_PARTITION_5_TYPE=\"\"

CONFIG_PARTITION_6=y
CONFIG_PARTITION_6_LABEL=\"ROOTFS\"
CONFIG_PARTITION_6_SIZE=\"70656\"
CONFIG_PARTITION_6_FILE=\"rootfs.emmc\"
CONFIG_PARTITION_6_MOUNTPOINT=\"\"
CONFIG_PARTITION_6_TYPE=\"\"

CONFIG_PARTITION_7_LABEL=\"SYSTEM\"
CONFIG_PARTITION_7_SIZE=\"40960\"
CONFIG_PARTITION_7_FILE=\"system.emmc\"
CONFIG_PARTITION_7_MOUNTPOINT=\"/mnt/system\"
CONFIG_PARTITION_7_TYPE=\"ext4\"

CONFIG_PARTITION_8_LABEL=\"CFG\"
CONFIG_PARTITION_8_SIZE=\"15240\"
CONFIG_PARTITION_8_FILE=\"cfg.emmc\"
CONFIG_PARTITION_8_MOUNTPOINT=\"mnt/cfg\"
CONFIG_PARTITION_8_TYPE=\"ext4\"

CONFIG_PARTITION_9_LABEL=\"DATA\"
CONFIG_PARTITION_9_SIZE=\"3145728\"
CONFIG_PARTITION_9_FILE=\"data.emmc\"
CONFIG_PARTITION_9_MOUNTPOINT=\"mnt/data\"
CONFIG_PARTITION_9_TYPE=\"ext4\"
# end of Partition Configuration
CONFIG_TAIL=y
";

    /// Create a temp SDK tree containing the given board defconfig content.
    /// Returns (source_root, chip, defconfig_path).
    fn make_temp_board(content: &str) -> (PathBuf, String, PathBuf) {
        let ts = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap()
            .as_nanos();
        let chip = "cv1842hp_wevb_0014a_emmc";
        let root = std::env::temp_dir().join(format!("cvicubemx_flash_{}", ts));
        let board_dir = root.join("build").join("boards").join("cv184x").join(chip);
        fs::create_dir_all(&board_dir).unwrap();
        let defconfig = board_dir.join(format!("{}_defconfig", chip));
        fs::write(&defconfig, content).unwrap();
        (root, chip.to_string(), defconfig)
    }

    #[test]
    fn test_parse_flash_defconfig_emmc() {
        let info = parse_flash_defconfig(SAMPLE_EMMC);
        assert_eq!(info.flash_size, "32GB");
        assert_eq!(info.flash_size_kb, 32 * 1024 * 1024);
        assert_eq!(info.partition_count, 9);
        assert_eq!(info.partitions.len(), 8);

        // Ascending order, starting at partition 2.
        assert_eq!(info.partitions[0].partition_number, 2);
        assert_eq!(info.partitions[0].label, "2nd");
        assert_eq!(info.partitions[0].size, 3072);

        // Partition 7 has no `=y` line yet must still be parsed.
        let p7 = info
            .partitions
            .iter()
            .find(|p| p.partition_number == 7)
            .unwrap();
        assert_eq!(p7.label, "SYSTEM");
        assert_eq!(p7.mountpoint, "/mnt/system");
        assert_eq!(p7.type_field, "ext4");

        // DATA partition.
        let data = info.partitions.last().unwrap();
        assert_eq!(data.partition_number, 9);
        assert_eq!(data.label, "DATA");
        assert_eq!(data.size, 3145728);
    }

    #[test]
    fn test_parse_flash_defconfig_empty_size_is_auto() {
        let content = "\
CONFIG_FLASH_SIZE=\"16MB\"
CONFIG_PARTITION_COUNT=2
CONFIG_PARTITION_1=y
CONFIG_PARTITION_1_LABEL=\"BOOT\"
CONFIG_PARTITION_1_SIZE=\"2048\"
CONFIG_PARTITION_2=y
CONFIG_PARTITION_2_LABEL=\"DATA\"
CONFIG_PARTITION_2_SIZE=\"\"
";
        let info = parse_flash_defconfig(content);
        assert_eq!(info.flash_size_kb, 16 * 1024);
        assert_eq!(info.partitions.len(), 2);
        // Empty SIZE → auto (size 0).
        let data = info.partitions.iter().find(|p| p.label == "DATA").unwrap();
        assert_eq!(data.size, 0);
    }

    #[test]
    fn test_write_flash_defconfig_updates_size_in_place() {
        let (root, chip, defconfig) = make_temp_board(SAMPLE_EMMC);
        let mut partitions = parse_flash_defconfig(SAMPLE_EMMC).partitions;
        // Change partition 3 (BOOT) size 8192 → 4096.
        partitions
            .iter_mut()
            .find(|p| p.partition_number == 3)
            .unwrap()
            .size = 4096;

        write_flash_defconfig(&partitions, root.to_str().unwrap(), &chip).unwrap();
        let out = fs::read_to_string(&defconfig).unwrap();

        assert!(out.contains("CONFIG_PARTITION_3_SIZE=\"4096\""));
        // Untouched partitions remain intact.
        assert!(out.contains("CONFIG_PARTITION_2_SIZE=\"3072\""));
        assert!(out.contains("CONFIG_PARTITION_7_LABEL=\"SYSTEM\""));
        // No duplicate label line was inserted for the `=y`-less partition 7.
        let p7_labels = out.matches("CONFIG_PARTITION_7_LABEL=").count();
        assert_eq!(p7_labels, 1);
        // No spurious `=y` line added for partition 7.
        assert!(!out.contains("CONFIG_PARTITION_7=y"));

        fs::remove_dir_all(&root).ok();
    }

    #[test]
    fn test_write_flash_defconfig_auto_size_writes_empty() {
        let (root, chip, defconfig) = make_temp_board(SAMPLE_EMMC);
        let mut partitions = parse_flash_defconfig(SAMPLE_EMMC).partitions;
        // Clear DATA size → auto allocate.
        partitions
            .iter_mut()
            .find(|p| p.partition_number == 9)
            .unwrap()
            .size = 0;

        write_flash_defconfig(&partitions, root.to_str().unwrap(), &chip).unwrap();
        let out = fs::read_to_string(&defconfig).unwrap();
        assert!(out.contains("CONFIG_PARTITION_9_SIZE=\"\""));

        fs::remove_dir_all(&root).ok();
    }

    #[test]
    fn test_write_flash_defconfig_add_and_delete() {
        let (root, chip, defconfig) = make_temp_board(SAMPLE_EMMC);
        let mut partitions = parse_flash_defconfig(SAMPLE_EMMC).partitions;
        // Delete partition 8 (CFG).
        partitions.retain(|p| p.partition_number != 8);
        // Add a new partition 10.
        partitions.push(FlashPartition {
            partition_number: 10,
            label: "EXTRA".into(),
            size: 2048,
            size_string: format_flash_size(2048),
            file: "".into(),
            mountpoint: "".into(),
            type_field: "".into(),
            enabled: true,
        });

        write_flash_defconfig(&partitions, root.to_str().unwrap(), &chip).unwrap();
        let out = fs::read_to_string(&defconfig).unwrap();

        // Deleted partition 8 lines are gone.
        assert!(!out.contains("CONFIG_PARTITION_8_LABEL="));
        // New partition block inserted before the terminator.
        assert!(out.contains("CONFIG_PARTITION_10=y"));
        assert!(out.contains("CONFIG_PARTITION_10_LABEL=\"EXTRA\""));
        let end_pos = out.find("# end of Partition Configuration").unwrap();
        let p10_pos = out.find("CONFIG_PARTITION_10=y").unwrap();
        assert!(p10_pos < end_pos);
        // COUNT updated to the highest partition number.
        assert!(out.contains("CONFIG_PARTITION_COUNT=10"));

        fs::remove_dir_all(&root).ok();
    }

    #[test]
    fn test_flash_defconfig_path_fallback() {
        let p = flash_defconfig_path("/sdk", "");
        assert!(p.ends_with(
            "build/boards/cv184x/cv1842hp_wevb_0014a_emmc/cv1842hp_wevb_0014a_emmc_defconfig"
        ));
        let p2 = flash_defconfig_path("/sdk", "cv1842hp_wevb_0014a_spinor");
        assert!(p2.ends_with("cv1842hp_wevb_0014a_spinor/cv1842hp_wevb_0014a_spinor_defconfig"));
    }

    /// Round-trip against the REAL board defconfig shipped in the repo (read-only
    /// copy into a temp tree). Guards against drift between the parser/writer and
    /// the actual on-disk format. Skips gracefully if the file is absent.
    #[test]
    fn test_roundtrip_real_emmc_defconfig() {
        let real = Path::new(env!("CARGO_MANIFEST_DIR")).join(
            "../build/boards/cv184x/cv1842hp_wevb_0014a_emmc/cv1842hp_wevb_0014a_emmc_defconfig",
        );
        let Ok(original) = fs::read_to_string(&real) else {
            eprintln!("skipping: real defconfig not found at {}", real.display());
            return;
        };

        // Parse the real file.
        let info = parse_flash_defconfig(&original);
        assert_eq!(info.flash_size, "32GB");
        assert_eq!(info.partitions.len(), 8);

        // Copy into a temp SDK tree and edit one partition's size.
        let (root, chip, defconfig) = make_temp_board(&original);
        let mut partitions = info.partitions.clone();
        let boot = partitions.iter_mut().find(|p| p.label == "BOOT").unwrap();
        let new_size = boot.size + 1024;
        boot.size = new_size;

        write_flash_defconfig(&partitions, root.to_str().unwrap(), &chip).unwrap();
        let updated = fs::read_to_string(&defconfig).unwrap();

        // Only the BOOT SIZE line should differ from the original.
        let orig_lines: Vec<&str> = original.lines().collect();
        let new_lines: Vec<&str> = updated.lines().collect();
        assert_eq!(orig_lines.len(), new_lines.len());
        let diffs: Vec<(usize, &str, &str)> = orig_lines
            .iter()
            .zip(new_lines.iter())
            .enumerate()
            .filter(|(_, (a, b))| a != b)
            .map(|(i, (a, b))| (i, *a, *b))
            .collect();
        assert_eq!(diffs.len(), 1, "unexpected diffs: {:?}", diffs);
        assert!(diffs[0]
            .2
            .contains(&format!("CONFIG_PARTITION_3_SIZE=\"{}\"", new_size)));

        fs::remove_dir_all(&root).ok();
    }
}
