#!/bin/bash
# CviCubeMX 一键端到端自动化测试脚本
# 依次运行前端构建、前端 Vitest 测试、Rust 单元/集成测试、Tauri 无捆绑打包编译检查

set -e

# 获取脚本所在目录，确保在根目录执行
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR/.."

echo "=================================================="
echo "🚀 开始执行 CviCubeMX 端到端自动化校验流程"
echo "=================================================="

# 1. 检查前端依赖并编译
echo ""
echo "📦 步骤 1: 进行前端编译 (npm run build)..."
if [ ! -d node_modules ]; then
    echo "Installing frontend dependencies..."
    npm install
fi
npm run build
echo "✅ 前端编译通过"

# 2. 运行前端 Vitest 测试
echo ""
echo "🧪 步骤 2: 运行前端 Vitest 全套测试..."
npx vitest run
echo "✅ 前端 Vitest 测试全部通过"

# 3. 运行 Rust 后端编译与单元/集成测试
echo ""
echo "🦀 步骤 3: 运行后端 Rust 测试 (cargo test)..."
cd src-tauri
cargo test
cd ..
echo "✅ 后端 Rust 单元与集成测试全部通过"

# 4. 运行 Tauri 开发打包检查 (tauri build --no-bundle)
echo ""
echo "🏗️  步骤 4: 验证 Tauri 应用打包编译 (npx tauri build --no-bundle)..."
npx tauri build --no-bundle
echo "✅ Tauri 应用打包验证成功，前端与 Rust 桥接无异常"

echo ""
echo "=================================================="
echo "🎉 恭喜！所有端到端自动化校验全部顺利通过！"
echo "=================================================="
