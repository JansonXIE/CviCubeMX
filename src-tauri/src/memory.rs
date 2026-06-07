use serde::{Deserialize, Serialize};
use std::fs;
use std::path::Path;

// 常量定义
pub const TOTAL_MEMORY_SIZE: u64 = 0x10000000; // 256MB
pub const MEMORY_BASE_ADDRESS: u64 = 0x80000000;

/// 内存区域配置结构
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MemoryRegion {
    pub name: String,
    pub start_address: u64,
    pub end_address: u64,
    pub size: u64,
    pub size_string: String,
    pub is_editable: bool,
    pub description: String,
}

/// 格式化大小（字节 → M/K/B 格式字符串）
/// 例如: 268435456 → "256M", 1024 → "1K", 100 → "100B", 0 → "0M"
pub fn format_size(size_in_bytes: u64) -> String {
    if size_in_bytes == 0 {
        return "0M".to_string();
    }

    const KB: u64 = 1024;
    const MB: u64 = KB * 1024;
    const GB: u64 = MB * 1024;

    if size_in_bytes >= GB {
        format!("{}G", size_in_bytes / GB)
    } else if size_in_bytes >= MB {
        format!("{}M", size_in_bytes / MB)
    } else if size_in_bytes >= KB {
        format!("{}K", size_in_bytes / KB)
    } else {
        format!("{}B", size_in_bytes)
    }
}

/// 格式化地址为十六进制字符串
/// 例如: 0x80000000 → "0x80000000"
pub fn format_address(addr: u64) -> String {
    format!("0x{:08X}", addr)
}

/// 从字符串解析地址
/// 支持格式: "0x80000000", "0X80000000", "80000000"
pub fn parse_address(address_str: &str) -> u64 {
    let clean_str = address_str.trim();

    if clean_str.starts_with("0x") || clean_str.starts_with("0X") {
        let hex_part = &clean_str[2..];
        u64::from_str_radix(hex_part, 16).unwrap_or(0)
    } else {
        u64::from_str_radix(clean_str, 16).unwrap_or(0)
    }
}

/// 检查两个内存区域是否重叠
/// 重叠条件: region1.end_address > region2.start_address && region2.end_address > region1.start_address
/// 注意：size 为 0 的区域不参与重叠检测（与 C++ 源码一致）
pub fn check_memory_overlap(region1: &MemoryRegion, region2: &MemoryRegion) -> bool {
    // size 为 0 的区域不参与重叠检测
    if region1.size == 0 || region2.size == 0 {
        return false;
    }
    region1.end_address > region2.start_address && region2.end_address > region1.start_address
}

/// 验证内存布局
/// 检查所有非零大小的区域是否存在重叠，以及地址是否在合理范围内
/// 注意：与 C++ 源码一致，重叠检测是信息性的——它记录重叠但不阻止配置
/// 此函数返回重叠区域列表作为警告，不作为硬错误
pub fn validate_memory_layout(regions: &[MemoryRegion]) -> Result<Vec<String>, String> {
    // 过滤出有实际大小的区域
    let mut sorted_regions: Vec<&MemoryRegion> = regions
        .iter()
        .filter(|r| r.size > 0)
        .collect();

    // 按起始地址排序
    sorted_regions.sort_by_key(|r| r.start_address);

    // 检查相邻区域是否重叠（信息性，不阻止配置）
    let mut overlap_warnings: Vec<String> = Vec::new();
    for i in 0..sorted_regions.len() - 1 {
        let current = sorted_regions[i];
        let next = sorted_regions[i + 1];

        if check_memory_overlap(current, next) {
            overlap_warnings.push(format!(
                "{} ({}) 与 {} ({}) 存在重叠",
                current.name,
                format_address(current.start_address),
                next.name,
                format_address(next.start_address)
            ));
        }
    }

    // 检查地址范围约束（所有区域应在基地址范围内）——这是硬错误
    for region in regions.iter().filter(|r| r.size > 0) {
        if region.start_address < MEMORY_BASE_ADDRESS {
            return Err(format!(
                "区域 {} 的起始地址 {} 小于内存基地址 {}",
                region.name,
                format_address(region.start_address),
                format_address(MEMORY_BASE_ADDRESS)
            ));
        }
    }

    Ok(overlap_warnings)
}

