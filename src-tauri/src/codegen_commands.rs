/// 代码生成器命令模块 (对应重构计划 M7)
///
/// Tauri command wrappers for codegen.rs:
/// - generate_code: 生成 cvi_board_init.c 代码
/// - update_existing_code: 增量更新已有文件

use crate::codegen::{generate_code as generate_code_impl, PinConfig, update_existing_code as update_existing_code_impl};

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