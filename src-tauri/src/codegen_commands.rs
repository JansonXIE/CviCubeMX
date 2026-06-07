/// 代码生成器命令模块 (对应重构计划 M7)
///
/// Tauri command wrappers for codegen.rs:
/// - generate_code: 生成 cvi_board_init.c 代码
/// - update_existing_code: 增量更新已有文件
use crate::codegen::{
    generate_code as generate_code_impl, update_existing_code as update_existing_code_impl,
    PinConfig,
};

/// 生成 cvi_board_init.c 代码
///
/// 对应 C++ CodeGenerator::generateCode()
/// 前端传入: chip_type, pin_configs (仅 user_configured=true 的引脚), output_path (可选)
/// 返回: 生成的 C 代码内容
#[tauri::command]
pub fn generate_code(
    chip_type: String,
    pin_configs: Vec<PinConfig>,
    output_path: Option<String>,
) -> Result<String, String> {
    generate_code_impl(&chip_type, &pin_configs, output_path.as_deref())
}

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