/// 验证内存约束条件（对应 C++ 中的 validateMemoryConstraints）
/// 检查特定区域之间的约束关系
pub fn validate_memory_constraints(regions: &[MemoryRegion]) -> Result<(), String> {
    // 构建名称到区域的映射
    let region_map: std::collections::HashMap<&str, &MemoryRegion> = regions
        .iter()
        .map(|r| (r.name.as_str(), r))
        .collect();

    let get_addr = |name: &str| -> u64 {
        region_map.get(name).map(|r| r.start_address).unwrap_or(0)
    };

    let get_size = |name: &str| -> u64 {
        region_map.get(name).map(|r| r.size).unwrap_or(0)
    };

    // RTOS_SYS_SIZE: 默认 4M
    let rtos_sys_size: u64 = get_size("RTOS_SYS");
    let rtos_sys_size = if rtos_sys_size > 0 { rtos_sys_size } else { 4 * 1024 * 1024 };

    let fsbl_c906l_start_addr = get_addr("FSBL_C906L_START");
    let rtos_ion_addr = get_addr("RTOS_ION");
    let rtos_compress_bin_addr = get_addr("RTOS_COMPRESS_BIN");
    let uimag_addr = get_addr("UIMAG");
    let uimag_size = get_size("UIMAG");
    let ion_size = get_size("ION");
    let rtos_ion_size = get_size("RTOS_ION");

    // 约束1: RTOS_ION_ADDR >= FSBL_C906L_START_ADDR + RTOS_SYS_SIZE
    if rtos_sys_size > 0 && rtos_ion_addr < fsbl_c906l_start_addr + rtos_sys_size {
        return Err(format!(
            "约束违反: RTOS_ION地址({}) 必须 >= FSBL_C906L_START地址({}) + RTOS_SYS_SIZE({})",
            format_address(rtos_ion_addr),
            format_address(fsbl_c906l_start_addr),
            format_address(rtos_sys_size)
        ));
    }

    // 约束2: RTOS_COMPRESS_BIN_ADDR >= UIMAG_ADDR + UIMAG_SIZE
    if rtos_sys_size > 0 && rtos_compress_bin_addr < uimag_addr + uimag_size {
        return Err(format!(
            "约束违反: RTOS_COMPRESS_BIN地址({}) 必须 >= UIMAG地址({}) + UIMAG大小({})",
            format_address(rtos_compress_bin_addr),
            format_address(uimag_addr),
            format_address(uimag_size)
        ));
    }

    // 约束3: ION_ADDR >= FSBL_C906L_START_ADDR + RTOS_SYS_SIZE（默认启用 ALIOS）
    let fixed_256m_boundary = MEMORY_BASE_ADDRESS + 256 * 1024 * 1024; // 0x90000000
    let calculated_rtos_ion_addr = fixed_256m_boundary - rtos_ion_size;
    let calculated_ion_addr = calculated_rtos_ion_addr - ion_size;

    if calculated_ion_addr < fsbl_c906l_start_addr + rtos_sys_size {
        return Err(format!(
            "约束违反: ION地址计算不符合要求\n\
             计算过程:\n\
             256M边界 = {}\n\
             RTOS_ION地址 = 256M边界 - RTOS_ION_SIZE = {} - {} = {}\n\
             ION地址 = RTOS_ION地址 - ION_SIZE = {} - {} = {}\n\
             约束要求: ION地址({}) >= FSBL_C906L_START({}) + RTOS_SYS_SIZE({}) = {}\n\
             提示: 尝试减小 ION_SIZE 或 RTOS_ION_SIZE",
            format_address(fixed_256m_boundary),
            format_address(fixed_256m_boundary),
            format_address(rtos_ion_size),
            format_address(calculated_rtos_ion_addr),
            format_address(calculated_rtos_ion_addr),
            format_address(ion_size),
            format_address(calculated_ion_addr),
            format_address(calculated_ion_addr),
            format_address(fsbl_c906l_start_addr),
            format_address(rtos_sys_size),
            format_address(fsbl_c906l_start_addr + rtos_sys_size)
        ));
    }

    Ok(())
}

