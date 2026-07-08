/// 代码生成器命令模块 (对应重构计划 M7)
///
/// Tauri command wrappers for codegen.rs:
/// - update_existing_code: 增量更新已有文件
/// - generate_board_init_code: 生成或增量更新 SDK 内的 cvi_board_init.c
use crate::codegen::{
    generate_code as generate_code_impl, parse_board_init as parse_board_init_impl,
    update_existing_code as update_existing_code_impl, PinConfig,
};

/// 增量更新已有 cvi_board_init.c 文件
///
/// 对应 C++ CodeGenerator::updateExistingFile()
/// 前端传入: file_path (已有文件路径), pin_configs (仅 user_configured=true 的引脚)
/// 返回: 操作结果消息
#[tauri::command]
pub fn update_existing_code(
    file_path: String,
    pin_configs: Vec<PinConfig>,
) -> Result<String, String> {
    update_existing_code_impl(&file_path, &pin_configs)
}

/// 验证 SDK 路径是否合法
///
/// 检查该路径下是否存在 build 目录、build/boards/cv184x 目录以及 build/boards/default/dts/cv184x 目录
#[tauri::command]
pub fn validate_sdk_path(path: String) -> Result<bool, String> {
    let base_path = std::path::Path::new(&path);
    if !base_path.exists() {
        return Ok(false);
    }

    let build_dir = base_path.join("build");
    if !build_dir.exists() || !build_dir.is_dir() {
        return Ok(false);
    }

    let crit1 = base_path.join("build/boards/cv184x");
    let crit2 = base_path.join("build/boards/default/dts/cv184x");

    Ok(crit1.exists() && crit1.is_dir() && crit2.exists() && crit2.is_dir())
}

/// 自动生成或增量更新 SDK 内的 cvi_board_init.c 文件
///
/// 对应 C++ CodeGenerator::generateCode() 中的路径自动存在性判定
#[tauri::command]
pub fn generate_board_init_code(
    sdk_path: String,
    chip_type: String,
    pin_configs: Vec<PinConfig>,
) -> Result<String, String> {
    let relative_path = format!("build/boards/cv184x/{}/u-boot/cvi_board_init.c", chip_type);
    let full_path = std::path::Path::new(&sdk_path).join(&relative_path);

    if full_path.exists() {
        // 如果文件存在，执行增量更新并写回
        update_existing_code_impl(&full_path.to_string_lossy(), &pin_configs)
    } else {
        // 如果文件不存在，生成完整的 C 初始化代码并写入该物理路径
        let code = generate_code_impl(&chip_type, &pin_configs, None)?;
        // 自动创建父级目录（如果不存在）
        if let Some(parent) = full_path.parent() {
            std::fs::create_dir_all(parent).map_err(|e| format!("无法创建目录: {}", e))?;
        }
        std::fs::write(&full_path, &code).map_err(|e| format!("无法写入代码文件: {}", e))?;
        Ok("File generated successfully".to_string())
    }
}

