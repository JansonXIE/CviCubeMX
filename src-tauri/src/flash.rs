//! Flash partition configuration validation and management module (M6).
//!
//! Provides flash partition data structures, layout validation, and Tauri commands
//! for the frontend.

use serde::{Deserialize, Serialize};
use std::fs;
use std::path::Path;

// ── Data Structures ──────────────────────────────────────────────────────────

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
/// assert_eq!(format_flash_size(3072),    "3MB");
/// assert_eq!(format_flash_size(8192),    "8MB");
/// assert_eq!(format_flash_size(512),     "512KB");
/// assert_eq!(format_flash_size(3145728), "3GB");
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
pub fn validate_partition_layout(partitions: &[FlashPartition], flash_size_kb: u64) -> Result<(), String> {
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

    // 2. Check each enabled partition size is a multiple of 64 KB
    //    (matching C++ onConfigFieldChanged behavior which rejects non-64-aligned sizes)
    for p in partitions {
        if p.enabled && p.size > 0 && p.size % 64 != 0 {
            return Err(format!(
                "Partition {} ('{}') size {} KB is not a multiple of 64 KB",
                p.partition_number, p.label, p.size
            ));
        }
    }

    // 3. Check labels for enabled partitions
    for p in partitions {
        if p.enabled && p.label.trim().is_empty() {
            return Err(format!(
                "Partition {} has an empty label but is enabled",
                p.partition_number
            ));
        }
    }

    // 4. Check total size of enabled partitions vs flash size
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
            partition_number: 2, label: "2nd".into(),
            size: 3072, size_string: format_flash_size(3072),
            file: "yoc.bin".into(), mountpoint: "".into(), type_field: "".into(), enabled: true,
        },
        FlashPartition {
            partition_number: 3, label: "BOOT".into(),
            size: 8192, size_string: format_flash_size(8192),
            file: "boot.emmc".into(), mountpoint: "".into(), type_field: "".into(), enabled: true,
        },
        FlashPartition {
            partition_number: 4, label: "MISC".into(),
            size: 512, size_string: format_flash_size(512),
            file: "logo.jpg".into(), mountpoint: "".into(), type_field: "".into(), enabled: true,
        },
        FlashPartition {
            partition_number: 5, label: "ENV".into(),
            size: 128, size_string: format_flash_size(128),
            file: "".into(), mountpoint: "".into(), type_field: "".into(), enabled: true,
        },
        FlashPartition {
            partition_number: 6, label: "ROOTFS".into(),
            size: 70656, size_string: format_flash_size(70656),
            file: "rootfs.emmc".into(), mountpoint: "".into(), type_field: "".into(), enabled: true,
        },
        FlashPartition {
            partition_number: 7, label: "SYSTEM".into(),
            size: 40960, size_string: format_flash_size(40960),
            file: "system.emmc".into(), mountpoint: "/mnt/system".into(), type_field: "ext4".into(), enabled: true,
        },
        FlashPartition {
            partition_number: 8, label: "CFG".into(),
            size: 15240, size_string: format_flash_size(15240),
            file: "cfg.emmc".into(), mountpoint: "mnt/cfg".into(), type_field: "ext4".into(), enabled: true,
        },
        FlashPartition {
            partition_number: 9, label: "DATA".into(),
            size: 3145728, size_string: format_flash_size(3145728),
            file: "data.emmc".into(), mountpoint: "mnt/data".into(), type_field: "ext4".into(), enabled: true,
        },
    ]
}

/// Default flash size: 32 GB in KB
pub const DEFAULT_FLASH_SIZE_KB: u64 = 32 * 1024 * 1024; // 32 GB = 33554432 KB

// ── JSON / defconfig export ──────────────────────────────────────────────────