/// 获取默认内存区域列表（对应 C++ 中的 initializeMemoryRegions）
pub fn get_default_memory_regions() -> Vec<MemoryRegion> {
    let region_data: Vec<(String, u64, u64, bool, String)> = vec![
        ("MONITOR".to_string(),            0x80000000, 0x0,        true, "监控区域".to_string()),
        ("KERNEL_MEMORY".to_string(),      0x80000000, 0x10000000, true, "内核内存区域".to_string()),
        ("FSBL_C906L_START".to_string(),   0x800a0000, 0x0,        true, "FSBL C906L启动区域".to_string()),
        ("OPENSBI_FDT".to_string(),        0x800a0000, 0x0,        true, "OpenSBI设备树".to_string()),
        ("RTOS_LOG".to_string(),           0x804a0000, 0x20000,    true, "RTOS日志区域".to_string()),
        ("SHARE_MEM".to_string(),          0x804c0000, 0x20000,    true, "共享内存".to_string()),
        ("SHARE_PARAM".to_string(),        0x804e0000, 0x10000,    true, "共享参数".to_string()),
        ("PQBIN".to_string(),              0x80500000, 0x80000,    true, "PQBIN区域".to_string()),
        ("RTOS_LOGO".to_string(),          0x80580000, 0x0,        true, "RTOS Logo".to_string()),
        ("CVI_UPDATE_HEADER".to_string(),  0x813ffc00, 0x400,      true, "CVI更新头".to_string()),
        ("FSBL_UNZIP".to_string(),         0x81400000, 0x400000,   true, "FSBL解压区域".to_string()),
        ("UIMAG".to_string(),              0x81400000, 0x400000,   true, "UI镜像".to_string()),
        ("RTOS_COMPRESS_BIN".to_string(),  0x81ea0000, 0x0,        true, "RTOS压缩二进制".to_string()),
        ("H26X_BITSTREAM".to_string(),     0x85500000, 0x0,        true, "H26X比特流".to_string()),
        ("H26X_ENC_BUFF".to_string(),      0x85500000, 0x0,        true, "H26X编码缓冲".to_string()),
        ("ION".to_string(),                0x85500000, 0x4b00000,  true, "ION内存池".to_string()),
        ("ISP_MEM_BASE".to_string(),       0x85500000, 0x0,        true, "ISP内存基址".to_string()),
        ("BOOTLOGO".to_string(),           0x89e3e000, 0x1c2000,   true, "启动Logo".to_string()),
        ("RTOS_ION".to_string(),           0x8a000000, 0x6000000,  true, "RTOS ION".to_string()),
    ];

    region_data
        .into_iter()
        .map(|(name, start_address, size, is_editable, description)| {
            let end_address = start_address + size;
            let size_string = format_size(size);
            MemoryRegion {
                name,
                start_address,
                end_address,
                size,
                size_string,
                is_editable,
                description,
            }
        })
        .collect()
}

// ==================== Tauri Commands ====================

#[tauri::command]
pub fn load_memory_regions() -> Result<Vec<MemoryRegion>, String> {
    Ok(get_default_memory_regions())
}

#[tauri::command]
pub fn validate_memory(regions: Vec<MemoryRegion>) -> Result<Vec<String>, String> {
    // 先检查重叠（返回重叠警告列表）
    let overlap_warnings = validate_memory_layout(&regions)?;
    // 再检查约束条件
    validate_memory_constraints(&regions)?;
    Ok(overlap_warnings)
}

#[tauri::command]
pub fn export_memory_json(regions: Vec<MemoryRegion>, path: String) -> Result<(), String> {
    let json_data = serde_json::json!({
        "memoryRegions": regions,
        "totalMemorySize": format!("0x{:x}", TOTAL_MEMORY_SIZE),
        "memoryBaseAddress": format!("0x{:x}", MEMORY_BASE_ADDRESS),
    });

    let json_str = serde_json::to_string_pretty(&json_data)
        .map_err(|e| format!("JSON序列化失败: {}", e))?;

    fs::write(&path, json_str)
        .map_err(|e| format!("写入文件失败: {}", e))?;

    Ok(())
}

