#!/bin/bash
# 脚本用于解决 Git 合并冲突

set -e

echo "=== 开始解决 Git 合并冲突 ==="
echo ""

# 进入项目目录
cd "$(dirname "$0")"

# 检查当前合并状态
echo "1. 检查 Git 合并状态..."
git status

echo ""
echo "2. 标记所有文件为已解决..."
git add -A

echo ""
echo "3. 完成合并..."
git merge --continue -m "Merge origin/main: resolve conflict with local frontend/backend changes"

echo ""
echo "=== 合并完成！==="
git log --oneline -5
