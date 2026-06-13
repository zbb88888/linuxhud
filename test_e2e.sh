#!/bin/bash

# LinuxHUD E2E 测试脚本
# 测试实际显示器显示效果

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== LinuxHUD E2E 测试 ===${NC}"
echo ""

# 检查权限
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}错误：需要 root 权限运行此测试${NC}"
    echo "请使用: sudo ./test_e2e.sh"
    exit 1
fi

# 检查构建
if [ ! -f "build/bin/linuxhud" ]; then
    echo -e "${YELLOW}警告：未找到构建文件，正在构建...${NC}"
    make build
fi

echo -e "${BLUE}1. 检查显示器连接${NC}"
CONNECTOR_STATUS=$(sudo ./build/bin/test_drm 2>&1 | grep "CONNECTED" | head -1)
if [ -z "$CONNECTOR_STATUS" ]; then
    echo -e "${RED}错误：未检测到已连接的显示器${NC}"
    exit 1
fi
echo -e "${GREEN}✓ 检测到显示器${NC}"
echo "  $CONNECTOR_STATUS"

echo ""
echo -e "${BLUE}2. 测试 HUD 显示${NC}"
echo "  将在屏幕上显示 'Hello LinuxHUD!' 文本"
echo "  按 Ctrl+C 停止测试"
echo ""
echo -e "${YELLOW}  请观察屏幕左上角是否显示文本...${NC}"
echo ""

# 运行 HUD 程序，5秒后自动停止
timeout 5 sudo ./build/bin/linuxhud -t "Hello LinuxHUD!" -x 50 -y 50 -w 400 -h 100 -a 200 -s 28 -v 2>&1 || true

echo ""
echo -e "${GREEN}✓ E2E 测试完成${NC}"
echo ""
echo "如果屏幕上显示了文本，说明 HUD 工作正常。"
echo "如果没有显示，请检查："
echo "  1. 显示器是否正确连接"
echo "  2. 显卡驱动是否支持 DRM/KMS"
echo "  3. Overlay Plane 是否可用"