#[tauri::command]
pub fn export_memory_defconfig(
    regions: Vec<MemoryRegion>,
    source_path: String,
    chip_type: String,
) -> Result<(), String> {
    // 构建defconfig文件路径
    let defconfig_path = format!(
        "{}/build/boards/cv184x/{}/{}_defconfig",
        source_path, chip_type, chip_type
    );

    let path = Path::new(&defconfig_path);
    if !path.exists() {
        return Err(format!("Defconfig文件不存在: {}", defconfig_path));
    }

    // 读取现有文件内容
    let content = fs::read_to_string(path)
        .map_err(|e| format!("无法读取defconfig文件: {}", e))?;

    let mut lines: Vec<String> = content.lines().map(|s| s.to_string()).collect();

    // 获取相关区域的大小
    let region_map: std::collections::HashMap<&str, &MemoryRegion> = regions
        .iter()
        .map(|r| (r.name.as_str(), r))
        .collect();

    let ion_size = region_map.get("ION").map(|r| r.size).unwrap_or(0);
    let rtos_ion_size = region_map.get("RTOS_ION").map(|r| r.size).unwrap_or(0);

    let ion_size_hex = format!("{:x}", ion_size);
    let rtos_ion_size_hex = format!("{:x}", rtos_ion_size);

    // 更新配置行
    let mut found_ion_size = false;
    let mut found_rtos_ion_size = false;

    for line in lines.iter_mut() {
        if line.starts_with("CONFIG_ION_SIZE=") {
            *line = format!("CONFIG_ION_SIZE=0x{}", ion_size_hex);
            found_ion_size = true;
        } else if line.starts_with("CONFIG_RTOS_ION_SIZE=") {
            *line = format!("CONFIG_RTOS_ION_SIZE=0x{}", rtos_ion_size_hex);
            found_rtos_ion_size = true;
        }
    }

    // 如果没有找到配置项，添加到文件末尾
    if !found_ion_size {
        lines.push(format!("CONFIG_ION_SIZE=0x{}", ion_size_hex));
    }
    if !found_rtos_ion_size {
        lines.push(format!("CONFIG_RTOS_ION_SIZE=0x{}", rtos_ion_size_hex));
    }

    // 写回文件
    let output = lines.join("\n");
    fs::write(path, output)
        .map_err(|e| format!("无法写入defconfig文件: {}", e))?;

    Ok(())
}

