use cvicubemx_lib::dts_parser::DtsParser;
use cvicubemx_lib::dts_writer::DtsWriter;
use cvicubemx_lib::peripheral::is_protected_property;

const SAMPLE: &str = r#"
&i2c0 {
	compatible = "snps,designware-i2c";
	reg = <0x0 0x04000000 0x0 0x1000>;
	status = "okay";
	clock-frequency = <100000>;
	wakeup-source;
};
&spi0 {
	compatible = "snps,dw-apb-ssi";
	status = "disabled";
};
&saradc {
	compatible = "cvitek,saradc";
	status = "okay";
};
"#;

// A1. 解析出全部属性，含三种 kind
#[test]
fn test_parse_all_raw_properties() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    let props = p.get_raw_properties("i2c0").unwrap();

    let keys: Vec<&str> = props.iter().map(|x| x.key.as_str()).collect();
    assert!(keys.contains(&"compatible"));
    assert!(keys.contains(&"reg"));
    assert!(keys.contains(&"status"));
    assert!(keys.contains(&"clock-frequency"));
    assert!(keys.contains(&"wakeup-source"));

    let comp = props.iter().find(|x| x.key == "compatible").unwrap();
    assert_eq!(comp.kind, "string");
    assert_eq!(comp.value, "snps,designware-i2c");

    let freq = props.iter().find(|x| x.key == "clock-frequency").unwrap();
    assert_eq!(freq.kind, "cell");
    assert_eq!(freq.value.trim(), "100000");

    let wake = props.iter().find(|x| x.key == "wakeup-source").unwrap();
    assert_eq!(wake.kind, "bool");
    assert_eq!(wake.value, "");
}

// A2. 保护属性标记正确
#[test]
fn test_protected_flag() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    let props = p.get_raw_properties("i2c0").unwrap();
    assert!(
        props
            .iter()
            .find(|x| x.key == "compatible")
            .unwrap()
            .protected
    );
    assert!(props.iter().find(|x| x.key == "reg").unwrap().protected);
    assert!(!props.iter().find(|x| x.key == "status").unwrap().protected);
    assert!(is_protected_property("clocks"));
    assert!(!is_protected_property("foo-bar"));
}

// A3. 保持声明顺序
#[test]
fn test_preserve_order() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    let props = p.get_raw_properties("i2c0").unwrap();
    let keys: Vec<&str> = props.iter().map(|x| x.key.as_str()).collect();
    assert_eq!(keys[0], "compatible");
    assert_eq!(keys[1], "reg");
    assert_eq!(keys[2], "status");
}

// A4. upsert：修改已存在的 cell 属性（不重复插入）
#[test]
fn test_set_existing_cell_property() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    DtsWriter::set_raw_property(&mut p, "i2c0", "clock-frequency", "400000", "cell").unwrap();
    let c = p.get_file_content();
    assert!(c.contains("clock-frequency = <400000>;"));
    assert_eq!(c.matches("clock-frequency").count(), 1); // 未重复
}

// A5. upsert：新增不存在的 string 属性
#[test]
fn test_add_new_string_property() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    DtsWriter::set_raw_property(&mut p, "spi0", "spi-max-frequency", "abc", "string").unwrap();
    let c = p.get_file_content();
    assert!(c.contains("spi-max-frequency = \"abc\";"));
}

// A6. upsert：新增 cell 属性到 SPI（真实场景 spi-max-frequency）
#[test]
fn test_add_cell_property_to_spi() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    DtsWriter::set_raw_property(&mut p, "spi0", "spi-max-frequency", "50000000", "cell").unwrap();
    assert!(p
        .get_file_content()
        .contains("spi-max-frequency = <50000000>;"));
}

// A7. 新增 bool 属性
#[test]
fn test_add_bool_property() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    DtsWriter::set_raw_property(&mut p, "saradc", "cvitek,use-vref", "", "bool").unwrap();
    assert!(p.get_file_content().contains("cvitek,use-vref;"));
}

// A8. 删除非保护属性
#[test]
fn test_delete_property() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    DtsWriter::delete_raw_property(&mut p, "i2c0", "wakeup-source").unwrap();
    let c = p.get_file_content();
    assert!(!c.contains("wakeup-source"));
    assert!(c.contains("clock-frequency = <100000>;"));
}

// A9. 删除后不留空行/不破坏相邻属性
#[test]
fn test_delete_no_dangling_blank_line() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    DtsWriter::delete_raw_property(&mut p, "i2c0", "clock-frequency").unwrap();
    let c = p.get_file_content();
    assert!(!c.contains("clock-frequency"));
    assert!(c.contains("status = \"okay\";"));
    assert!(c.contains("wakeup-source;"));
    assert!(!c.contains("\n\n\n"));
}

// A10. 保护属性拒绝修改
#[test]
fn test_reject_set_protected() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    let r = DtsWriter::set_raw_property(&mut p, "i2c0", "compatible", "x", "string");
    assert!(r.is_err());
    assert!(p.get_file_content().contains("snps,designware-i2c"));
}

// A11. 保护属性拒绝删除
#[test]
fn test_reject_delete_protected() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    assert!(DtsWriter::delete_raw_property(&mut p, "i2c0", "reg").is_err());
    assert!(p.get_file_content().contains("reg = <0x0 0x04000000"));
}

// A12. 不误入子节点
#[test]
fn test_ignore_child_node_properties() {
    let content = r#"
&spi0 {
	status = "okay";
	flash@0 {
		compatible = "jedec,spi-nor";
		reg = <0>;
	};
};
"#;
    let mut p = DtsParser::new();
    p.load_content(content);
    let props = p.get_raw_properties("spi0").unwrap();
    let keys: Vec<&str> = props.iter().map(|x| x.key.as_str()).collect();
    assert!(keys.contains(&"status"));
    assert_eq!(keys.iter().filter(|k| **k == "compatible").count(), 0);
}

// A13. 多行 cell 值解析
#[test]
fn test_parse_multiline_cell() {
    let content = r#"
sysdma_remap {
	ch-remap = <0 5 2 3
				42 42 4 7>;
	status = "okay";
};
"#;
    let mut p = DtsParser::new();
    p.load_content(content);
    let props = p.get_raw_properties("sysdma_remap").unwrap();
    let ch = props.iter().find(|x| x.key == "ch-remap").unwrap();
    assert_eq!(ch.kind, "cell");
    assert!(ch.value.contains("0 5 2 3"));
    assert!(ch.value.contains("42 42 4 7"));
}

// A14. 找不到节点返回错误
#[test]
fn test_missing_node_error() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    assert!(p.get_raw_properties("not_exist").is_err());
}

// A15. 回归：raw 编辑不破坏现有友好属性解析
#[test]
fn test_regression_friendly_props_intact() {
    let mut p = DtsParser::new();
    p.load_content(SAMPLE);
    DtsWriter::set_raw_property(&mut p, "i2c0", "foo", "1", "cell").unwrap();
    let i2c0 = p.get_peripheral("i2c0").unwrap();
    assert_eq!(i2c0.status, "okay");
    assert_eq!(i2c0.clock_frequency, 100000);
    assert!(p.get_file_content().contains("foo = <1>;"));
}
