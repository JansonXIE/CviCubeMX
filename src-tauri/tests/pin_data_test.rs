use cvicubemx_lib::pin_data::load_pin_data;
use cvicubemx_lib::pin_data_tool::{
    clean_pin_name, get_function_name_remap, is_bga_corner, parse_function_select_cell,
    pin_sort_key,
};

#[test]
fn test_clean_pin_name_integration() {
    assert_eq!(clean_pin_name("PAD_ETH_RXM___EPHY_TXP"), "PAD_ETH_RXM");
    assert_eq!(clean_pin_name("PAD_MIPI_TXM4"), "PAD_MIPI_TXM4");
}

#[test]
fn test_function_name_remap_integration() {
    let remap = get_function_name_remap();
    assert_eq!(remap.len(), 8);
    assert_eq!(remap.get("CR_4WTMS").copied(), Some("CV_2WTMS_CR_4WTMS"));
    assert_eq!(remap.get("CR_2WTCK").copied(), Some("CV_4WTCK_CR_2WTCK"));
}

#[test]
fn test_parse_function_select_cell_integration() {
    let content = "0 : UART0_TX (default)\n3 : XGPIOA_16";
    let (functions, default) = parse_function_select_cell(content);
    assert_eq!(default, "UART0_TX");
    assert!(functions.contains(&"UART0_TX".to_string()));
}

#[test]
fn test_pin_sort_key_integration() {
    let key_a2 = pin_sort_key("A2");
    let key_a10 = pin_sort_key("A10");
    assert!(key_a2 < key_a10);
}

#[test]
fn test_load_pin_data_bga_corners() {
    let pins = load_pin_data("cv1842hp".to_string()).unwrap();
    // 确保 BGA 四角引脚不在加载的列表中
    assert!(!pins.iter().any(|p| is_bga_corner(&p.pin_num)));
}
