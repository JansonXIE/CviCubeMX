use cvicubemx_lib::memory::{
    collect_memory_overlaps, get_default_memory_regions, validate_memory_constraints, MemoryRegion,
};

#[test]
fn test_memory_layout_validation_integration() {
    // 1. 重叠区域校验测试：重叠不再阻断校验（与 C++ 原逻辑一致），
    //    validate_memory 返回 Ok 并把重叠作为警告列表带回。
    let overlapping_regions = vec![
        MemoryRegion {
            name: "RegionA".to_string(),
            start_address: 0x80000000,
            end_address: 0x81000000,
            size: 0x1000000,
            size_string: "16M".to_string(),
            is_editable: true,
            description: "".to_string(),
        },
        MemoryRegion {
            name: "RegionB".to_string(),
            start_address: 0x80800000, // 与 A 重合
            end_address: 0x81800000,
            size: 0x1000000,
            size_string: "16M".to_string(),
            is_editable: true,
            description: "".to_string(),
        },
    ];

    // 地址范围校验不因重叠而失败（重叠不再是硬错误）
    assert!(cvicubemx_lib::memory::validate_memory_layout(&overlapping_regions).is_ok());
    // 重叠被收集为信息性警告
    let warnings = collect_memory_overlaps(&overlapping_regions);
    assert_eq!(warnings.len(), 1);
    assert!(warnings[0].contains("重叠"));

    // 2. 合法无重叠校验测试
    let layout_res = cvicubemx_lib::memory::validate_memory_layout(&[
        MemoryRegion {
            name: "RegionA".to_string(),
            start_address: 0x80000000,
            end_address: 0x80100000,
            size: 0x100000,
            size_string: "1M".to_string(),
            is_editable: true,
            description: "".to_string(),
        },
        MemoryRegion {
            name: "RegionB".to_string(),
            start_address: 0x80100000,
            end_address: 0x80200000,
            size: 0x100000,
            size_string: "1M".to_string(),
            is_editable: true,
            description: "".to_string(),
        },
    ]);
    assert!(layout_res.is_ok());
}

#[test]
fn test_memory_constraints_integration() {
    let mut regions = get_default_memory_regions();

    // 强制违反约束 1: RTOS_ION_ADDR >= FSBL_C906L_START_ADDR + RTOS_SYS_SIZE
    // FSBL_C906L_START 默认是 0x800a0000, RTOS_SYS 大小 4M = 0x400000
    // 它们的和是 0x804a0000
    // 我们把 RTOS_ION 地址改成 0x80400000, 就会违反此约束
    if let Some(rtos_ion) = regions.iter_mut().find(|r| r.name == "RTOS_ION") {
        rtos_ion.start_address = 0x80400000;
    }

    let result = validate_memory_constraints(&regions);
    assert!(result.is_err());
    assert!(result.unwrap_err().contains("RTOS_ION地址"));
}
