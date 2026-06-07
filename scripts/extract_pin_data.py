#!/usr/bin/env python3
"""
从 pinfunction.cpp 提取所有引脚功能数据，生成 JSON 供 Rust 后端使用。

输出格式:
[
  {
    "pin_num": "A2",           // BGA位置编号或QFN数字编号
    "pin_name": "PAD_MIPI_TXM4", // 已清理的PAD名称
    "supported_functions": ["VI0_D_15", ...],
    "default_function": "XGPIOC_18"
  },
  ...
]
"""

import re
import json
import sys

PINFUNCTION_CPP = '../src/pinfunction.cpp'

def clean_pin_name(pin_name):
    """Pin Name 含 ___ 时只取第一段"""
    if '___' in pin_name:
        return pin_name.split('___')[0]
    return pin_name

def extract_pin_data(filepath):
    """解析 pinfunction.cpp 提取所有 m_pinFunctions 和 m_defaultFunctions"""

    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    pins = {}

    # 匹配 m_pinFunctions["<name>"] = QStringList() << "func1" << "func2" ...;
    # 也匹配 m_pinFunctions["<name>"] = QStringList()<< "func1" ...; (无空格版本)
    pin_func_pattern = re.compile(
        r'm_pinFunctions\["([^"]+)"\]\s*=\s*QStringList\(\s*\)\s*<<\s*(.+?)\s*;',
        re.DOTALL
    )

    for match in pin_func_pattern.finditer(content):
        pin_name = match.group(1)
        functions_str = match.group(2)

        # 提取所有 "func_name"
        functions = re.findall(r'"([^"]+)"', functions_str)

        cleaned_name = clean_pin_name(pin_name)
        pins[cleaned_name] = {
            'pin_name': cleaned_name,
            'supported_functions': functions,
            'default_function': None,  # 待填充
            'pin_num': None,  # 待从注释提取
        }

    # 匹配 m_defaultFunctions["<name>"] = "<default>";
    default_pattern = re.compile(
        r'm_defaultFunctions\["([^"]+)"\]\s*=\s*"([^"]+)"',
    )

    for match in default_pattern.finditer(content):
        pin_name = match.group(1)
        default_func = match.group(2)
        cleaned_name = clean_pin_name(pin_name)
        if cleaned_name in pins:
            pins[cleaned_name]['default_function'] = default_func

    # 从注释提取 pin_num (BGA位置或QFN编号)
    # 格式1: // A2 引脚功能定义（Pin name: PAD_MIPI_TXM4）
    # 格式2: // A2 引脚功能定义 Pin name: PAD_MIPI_TXM4
    # 格式3: // 83 引脚功能定义（Pin name: PAD_MIPI_TXM2）
    # 格式4: // 2 引脚功能定义（Pin name: PAD_AUD_AINL_MIC）

    # 先找带注释的
    comment_pattern = re.compile(
        r'//\s*([A-Z]+\d+|\d+)\s+引脚功能定义.*?Pin\s+name:\s*(\S+)',
    )

    # 也处理括号版本
    comment_pattern_paren = re.compile(
        r'//\s*([A-Z]+\d+|\d+)\s+引脚功能定义.*?Pin\s+name:\s*(\S+)\）',
    )

    # 遍历所有注释匹配
    for match in comment_pattern.finditer(content):
        pin_num = match.group(1)
        pin_name_raw = match.group(2).rstrip('）')  # 去掉可能残留的右括号
        cleaned_name = clean_pin_name(pin_name_raw)
        if cleaned_name in pins:
            pins[cleaned_name]['pin_num'] = pin_num

    # 构建最终列表
    result = []
    for name, data in pins.items():
        if data['default_function'] is None:
            print(f"WARNING: {name} has no default_function, using first function or GPIO")
            if data['supported_functions']:
                # 找第一个含 XGPIO 的
                gpio_funcs = [f for f in data['supported_functions'] if 'XGPIO' in f]
                if gpio_funcs:
                    data['default_function'] = gpio_funcs[0]
                else:
                    data['default_function'] = data['supported_functions'][0]
            else:
                data['default_function'] = 'GPIO'

        if data['pin_num'] is None:
            print(f"WARNING: {name} has no pin_num from comment")
            data['pin_num'] = ''

        result.append({
            'pin_num': data['pin_num'],
            'pin_name': data['pin_name'],
            'supported_functions': data['supported_functions'],
            'default_function': data['default_function'],
        })

    # 按自然排序
    def sort_key(item):
        num = item['pin_num']
        match = re.match(r'([A-Z]+)(\d+)', num)
        if match:
            return (match.group(1), int(match.group(2)))
        try:
            return ('', int(num))
        except ValueError:
            return (num, 0)

    result.sort(key=sort_key)

    return result

def main():
    filepath = PINFUNCTION_CPP
    if len(sys.argv) > 1:
        filepath = sys.argv[1]

    print(f"Extracting pin data from: {filepath}")
    pins = extract_pin_data(filepath)
    print(f"Extracted {len(pins)} pins")

    # 输出 JSON
    output_path = '../src-tauri/pin_data.json'
    if len(sys.argv) > 2:
        output_path = sys.argv[2]

    with open(output_path, 'w', encoding='utf-8') as f:
        json.dump(pins, f, indent=2, ensure_ascii=False)

    print(f"Written to: {output_path}")

    # 验证关键样本
    sample_pins = {p['pin_name']: p for p in pins}

    # 验证 PAD_MIPI_TXM4
    if 'PAD_MIPI_TXM4' in sample_pins:
        p = sample_pins['PAD_MIPI_TXM4']
        assert len(p['supported_functions']) == 8, f"PAD_MIPI_TXM4 should have 8 functions, got {len(p['supported_functions'])}"
        assert p['default_function'] == 'XGPIOC_18', f"PAD_MIPI_TXM4 default should be XGPIOC_18, got {p['default_function']}"
        print("[OK] PAD_MIPI_TXM4: 8 functions, default=XGPIOC_18")

    # 验证 RSTN
    if 'RSTN' in sample_pins:
        p = sample_pins['RSTN']
        assert len(p['supported_functions']) == 1, f"RSTN should have 1 function, got {len(p['supported_functions'])}"
        assert p['default_function'] == 'RSTN'
        print("[OK] RSTN: 1 function, default=RSTN")

    # 验证 USB_VBUS_DET
    if 'USB_VBUS_DET' in sample_pins:
        p = sample_pins['USB_VBUS_DET']
        assert p['supported_functions'][0] == 'USB_VBUS_DET'
        assert p['default_function'] == 'USB_VBUS_DET'
        print("[OK] USB_VBUS_DET: first=USB_VBUS_DET, default=USB_VBUS_DET")

    # 验证 SD0_D1
    if 'SD0_D1' in sample_pins:
        p = sample_pins['SD0_D1']
        assert p['default_function'] == 'SDIO0_D_1'
        print("[OK] SD0_D1: default=SDIO0_D_1")

    # 验证 PAD_ETH_RXM (___ 清理)
    if 'PAD_ETH_RXM' in sample_pins:
        p = sample_pins['PAD_ETH_RXM']
        print(f"[OK] PAD_ETH_RXM (cleaned from ___): {len(p['supported_functions'])} functions, default={p['default_function']}")

    print(f"\nTotal unique pin names: {len(sample_pins)}")

    # 统计有pin_num的和无pin_num的
    with_num = sum(1 for p in pins if p['pin_num'])
    without_num = sum(1 for p in pins if not p['pin_num'])
    print(f"Pins with pin_num: {with_num}, without: {without_num}")

if __name__ == '__main__':
    main()