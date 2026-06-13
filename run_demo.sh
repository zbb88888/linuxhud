#!/bin/bash

# LinuxHUD 演示脚本

echo "=== LinuxHUD 演示 ==="
echo ""

# 检查权限
if [ "$EUID" -ne 0 ]; then
    echo "请使用 sudo 运行此脚本"
    exit 1
fi

# 检查 DRM 设备
if [ ! -d "/dev/dri" ]; then
    echo "错误：未找到 DRM 设备"
    exit 1
fi

# 编译程序
echo "1. 编译 LinuxHUD..."
make clean
make

if [ $? -ne 0 ]; then
    echo "编译失败"
    exit 1
fi

echo ""
echo "2. 运行 DRM 测试..."
./test_drm

echo ""
echo "3. 启动 LinuxHUD 演示..."
echo "   按 Ctrl+C 停止"
echo ""

# 运行演示
./linuxhud -t "Hello LinuxHUD!" \
           -x 50 \
           -y 50 \
           -w 400 \
           -h 120 \
           -a 200 \
           -s 28 \
           -c "00FF00" \
           -b "000000" \
           -v

echo ""
echo "演示结束"
