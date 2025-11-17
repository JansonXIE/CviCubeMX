import pandas as pd
import os
import re
import sys

# --- 配置 ---

# 1. 输入的 Excel 文件名 (可通过命令行参数指定)
EXCEL_FILE = 'pins.xlsx'  # 默认值

# 2. Excel 中包含数据的表单名称（"第四个工作表" = 索引 3）
SHEET_NAME = 3

# 3. 数据从第几行开始（0表示第一行是标题）
HEADER_ROW_INDEX = 0 

# 4. Excel 中的列名
COL_PIN_NUM = "Pin Num"          # A列: 引脚编号
COL_PIN_NAME = "Pin Name"        # B列: 引脚名称
COL_FUNCTIONS_LIST = "Description" # I列: 功能描述

# 5. 输出的 C++ 文件名
OUTPUT_PIN_FUNCTIONS_CPP = 'initializePinFunctions.cpp'
OUTPUT_PIN_MAPPINGS_CPP = 'initializePinNameMappings.cpp'

# --- 新增：功能名称重映射规则 (用于 initializePinFunctions.cpp) ---
FUNCTION_NAME_REMAP = {
    "CR_4WTMS": "CV_2WTMS_CR_4WTMS",
    "CR_4WTCK": "CV_2WTCK_CR_4WTCK",
    "CR_4WTDI": "CV_SCL0__CR_4WTDI",
    "CR_4WTDO": "CV_SDA0__CR_4WTDO",
    "CR_SCL0": "CV_4WTDI_CR_SCL0",
    "CR_SDA0": "CV_4WTMS_CR_SDA0",
    "CR_2WTMS": "CV_4WTDO_CR_2WTMS",
    "CR_2WTCK": "CV_4WTCK_CR_2WTCK",
}

# --- 解析函数 (已更新) ---
def parse_function_select_cell(cell_content):
    """
    解析 "Description" (I列) 中的多行字符串。
    并应用 FUNCTION_NAME_REMAP 规则。
    """
    functions_list = []
    default_function = "GPIO" # 默认备用值
    
    if pd.isna(cell_content):
        return ["GPIO"], "GPIO"
        
    cell_content = str(cell_content)
    lines = cell_content.splitlines() # 按行分割
    
    found_default = False

    for line in lines:
        line = line.strip()
        
        # 跳过空行、标题行和 "Others" 行
        if not line or "function select" in line or "Others :" in line:
            continue
            
        # 查找 "index : function" 格式
        if ":" in line:
            parts = line.split(':', 1)
            if len(parts) == 2:
                # 提取冒号后的功能信息
                func_info = parts[1].strip()
                
                # 检查是否为默认值
                if "(default)" in func_info:
                    # 移除(default)并替换括号
                    func_name = func_info.replace("(default)", "").strip().replace("[", "_").replace("]", "")
                    # --- 应用重映射 ---
                    func_name = FUNCTION_NAME_REMAP.get(func_name, func_name)
                    
                    default_function = func_name
                    functions_list.append(func_name)
                    found_default = True
                else:
                    # 替换括号
                    func_name = func_info.strip().replace("[", "_").replace("]", "")
                    # --- 应用重映射 ---
                    func_name = FUNCTION_NAME_REMAP.get(func_name, func_name)

                    if func_name: # 确保不是空字符串
                        functions_list.append(func_name)

    # 如果在列表中没有找到 (default) 标记，但仍然有功能
    if not found_default and functions_list:
        # 查找第一个包含 "XGPIO" 的功能作为备用默认值
        gpio_fallback = [f for f in functions_list if "XGPIO" in f]
        if gpio_fallback:
            default_function = gpio_fallback[0]
        else:
            # 否则，只取第一个
            default_function = functions_list[0]
    
    # 如果解析后列表为空，则使用 GPIO
    if not functions_list:
        functions_list = ["GPIO"]
        default_function = "GPIO"
        
    # 确保默认功能本身也在列表中
    if default_function not in functions_list:
         functions_list.append(default_function)

    return functions_list, default_function

# --- 排序辅助函数 ---
def get_pin_sort_key(pin):
    """
    生成用于对引脚编号 (e.g., A2, A10, B1) 进行自然排序的键。
    键是一个元组 (letter_part, number_part)。
    """
    pin_num = pin['num']
    match = re.match(r"([A-Z]+)(\d+)", pin_num)
    if match:
        letter_part = match.group(1)
        number_part = int(match.group(2))
        return (letter_part, number_part)
    else:
        # 针对 QFN 引脚（纯数字）或其他格式的回退
        try:
            # 尝试视为纯数字
            return ("", int(pin_num))
        except ValueError:
            # 视为普通字符串
            return (pin_num, 0)
            
# --- 脚本 ---

