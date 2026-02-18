#!/bin/bash

# 配置 Git 用户
git config --global user.name "GitHub Actions"
git config --global user.email "actions@github.com"

# 只初始化 3rdparty 下的子模块
echo "初始化 3rdparty 子模块..."
git submodule update --init 3rdparty/*

# 进入 3rdparty 目录更新子模块
cd 3rdparty
for dir in */; do
    if [ -d "$dir/.git" ]; then
        echo "更新子模块: $dir"
        cd "$dir"
        git pull origin $(git branch --show-current || echo "main")
        cd ..
    fi
done
cd ..

# 更新父仓库中的子模块引用
echo "更新父仓库中的子模块引用..."
git submodule update --remote --recursive 3rdparty/*

# 提交更新
git add --all

if ! git diff-index --quiet HEAD --; then
    git commit -m "Automated 3rdparty submodule update $(date)"
    git push
else
    echo "没有 3rdparty 子模块更新"
fi
