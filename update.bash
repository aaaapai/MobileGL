#!/bin/bash

# 初始化并更新所有子模块
git submodule update --init --recursive

# 更新所有子模块到最新版本
git submodule foreach --recursive 'git checkout $(git config -f $toplevel/.gitmodules submodule.$name.branch || echo main)'
git submodule foreach --recursive 'git pull origin $(git config -f $toplevel/.gitmodules submodule.$name.branch || echo main)'

# 提交父仓库中的子模块引用更新
git add --all

# 检查是否有更改
if ! git diff-index --quiet HEAD --; then
    git config --global user.name "GitHub Actions"
    git config --global user.email "actions@github.com"
    git commit -m "Automated submodule update $(date)"
    git push
else
    echo "No changes to commit"
fi