# --- 已更新：load_pin_data 函数 ---
def load_pin_data(excel_file):
    """
    从 Excel 文件加载和解析引脚数据。
    将清理 "Pin Name" (例如 PAD_ETH_RXM___EPHY_TXP -> PAD_ETH_RXM)。
    """
    try:
        df = pd.read_excel(
            excel_file, 
            sheet_name=SHEET_NAME, 
            header=HEADER_ROW_INDEX,
            dtype=str  # 将所有列都读取为字符串
        )
    except FileNotFoundError:
        print(f"错误: 找不到文件 '{excel_file}'。")
        print("请确保 Excel 文件与脚本在同一目录中，或提供完整路径。")
        return None
    except ValueError as e:
        if "Worksheet" in str(e):
            print(f"错误: 找不到工作表。请确保索引 '{SHEET_NAME}' (即第四个工作表) 存在。")
            return None
        print(f"读取 Excel 时出错: {e}")
        return None
    except Exception as e:
        print(f"读取 Excel 时出错: {e}")
        return None

    print(f"成功从 '{excel_file}' 的第 {SHEET_NAME + 1} 个工作表加载 {len(df)} 行数据。")
    
    pins = []
    all_functions = set() 

    required_cols = {COL_PIN_NUM, COL_PIN_NAME, COL_FUNCTIONS_LIST}
    if not required_cols.issubset(df.columns):
        print(f"错误: Excel 文件缺少必要的列。")
        print(f"需要: {required_cols}")
        print(f"实际: {set(df.columns)}")
        print("请检查 Excel 标题行是否与脚本配置完全一致（包括大小写和空格）。")
        return None

    for index, row in df.iterrows():
        pin_num = row.get(COL_PIN_NUM)
        pin_name = row.get(COL_PIN_NAME)
        functions_str = row.get(COL_FUNCTIONS_LIST) 

        if pd.isna(pin_num) or pd.isna(pin_name) or pin_num.strip() == "" or pin_name.strip() == "":
            print(f"警告: 跳过第 {index + 2} 行，Pin Num 或 Pin Name 为空。")
            continue
            
        pin_num = pin_num.strip()
        pin_name = pin_name.strip()

        # --- 新增：清理 Pin Name (用于 initializePinNameMappings.cpp) ---
        if "___" in pin_name:
            original_pin_name = pin_name
            pin_name = pin_name.split("___")[0].strip()
            print(f"    -> 提示: Pin Name '{original_pin_name}' 已清理为 '{pin_name}'.")
        # --- 结束新增 ---
            
        functions_list, default_func = parse_function_select_cell(functions_str)

        pins.append({
            "num": pin_num,
            "name": pin_name,
            "functions": functions_list,
            "default": default_func
        })
        
        all_functions.update(functions_list) 

    print(f"成功解析 {len(pins)} 个引脚。")
    return pins, all_functions

def generate_pin_mappings_cpp(pins, output_file):
    """生成 initializePinNameMappings.cpp (带排序功能)"""
    print(f"正在生成 {output_file}...")
    
    try:
        sorted_pins = sorted(pins, key=get_pin_sort_key)
        print("已根据 Pin Num (Ax, Bx, ...) 排序。")
    except Exception as e:
        print(f"排序时出错: {e}. 将使用原始顺序。")
        sorted_pins = pins

    try:
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write("void MainWindow::initializePinNameMappings()\n")
            f.write("{\n")
            f.write("    // BGA引脚映射（字母数字格式）- 由脚本自动生成并排序\n")
            
            current_letter = "" 
            
            for pin in sorted_pins:
                
                pin_letter = ""
                match = re.match(r"([A-Z]+)", pin['num'])
                if match:
                    pin_letter = match.group(1)

                if current_letter != "" and pin_letter != current_letter:
                    f.write("\n") 
                current_letter = pin_letter
            
                # pin["name"] 现在是清理后的名称
                f.write(f'    m_pinNameMappings["{pin["num"]}"] = "{pin["name"]}";\n')
                
            f.write("}\n")
        print(f"成功写入 {output_file}。")
    except Exception as e:
        print(f"写入 {output_file} 时出错: {e}")

