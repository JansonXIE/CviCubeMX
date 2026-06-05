#!/bin/bash
# CviCubeMX 重构前功能验证测试 - 一键执行脚本
# 用法: bash scripts/run_all_tests.sh [--verbose] [--watch]

set -e

VERBOSITY=""
EXTRA_ARGS=""

for arg in "$@"; do
  case $arg in
    --verbose)
      VERBOSITY="--reporter=verbose"
      ;;
    --watch)
      EXTRA_ARGS="--watch"
      ;;
  esac
done

echo "=========================================="
echo "  CviCubeMX 重构前功能验证测试套件"
echo "=========================================="
echo ""

# 前端 Vitest 测试
echo "--- 运行前端 (TypeScript) 测试 ---"
if [ -f package.json ]; then
  if [ ! -d node_modules ]; then
    echo "安装依赖..."
    npm install
  fi
  npx vitest run $VERBOSITY $EXTRA_ARGS
  echo ""
  echo "✅ 前端测试完成"
else
  echo "⚠️  未找到 package.json, 跳过前端测试"
fi

echo ""

# Rust 后端测试
echo "--- 运行后端 (Rust) 测试 ---"
if [ -f src-tauri/Cargo.toml ]; then
  cd src-tauri && cargo test && cd ..
  echo ""
  echo "✅ 后端测试完成"
else
  echo "⚠️  未找到 src-tauri/Cargo.toml, 跳过后端测试"
fi

echo ""
echo "=========================================="
echo "  全部测试执行完毕"
echo "=========================================="