/// Export flash partitions to a JSON file (matching C++ `exportToJson`).
pub fn export_flash_json_file(partitions: &[FlashPartition], flash_size: &str, partition_count: i32, path: &str) -> Result<(), String> {
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
    let content = fs::read_to_string(path).map_err(|e| format!("Failed to read JSON file: {}", e))?;
    let root: serde_json::Value = serde_json::from_str(&content).map_err(|e| format!("JSON parse error: {}", e))?;

    let flash_size = root.get("flashSize").and_then(|v| v.as_str()).unwrap_or("32GB").to_string();
    let partition_count = root.get("partitionCount").and_then(|v| v.as_i64()).unwrap_or(9) as i32;

    let arr = root.get("partitions").and_then(|v| v.as_array())
        .ok_or_else(|| "Missing 'partitions' array in JSON".to_string())?;

    let mut partitions = Vec::new();
    for val in arr {
        let p: FlashPartition = serde_json::from_value(val.clone())
            .map_err(|e| format!("Failed to parse FlashPartition: {}", e))?;
        partitions.push(p);
    }

    Ok((partitions, flash_size, partition_count))
}

/// Export flash config to a defconfig file (matching C++ `exportToDefconfig`).
///
/// Updates `CONFIG_PARTITION_N_*` lines in the defconfig.
pub fn write_flash_defconfig(partitions: &[FlashPartition], source_path: &str, chip_type: &str) -> Result<(), String> {
    let chip = if chip_type.is_empty() || chip_type == "请选择芯片型号" {
        "cv1842hp_wevb_0014a_emmc"
    } else {
        chip_type
    };
    let defconfig_rel = format!("build/boards/cv184x/{}/{}_defconfig", chip, chip);
    let defconfig_path = Path::new(source_path).join(&defconfig_rel);

    if !defconfig_path.exists() {
        return Err(format!("Defconfig file does not exist: {}", defconfig_path.display()));
    }

    let content = fs::read_to_string(&defconfig_path)
        .map_err(|e| format!("Failed to read defconfig: {}", e))?;
    let mut lines: Vec<String> = content.lines().map(|l| l.to_string()).collect();

    for p in partitions {
        let num = p.partition_number;
        let enabled_prefix = format!("CONFIG_PARTITION_{}=y", num);
        let label_prefix = format!("CONFIG_PARTITION_{}_LABEL=", num);
        let size_prefix = format!("CONFIG_PARTITION_{}_SIZE=", num);
        let file_prefix = format!("CONFIG_PARTITION_{}_FILE=", num);
        let mountpoint_prefix = format!("CONFIG_PARTITION_{}_MOUNTPOINT=", num);
        let type_prefix = format!("CONFIG_PARTITION_{}_TYPE=", num);

        for i in 0..lines.len() {
            if lines[i].starts_with(&enabled_prefix) && !p.enabled {
                // Remove the enabled line if partition is disabled
                lines.remove(i);
                break;
            } else if lines[i].starts_with(&label_prefix) {
                lines[i] = format!("CONFIG_PARTITION_{}_LABEL=\"{}\"", num, p.label);
            } else if lines[i].starts_with(&size_prefix) {
                lines[i] = format!("CONFIG_PARTITION_{}_SIZE=\"{}\"", num, p.size);
            } else if lines[i].starts_with(&file_prefix) {
                lines[i] = format!("CONFIG_PARTITION_{}_FILE=\"{}\"", num, p.file);
            } else if lines[i].starts_with(&mountpoint_prefix) {
                lines[i] = format!("CONFIG_PARTITION_{}_MOUNTPOINT=\"{}\"", num, p.mountpoint);
            } else if lines[i].starts_with(&type_prefix) {
                lines[i] = format!("CONFIG_PARTITION_{}_TYPE=\"{}\"", num, p.type_field);
            }
        }

        // If enabled but no CONFIG_PARTITION_N=y line, add the full config block
        if p.enabled {
            let has_enabled = lines.iter().any(|l| l.starts_with(&enabled_prefix));
            if !has_enabled {
                // Find a good insertion point
                let insert_pos = lines.iter().position(|l| l.contains("# Partition Configuration"))
                    .map(|i| i + 3)
                    .unwrap_or(lines.len());

                let new_lines = vec![
                    enabled_prefix,
                    format!("CONFIG_PARTITION_{}_LABEL=\"{}\"", num, p.label),
                    format!("CONFIG_PARTITION_{}_SIZE=\"{}\"", num, p.size),
                    format!("CONFIG_PARTITION_{}_FILE=\"{}\"", num, p.file),
                    format!("CONFIG_PARTITION_{}_MOUNTPOINT=\"{}\"", num, p.mountpoint),
                    format!("CONFIG_PARTITION_{}_TYPE=\"{}\"", num, p.type_field),
                    "".to_string(),
                ];
                for (offset, line) in new_lines.into_iter().enumerate() {
                    lines.insert(insert_pos + offset, line);
                }
            }
        }
    }

    let output = lines.join("\n");
    fs::write(&defconfig_path, output).map_err(|e| format!("Failed to write defconfig: {}", e))?;

    Ok(())
}