// ==================== Unit Tests ====================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_format_size() {
        assert_eq!(format_size(268435456), "256M");
        assert_eq!(format_size(0), "0M");
        assert_eq!(format_size(1024), "1K");
        assert_eq!(format_size(100), "100B");
        assert_eq!(format_size(4 * 1024 * 1024), "4M");
        assert_eq!(format_size(75 * 1024 * 1024), "75M");
        assert_eq!(format_size(128 * 1024), "128K");
        assert_eq!(format_size(64 * 1024), "64K");
        assert_eq!(format_size(512 * 1024), "512K");
        assert_eq!(format_size(1 * 1024 * 1024 * 1024), "1G");
    }

    #[test]
    fn test_format_address() {
        assert_eq!(format_address(0x80000000), "0x80000000");
        assert_eq!(format_address(0x0), "0x00000000");
        assert_eq!(format_address(0x8a000000), "0x8A000000");
        assert_eq!(format_address(0x85500000), "0x85500000");
    }

    #[test]
    fn test_parse_address() {
        assert_eq!(parse_address("0x80000000"), 0x80000000);
        assert_eq!(parse_address("0X80000000"), 0x80000000);
        assert_eq!(parse_address("80000000"), 0x80000000);
        assert_eq!(parse_address("  0x80000000  "), 0x80000000); // trimming
        assert_eq!(parse_address("invalid"), 0); // invalid returns 0
    }

    #[test]
    fn test_check_memory_overlap_no_overlap() {
        let region1 = MemoryRegion {
            name: "A".to_string(),
            start_address: 0x80000000,
            end_address: 0x80010000,
            size: 0x10000,
            size_string: "64K".to_string(),
            is_editable: true,
            description: "".to_string(),
        };
        let region2 = MemoryRegion {
            name: "B".to_string(),
            start_address: 0x80010000,
            end_address: 0x80020000,
            size: 0x10000,
            size_string: "64K".to_string(),
            is_editable: true,
            description: "".to_string(),
        };
        // 不重叠: region1的end等于region2的start（边界相邻不算重叠）
        assert!(!check_memory_overlap(&region1, &region2));
    }

    #[test]
    fn test_check_memory_overlap_with_overlap() {
        let region1 = MemoryRegion {
            name: "A".to_string(),
            start_address: 0x80000000,
            end_address: 0x80020000,
            size: 0x20000,
            size_string: "128K".to_string(),
            is_editable: true,
            description: "".to_string(),
        };
        let region2 = MemoryRegion {
            name: "B".to_string(),
            start_address: 0x80010000,
            end_address: 0x80030000,
            size: 0x20000,
            size_string: "128K".to_string(),
            is_editable: true,
            description: "".to_string(),
        };
        // 重叠: region1.end > region2.start
        assert!(check_memory_overlap(&region1, &region2));
    }

    #[test]
    fn test_check_memory_overlap_zero_size() {
        let region1 = MemoryRegion {
            name: "A".to_string(),
            start_address: 0x80000000,
            end_address: 0x80000000,
            size: 0,
            size_string: "0M".to_string(),
            is_editable: true,
            description: "".to_string(),
        };
        let region2 = MemoryRegion {
            name: "B".to_string(),
            start_address: 0x80000000,
            end_address: 0x80010000,
            size: 0x10000,
            size_string: "64K".to_string(),
            is_editable: true,
            description: "".to_string(),
        };
        // size为0的区域不参与重叠检测
        assert!(!check_memory_overlap(&region1, &region2));
    }

    #[test]
    fn test_validate_memory_layout_overlap_warning() {
        // Overlapping regions produce warnings, not errors
        let regions = vec![
            MemoryRegion {
                name: "A".to_string(),
                start_address: 0x80000000,
                end_address: 0x80020000,
                size: 0x20000,
                size_string: "128K".to_string(),
                is_editable: true,
                description: "".to_string(),
            },
            MemoryRegion {
                name: "B".to_string(),
                start_address: 0x80010000,
                end_address: 0x80030000,
                size: 0x20000,
                size_string: "128K".to_string(),
                is_editable: true,
                description: "".to_string(),
            },
        ];
        let result = validate_memory_layout(&regions);
        assert!(result.is_ok()); // No hard error
        let warnings = result.unwrap();
        assert!(!warnings.is_empty()); // But has overlap warnings
        assert!(warnings[0].contains("重叠"));
    }

    #[test]
    fn test_validate_memory_layout_no_overlap() {
        let regions = vec![
            MemoryRegion {
                name: "A".to_string(),
                start_address: 0x80000000,
                end_address: 0x80010000,
                size: 0x10000,
                size_string: "64K".to_string(),
                is_editable: true,
                description: "".to_string(),
            },
            MemoryRegion {
                name: "B".to_string(),
                start_address: 0x80010000,
                end_address: 0x80020000,
                size: 0x10000,
                size_string: "64K".to_string(),
                is_editable: true,
                description: "".to_string(),
            },
        ];
        let result = validate_memory_layout(&regions);
        assert!(result.is_ok());
    }

    #[test]
    fn test_validate_memory_layout_address_below_base() {
        let regions = vec![
            MemoryRegion {
                name: "A".to_string(),
                start_address: 0x70000000, // 低于基地址
                end_address: 0x70010000,
                size: 0x10000,
                size_string: "64K".to_string(),
                is_editable: true,
                description: "".to_string(),
            },
        ];
        let result = validate_memory_layout(&regions);
        assert!(result.is_err());
        let err_msg = result.unwrap_err();
        assert!(err_msg.contains("小于内存基地址"));
    }

    #[test]
    fn test_get_default_memory_regions() {
        let regions = get_default_memory_regions();
        assert_eq!(regions.len(), 19);

        // 检查第一个区域
        assert_eq!(regions[0].name, "MONITOR");
        assert_eq!(regions[0].start_address, 0x80000000);
        assert_eq!(regions[0].size, 0);

        // 检查 KERNEL_MEMORY
        let kernel = regions.iter().find(|r| r.name == "KERNEL_MEMORY").unwrap();
        assert_eq!(kernel.start_address, 0x80000000);
        assert_eq!(kernel.size, 0x10000000);
        assert_eq!(kernel.size_string, "256M");

        // 检查 ION
        let ion = regions.iter().find(|r| r.name == "ION").unwrap();
        assert_eq!(ion.start_address, 0x85500000);
        assert_eq!(ion.size, 0x4b00000);
        assert_eq!(ion.size_string, "75M");

        // 检查 RTOS_ION
        let rtos_ion = regions.iter().find(|r| r.name == "RTOS_ION").unwrap();
        assert_eq!(rtos_ion.start_address, 0x8a000000);
        assert_eq!(rtos_ion.size, 0x6000000);
        assert_eq!(rtos_ion.size_string, "96M");
    }

    #[test]
    fn test_validate_default_memory_regions() {
        let regions = get_default_memory_regions();
        // 默认配置应该通过验证（不重叠）
        let result = validate_memory_layout(&regions);
        assert!(result.is_ok());
    }

    #[test]
    fn test_format_size_various() {
        // 边界值测试
        assert_eq!(format_size(1), "1B");
        assert_eq!(format_size(1023), "1023B");
        assert_eq!(format_size(1024), "1K");
        assert_eq!(format_size(1024 * 1024 - 1), "1023K");
        assert_eq!(format_size(1024 * 1024), "1M");
    }
}