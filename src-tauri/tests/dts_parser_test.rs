use cvicubemx_lib::dts_parser::DtsParser;
use cvicubemx_lib::dts_writer::DtsWriter;

#[test]
fn test_dts_parsing_and_dma_cascade() {
    let content = r#"
&uart0 {
	status = "okay";
	current-speed = <115200>;
};
&i2c0 {
	status = "okay";
	clock-frequency = <100000>;
};
sysdma_remap {
	ch-remap = <0 5 12 13 42 42 4 7>;
	status = "okay";
};
"#;

    let mut parser = DtsParser::new();
    parser.load_content(content);

    // 验证初始解析
    {
        let peripherals = parser.get_peripherals();
        assert!(peripherals.contains_key("uart0"));
        assert!(peripherals.contains_key("i2c0"));
        assert!(peripherals.contains_key("sysdma_remap"));

        let uart0 = peripherals.get("uart0").unwrap();
        assert_eq!(uart0.status, "okay");
        assert_eq!(uart0.current_speed, 115200);

        let i2c0 = peripherals.get("i2c0").unwrap();
        assert_eq!(i2c0.clock_frequency, 100000);

        let sysdma = peripherals.get("sysdma_remap").unwrap();
        assert_eq!(sysdma.sysdma_channels, vec!["0", "5", "12", "13", "42", "42", "4", "7"]);
    }

    // 联动修改：修改 sysdma 通道，将 12 更改为 8 (对应 uart0_rx)
    let new_channels = vec![
        "0".to_string(), "5".to_string(), "8".to_string(), "13".to_string(),
        "42".to_string(), "42".to_string(), "4".to_string(), "7".to_string()
    ];

    DtsWriter::update_sysdma_channels(&mut parser, "sysdma_remap", new_channels).unwrap();

    // 检查联动后的文件内容
    let updated_content = parser.get_file_content();
    
    // 应该给 uart0 加上 dmas, dma-names, capability 属性
    assert!(updated_content.contains("dmas = <&dmac 2 1 1>;"));
    assert!(updated_content.contains("dma-names = \"rx\";"));
    assert!(updated_content.contains("capability = \"rx\";"));
}