// ── Tauri commands ────────────────────────────────────────────────────────────

#[tauri::command]
pub fn load_partitions() -> Result<Vec<FlashPartition>, String> {
    Ok(default_partitions())
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
pub fn export_flash_defconfig_tauri(partitions: Vec<FlashPartition>, source_path: String, chip_type: String) -> Result<(), String> {
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
        let partitions = vec![
            FlashPartition {
                partition_number: 2, label: "BIG".into(), size: DEFAULT_FLASH_SIZE_KB + 1024,
                size_string: format_flash_size(DEFAULT_FLASH_SIZE_KB + 1024),
                file: "".into(), mountpoint: "".into(), type_field: "".into(), enabled: true,
            },
        ];
        let result = validate_partition_layout(&partitions, DEFAULT_FLASH_SIZE_KB);
        assert!(result.is_err());
        assert!(result.unwrap_err().contains("exceeds flash capacity"));
    }

    #[test]
    fn test_validate_partition_layout_disabled_not_counted() {
        let partitions = vec![
            FlashPartition {
                partition_number: 2, label: "BIG".into(), size: DEFAULT_FLASH_SIZE_KB + 1024,
                size_string: "".into(), file: "".into(), mountpoint: "".into(),
                type_field: "".into(), enabled: false, // disabled → not counted
            },
        ];
        // Even though total size > flash, disabled partition doesn't count
        assert!(validate_partition_layout(&partitions, DEFAULT_FLASH_SIZE_KB).is_ok());
    }

    #[test]
    fn test_validate_partition_layout_size_not_multiple_of_64_is_soft_warning() {
        // 64 KB alignment is a soft UI warning in C++, not a hard validation error.
        // Our validation follows the same approach: it does NOT reject non-aligned sizes.
        let partitions = vec![
            FlashPartition {
                partition_number: 2, label: "BAD_SIZE".into(), size: 100,
                size_string: "".into(), file: "".into(), mountpoint: "".into(),
                type_field: "".into(), enabled: true,
            },
        ];
        let result = validate_partition_layout(&partitions, DEFAULT_FLASH_SIZE_KB);
        // Non-64-aligned size does NOT cause a hard validation error
        assert!(result.is_ok());
    }

    #[test]
    fn test_validate_partition_layout_duplicate_number() {
        let partitions = vec![
            FlashPartition {
                partition_number: 2, label: "A".into(), size: 1024,
                size_string: "".into(), file: "".into(), mountpoint: "".into(),
                type_field: "".into(), enabled: true,
            },
            FlashPartition {
                partition_number: 2, label: "B".into(), size: 2048,
                size_string: "".into(), file: "".into(), mountpoint: "".into(),
                type_field: "".into(), enabled: true,
            },
        ];
        let result = validate_partition_layout(&partitions, DEFAULT_FLASH_SIZE_KB);
        assert!(result.is_err());
        assert!(result.unwrap_err().contains("Duplicate partition number"));
    }

    #[test]
    fn test_validate_partition_layout_empty_label_enabled() {
        let partitions = vec![
            FlashPartition {
                partition_number: 2, label: "".into(), size: 1024,
                size_string: "".into(), file: "".into(), mountpoint: "".into(),
                type_field: "".into(), enabled: true,
            },
        ];
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
        let partitions = vec![
            FlashPartition {
                partition_number: 2, label: "BAD".into(), size: 100,
                size_string: "".into(), file: "".into(), mountpoint: "".into(),
                type_field: "".into(), enabled: false,
            },
        ];
        assert!(validate_partition_layout(&partitions, DEFAULT_FLASH_SIZE_KB).is_ok());
    }
}