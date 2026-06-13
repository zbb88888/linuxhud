#!/bin/bash

# LinuxHUD 虚拟终端测试脚本
# 在虚拟终端中运行此脚本来测试 HUD 显示

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}   LinuxHUD 虚拟终端测试${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# 检查 root 权限
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}错误：需要 root 权限${NC}"
    echo "请使用: sudo $0"
    exit 1
fi

# 检查 DRM 设备
if [ ! -d "/dev/dri" ]; then
    echo -e "${RED}错误：未找到 DRM 设备${NC}"
    exit 1
fi

# 检查显示器
CONNECTOR_STATUS=$(./build/bin/test_drm 2>&1 | grep "CONNECTED" | head -1)
if [ -z "$CONNECTOR_STATUS" ]; then
    echo -e "${RED}错误：未检测到显示器${NC}"
    exit 1
fi

echo -e "${GREEN}✓ DRM 设备就绪${NC}"
echo -e "${GREEN}✓ 显示器已连接${NC}"
echo ""

# 检查是否在虚拟终端
TTY=$(tty)
if [[ "$TTY" != /dev/tty* ]]; then
    echo -e "${YELLOW}提示：建议在虚拟终端中运行此脚本${NC}"
    echo ""
    echo "操作步骤:"
    echo "1. 按 Ctrl+Alt+F2 切换到虚拟终端"
    echo "2. 登录"
    echo "3. 运行: cd $(pwd) && sudo $0"
    echo "4. 测试完成后按 Ctrl+Alt+F7 返回图形界面"
    echo ""
    read -p "是否继续测试? (y/N): " CONTINUE
    if [ "$CONTINUE" != "y" ] && [ "$CONTINUE" != "Y" ]; then
        exit 0
    fi
fi

# 检查是否有显示管理器占用
GDM_PID=$(pgrep -x gdm3 || pgrep -x gdm || echo "")
XORG_PID=$(pgrep -x Xorg || pgrep -x X || echo "")

if [ -n "$GDM_PID" ] || [ -n "$XORG_PID" ]; then
    echo -e "${YELLOW}警告：检测到显示管理器${NC}"
    [ -n "$GDM_PID" ] && echo "  - GDM (PID: $GDM_PID)"
    [ -n "$XORG_PID" ] && echo "  - Xorg (PID: $XORG_PID)"
    echo ""
    echo -e "${YELLOW}可能需要停止显示管理器才能测试${NC}"
    echo "运行: sudo systemctl stop gdm"
    echo ""
fi

echo -e "${BLUE}开始测试...${NC}"
echo ""

# 测试 1: 基本显示
echo -e "${CYAN}测试 1: 基本 HUD 显示${NC}"
echo "将在左上角显示 'Hello LinuxHUD!' 文本"
echo "按 Ctrl+C 停止"
echo ""
read -p "按 Enter 开始测试..." 

./build/bin/linuxhud -t "Hello LinuxHUD!" \
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
echo -e "${GREEN}测试完成${NC}"
