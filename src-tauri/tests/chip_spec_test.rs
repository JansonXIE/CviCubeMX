use cvicubemx_lib::chip_spec::{get_all_chip_specs, get_chip_spec, load_chip_spec};

#[test]
fn test_get_all_chip_specs() {
    let specs = get_all_chip_specs();
    assert_eq!(specs.len(), 6);
    
    // 校验 package 类型
    assert!(specs.iter().any(|s| s.chip_type == "cv1842hp" && s.package == "BGA"));
    assert!(specs.iter().any(|s| s.chip_type == "cv1811c" && s.package == "QFN"));
}

#[test]
fn test_get_chip_spec_valid() {
    let spec = get_chip_spec("cv1842hp").unwrap();
    assert_eq!(spec.chip_type, "cv1842hp");
    assert_eq!(spec.package, "BGA");
    assert_eq!(spec.pin_count, 221);
    assert_eq!(spec.rows, Some("ABCDEFGHJKLMNPR".to_string()));
    assert_eq!(spec.cols, Some(15));
}

#[test]
fn test_load_chip_spec_tauri_command() {
    let spec = load_chip_spec("cv1801c".to_string()).unwrap();
    assert_eq!(spec.chip_type, "cv1801c");
    assert_eq!(spec.package, "QFN");
    assert_eq!(spec.pin_count, 64);
}

#[test]
fn test_get_chip_spec_invalid() {
    let spec = get_chip_spec("invalid_chip");
    assert!(spec.is_err());
}
