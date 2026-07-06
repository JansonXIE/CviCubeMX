pub mod ai_chat;
pub mod chip_spec;
pub mod clock_calc;
pub mod clock_commands;
pub mod codegen;
pub mod codegen_commands;
pub mod dts_parser;
pub mod dts_writer;
pub mod flash;
pub mod memory;
pub mod peripheral;
pub mod pin_data;
pub mod pin_data_tool;
pub mod pin_mux;

use peripheral::DtsState;

#[tauri::command]
fn greet(name: String) -> String {
    format!("Hello, {}! You've been greeted from Rust!", name)
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_dialog::init())
        .plugin(tauri_plugin_opener::init())
        .manage(DtsState::new())
        .invoke_handler(tauri::generate_handler![
            greet,
            ai_chat::send_ai_message,
            ai_chat::save_ai_config,
            ai_chat::load_ai_config,
 ai_chat::list_chat_sessions,
 ai_chat::get_chat_session,
 ai_chat::create_chat_session,
 ai_chat::save_chat_session,
 ai_chat::delete_chat_session,
            chip_spec::load_chip_spec,
            pin_data::load_pin_data,
            pin_data::set_pin_function,
            pin_data::clear_pin_functions,
            pin_mux::get_mux_functions,
            clock_commands::compute_clock_tree,
            clock_commands::save_module_positions,
            clock_commands::load_module_positions,
            clock_commands::export_clock_defconfig,
            peripheral::load_dts_peripherals,
            peripheral::get_dts_content,
            peripheral::set_peripheral_status,
            peripheral::set_peripheral_clock_frequency,
            peripheral::set_peripheral_pwm_cells,
            peripheral::set_peripheral_current_speed,
            peripheral::set_peripheral_sysdma_channels,
            peripheral::get_peripheral_raw_properties,
            peripheral::set_peripheral_raw_property,
            peripheral::delete_peripheral_raw_property,
            // M5: Memory configuration commands
            memory::load_memory_regions,
            memory::validate_memory,
            memory::export_memory_json,
            memory::export_memory_defconfig,
            // M6: Flash partition commands
            flash::load_partitions,
            flash::read_flash_board_info,
            flash::validate_partitions,
            flash::export_flash_json,
            flash::export_flash_defconfig,
            // M7: Codegen commands
            codegen_commands::update_existing_code,
            codegen_commands::validate_sdk_path,
            codegen_commands::generate_board_init_code,
            codegen_commands::read_board_init_config,
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