/// 读取并解析 SDK 内已有的 cvi_board_init.c，恢复引脚复用配置
///
/// 读取 build/boards/cv184x/{chip_type}/u-boot/cvi_board_init.c，
/// 解析其中的 PINMUX_CONFIG 语句，并写入后端用户配置 (USER_CONFIG)，
/// 使前端随后重新 load_pin_data 时能反映已保存的复用状态（而不是每次都是默认值）。
///
/// 若文件不存在，返回空列表（视为「尚未生成」而非错误）。
/// 返回解析出的 PinConfig 列表，供前端按需使用。
#[tauri::command]
pub fn read_board_init_config(
    sdk_path: String,
    chip_type: String,
) -> Result<Vec<PinConfig>, String> {
    let relative_path = format!("build/boards/cv184x/{}/u-boot/cvi_board_init.c", chip_type);
    let full_path = std::path::Path::new(&sdk_path).join(&relative_path);

    // The selected SDK path is the source of truth for restored pin state.
    // Clear stale runtime config before applying the current SDK file.
    crate::pin_data::clear_pin_functions(chip_type.clone())?;
    if !full_path.exists() {
        return Ok(Vec::new());
    }

    let content = std::fs::read_to_string(&full_path)
        .map_err(|e| format!("无法读取 cvi_board_init.c: {}", e))?;

    let configs = parse_board_init_impl(&content);

    // 写入后端用户配置，使 load_pin_data 反映恢复出的状态 (含二级 mux 的 state)
    let pairs: Vec<(String, String, Option<String>)> = configs
        .iter()
        .map(|c| (c.pin_name.clone(), c.function.clone(), c.state.clone()))
        .collect();
    crate::pin_data::seed_user_configs(&chip_type, &pairs);

    Ok(configs)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::pin_data;

    fn temp_sdk_path(name: &str) -> std::path::PathBuf {
        let path = std::env::temp_dir().join(format!("cvicubemx_{}_{}", name, std::process::id()));
        let _ = std::fs::remove_dir_all(&path);
        path
    }

    fn find_pin(chip_type: &str, pin_name: &str) -> pin_data::PinInfo {
        pin_data::load_pin_data(chip_type.to_string())
            .unwrap()
            .into_iter()
            .find(|pin| pin.pin_name == pin_name)
            .unwrap()
    }

    #[test]
    fn read_board_init_config_clears_when_file_missing() {
        let chip_type = "cv1842hp";
        pin_data::set_pin_function(
            chip_type.to_string(),
            "PAD_MIPI_TXM4".to_string(),
            "UART0_TX".to_string(),
            None,
        )
        .unwrap();

        let configured = find_pin(chip_type, "PAD_MIPI_TXM4");
        assert_eq!(configured.current_function, "UART0_TX");
        assert!(configured.user_configured);

        let sdk_path = temp_sdk_path("missing_board_init");
        std::fs::create_dir_all(&sdk_path).unwrap();

        let configs = read_board_init_config(
            sdk_path.to_string_lossy().to_string(),
            chip_type.to_string(),
        )
        .unwrap();

        assert!(configs.is_empty());
        let reset = find_pin(chip_type, "PAD_MIPI_TXM4");
        assert_eq!(reset.current_function, "XGPIOC_18");
        assert!(!reset.user_configured);

        let _ = std::fs::remove_dir_all(sdk_path);
    }

    #[test]
    fn read_board_init_config_replaces_existing_chip_config() {
        let chip_type = "cv1842hp";
        pin_data::set_pin_function(
            chip_type.to_string(),
            "PAD_MIPI_TXM4".to_string(),
            "UART0_TX".to_string(),
            None,
        )
        .unwrap();
        pin_data::set_pin_function(
            chip_type.to_string(),
            "CAM_MCLK0".to_string(),
            "CAM_MCLK0".to_string(),
            None,
        )
        .unwrap();

        let sdk_path = temp_sdk_path("replace_board_init");
        let board_dir = sdk_path.join(format!("build/boards/cv184x/{}/u-boot", chip_type));
        std::fs::create_dir_all(&board_dir).unwrap();
        std::fs::write(
 board_dir.join("cvi_board_init.c"),
 "void cvi_board_init(void) {\n// Generated PINMUX configurations\nPINMUX_CONFIG(PAD_MIPI_TXM4, SD1_CLK);\nreturn0;\n}\n",
 )
 .unwrap();

        let configs = read_board_init_config(
            sdk_path.to_string_lossy().to_string(),
            chip_type.to_string(),
        )
        .unwrap();

        assert_eq!(configs.len(), 1);
        let replaced = find_pin(chip_type, "PAD_MIPI_TXM4");
        assert_eq!(replaced.current_function, "SD1_CLK");
        assert!(replaced.user_configured);

        let removed = find_pin(chip_type, "CAM_MCLK0");
        assert_eq!(removed.current_function, "XGPIOA_0");
        assert!(!removed.user_configured);

        let _ = std::fs::remove_dir_all(sdk_path);
    }
}
