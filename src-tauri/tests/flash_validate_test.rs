use cvicubemx_lib::flash::{
    validate_partitions, validate_partition_layout, default_partitions,
    FlashPartition, DEFAULT_FLASH_SIZE_KB
};

#[test]
fn test_flash_partition_validation_integration() {
    // 1. 验证默认分区表
    let partitions = default_partitions();
    let result = validate_partitions(partitions);
    assert!(result.is_ok());

    // 2. 重复分区编号校验（期望失败）
    let bad_partitions = vec![
        FlashPartition {
            partition_number: 2, label: "BOOT".into(), size: 1024,
            size_string: "1M".into(), file: "".into(), mountpoint: "".into(),
            type_field: "".into(), enabled: true,
        },
        FlashPartition {
            partition_number: 2, label: "ROOTFS".into(), size: 2048,
            size_string: "2M".into(), file: "".into(), mountpoint: "".into(),
            type_field: "".into(), enabled: true,
        },
    ];
    let result = validate_partitions(bad_partitions);
    assert!(result.is_err());
    assert!(result.unwrap_err().contains("Duplicate partition number"));

    // 3. 空标签校验（期望失败）
    let empty_label = vec![
        FlashPartition {
            partition_number: 2, label: "".into(), size: 1024,
            size_string: "1M".into(), file: "".into(), mountpoint: "".into(),
            type_field: "".into(), enabled: true,
        },
    ];
    let result = validate_partitions(empty_label);
    assert!(result.is_err());
    assert!(result.unwrap_err().contains("empty label"));

    // 4. 总大小溢出校验
    let overflow = vec![
        FlashPartition {
            partition_number: 2, label: "BOOT".into(), size: DEFAULT_FLASH_SIZE_KB + 1024,
            size_string: "TooBig".into(), file: "".into(), mountpoint: "".into(),
            type_field: "".into(), enabled: true,
        },
    ];
    let result = validate_partition_layout(&overflow, DEFAULT_FLASH_SIZE_KB);
    assert!(result.is_err());
    assert!(result.unwrap_err().contains("exceeds flash capacity"));
}