def generate_pin_functions_cpp(pins, all_functions, output_file):
    """生成 initializePinFunctions.cpp (带排序和修复)"""
    print(f"正在生成 {output_file}...")
    
    try:
        sorted_pins = sorted(pins, key=get_pin_sort_key)
        print("已根据 Pin Num (Ax, Bx, ...) 排序。")
    except Exception as e:
        print(f"排序时出错: {e}. 将使用原始顺序。")
        sorted_pins = pins

    try:
        with open(output_file, 'w', encoding='utf-8') as f:
            # 文件头和构造函数
            f.write('#include "pinfunction.h"\n\n')
            f.write('PinFunction::PinFunction()\n')
            f.write('{\n')
            f.write('    initializePinFunctions();\n')
            f.write('}\n\n')

            # 辅助函数
            f.write('QStringList PinFunction::getSupportedFunctions(const QString& pinName) const\n')
            f.write('{\n')
            f.write('    return m_pinFunctions.value(pinName, QStringList() << "GPIO");\n')
            f.write('}\n\n')
            
            f.write('QString PinFunction::getDefaultFunction(const QString& pinName) const\n')
            f.write('{\n')
            f.write('    return m_defaultFunctions.value(pinName, "GPIO");\n')
            f.write('}\n\n')

            f.write('QString PinFunction::getFunctionMacroName(const QString& pinName, const QString& function) const\n')
            f.write('{\n')
            f.write('    QString key = pinName + "_" + function;\n')
            f.write('    return m_functionMacros.value(key, function.toUpper());\n')
            f.write('}\n\n')

            f.write('bool PinFunction::isPinFunctionSupported(const QString& pinName, const QString& function) const\n')
            f.write('{\n')
            f.write('    return m_pinFunctions.value(pinName).contains(function);\n')
            f.write('}\n\n')

            # --- 主要的 initializePinFunctions ---
            f.write('void PinFunction::initializePinFunctions()\n')
            f.write('{\n')
            
            current_letter = "" 
            for pin in sorted_pins:
            
                pin_letter = ""
                match = re.match(r"([A-Z]+)", pin['num'])
                if match:
                    pin_letter = match.group(1)

                if current_letter != "" and pin_letter != current_letter:
                    f.write('\n') 
                current_letter = pin_letter

                f.write(f'    // {pin["num"]} 引脚功能定义（Pin name: {pin["name"]}）\n')
                f.write(f'    // 引脚名称使用实际的PAD名称\n')
                
                # pin["name"] 是清理后的键
                # pin["functions"] 包含重映射后的功能名称
                functions_str = '"' + '" << "'.join(pin["functions"]) + '"'
                f.write(f'    m_pinFunctions["{pin["name"]}"] = QStringList() << {functions_str};\n')
                
                f.write(f'    m_defaultFunctions["{pin["name"]}"] = "{pin["default"]}";\n')
                
                f.write('    // 功能宏映射\n')
                for func in sorted(pin["functions"]):
                    f.write(f'    m_functionMacros["{func}"] = "{func}";\n')
                f.write('\n') 
            
            f.write('\n') 
            
            # 插入您原始文件中的 BGA/QFN 备用逻辑
            f.write('    // --- 原始文件中的备用逻辑 ---\n')
            f.write('    // 为其他引脚添加基本功能支持（可以根据需要扩展）\n')
            f.write('    QStringList basicFunctions = QStringList() << "GPIO" << "ADC" << "PWM" << "I2C" << "UART" << "SPI";\n\n')
            
            f.write('    // QFN引脚（数字编号1-88）\n')
            f.write('    for (int i = 1; i <= 88; ++i) {\n')
            f.write('        QString pinName = QString::number(i);\n')
            f.write('        if (!m_pinFunctions.contains(pinName)) {\n')
            f.write('            m_pinFunctions[pinName] = basicFunctions;\n')
            f.write('            m_defaultFunctions[pinName] = "GPIO";\n')
            f.write('        }\n')
            f.write('    }\n\n')
            
            f.write('    // BGA引脚（A1-R15格式，跳过I行）\n')
            f.write('    QString rows = "ABCDEFGHJKLMNOPQR";  // 跳过I\n')
            f.write('    for (int r = 0; r < rows.length(); ++r) {\n')
            f.write('        for (int c = 1; c <= 15; ++c) {\n')
            f.write('            QString pinName = QString("%1%2").arg(rows[r]).arg(c);\n\n')
            
            f.write('            // 跳过四个角的引脚\n')
            f.write('            bool isCorner = (r == 0 && c == 1) ||          // A1 - 左上角\n')
            f.write('                           (r == 0 && c == 15) ||          // A15 - 右上角\n')
            f.write('                           (r == rows.length()-1 && c == 1) ||   // R1 - 左下角\n')
            f.write('                           (r == rows.length()-1 && c == 15);    // R15 - 右下角\n\n')
            
            f.write('            if (!isCorner && !m_pinFunctions.contains(pinName)) {\n')
            f.write('                m_pinFunctions[pinName] = basicFunctions;\n')
            f.write('                m_defaultFunctions[pinName] = "GPIO";\n')
            f.write('            }\n')
            f.write('        }\n')
            f.write('    }\n')
            
            f.write('}\n')
            
        print(f"成功写入 {output_file}。")
    except Exception as e:
        print(f"写入 {output_file} 时出错: {e}")

def main():
    """主执行函数"""
    # 解析命令行参数
    excel_file = EXCEL_FILE  # 使用默认值
    
    if len(sys.argv) > 1:
        excel_file = sys.argv[1]
        print(f"使用命令行指定的文件: {excel_file}")
    else:
        print(f"未指定文件，使用默认文件: {excel_file}")
        print(f"提示: 可以使用 'python {sys.argv[0]} <excel文件路径>' 指定Excel文件")
    
    pin_data = load_pin_data(excel_file)
    
    if pin_data:
        pins, all_functions = pin_data
        
        generate_pin_mappings_cpp(pins, OUTPUT_PIN_MAPPINGS_CPP)
        print("-" * 20)
        generate_pin_functions_cpp(pins, all_functions, OUTPUT_PIN_FUNCTIONS_CPP)
        print("-" * 20)
        print("脚本执行完毕。")

if __name__ == "__main__":
    main()