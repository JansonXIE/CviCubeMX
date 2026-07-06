use cvicubemx_lib::codegen::{generate_code, is_gpio_mode, update_existing_code, PinConfig};
use std::fs;

#[test]
fn test_codegen_is_gpio_mode() {
    assert!(is_gpio_mode("XGPIOC_18"));
    assert!(is_gpio_mode("PWR_GPIO_0"));
    assert!(!is_gpio_mode("UART0_TX"));
}

#[test]
fn test_generate_and_update_code_integration() {
    //1.生成代码
    let pin_configs = vec![
        PinConfig {
            pin_name: "PAD_MIPI_TXM4".to_string(),
            function: "XGPIOC_18".to_string(),
            user_configured: true,
            state: None,
        },
        PinConfig {
            pin_name: "PAD_ETH_TXP".to_string(),
            // 配置成非 ETH复用功能时，需要先解锁 ETH pad，再切 pin function
            function: "SPI1_SDO".to_string(),
            user_configured: true,
            state: None,
        },
    ];

    let generated = generate_code("cv1842hp", &pin_configs, None).unwrap();

    // 应该包含 ETH pad 解锁寄存器序列 (PAD_ETH_TXP 被配置即触发，
    // 且必须先把 pad 从内部 EPHY释放出来，再切到非 ETH pin function)
    assert!(generated.contains("Unlock ETH pads"));
    assert!(generated.contains("mmio_write_32(0x03009804, 0x1);"));
    assert!(generated.contains("mmio_write_32(0x03009800, 0x0905);"));
    assert!(generated.contains("udelay(10); /* wait 10us */"));
    assert!(generated.contains("mmio_write_32(0x03009070, 0x606);"));
    let eth_unlock_pos = generated.find("Unlock ETH pads").unwrap();
    let eth_pinmux_pos = generated
        .find("PINMUX_CONFIG(PAD_ETH_TXP, SPI1_SDO);")
        .unwrap();
    assert!(eth_unlock_pos < eth_pinmux_pos);

    let pin_configs_with_uart = vec![PinConfig {
        pin_name: "UART0_TX".to_string(),
        // 使用真正的非 GPIO 功能
        function: "UART0_TX".to_string(),
        user_configured: true,
        state: None,
    }];
    let generated2 = generate_code("cv1842hp", &pin_configs_with_uart, None).unwrap();
    assert!(generated2.contains("PINMUX_CONFIG(UART0_TX, UART0_TX);"));

    //2. 增量更新已有文件测试
    let timestamp = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let temp_file = std::env::temp_dir().join(format!("cvi_board_init_{}.c", timestamp));

    let initial_c_content = r#"
void board_init() {
 // some original code
 return 0;
}
"#;
    fs::write(&temp_file, initial_c_content).unwrap();

    let update_res = update_existing_code(temp_file.to_str().unwrap(), &pin_configs_with_uart);
    assert!(update_res.is_ok());

    let updated_content = fs::read_to_string(&temp_file).unwrap();
    assert!(updated_content.contains("// Generated PINMUX configurations"));
    assert!(updated_content.contains("PINMUX_CONFIG(UART0_TX, UART0_TX);"));

    fs::remove_file(&temp_file).ok();
}
