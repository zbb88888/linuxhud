#!/bin/bash

# LinuxHUD 显示测试脚本
# 安全地测试 HUD 是否可以在显示器上直接显示

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}   LinuxHUD 显示测试${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# 检查 root 权限
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}错误：需要 root 权限运行此脚本${NC}"
    echo "请使用: sudo $0"
    exit 1
fi

# 步骤 1: 检查 DRM 设备
echo -e "${CYAN}[步骤 1] 检查 DRM 设备...${NC}"
if [ ! -d "/dev/dri" ]; then
    echo -e "${RED}✗ 未找到 DRM 设备${NC}"
    exit 1
fi
echo -e "${GREEN}✓ DRM 设备存在${NC}"

# 检查是否有已连接的显示器
CONNECTOR_STATUS=$(./build/bin/test_drm 2>&1 | grep "CONNECTED" | head -1)
if [ -z "$CONNECTOR_STATUS" ]; then
    echo -e "${RED}✗ 未检测到已连接的显示器${NC}"
    exit 1
fi
echo -e "${GREEN}✓ 检测到显示器${NC}"
echo "  $CONNECTOR_STATUS"

# 步骤 2: 检查 DRM Master 状态
echo ""
echo -e "${CYAN}[步骤 2] 检查 DRM Master 状态...${NC}"
GDM_PID=$(pgrep -x gdm3 || pgrep -x gdm || echo "")
XORG_PID=$(pgrep -x Xorg || pgrep -x X || echo "")

if [ -n "$GDM_PID" ] || [ -n "$XORG_PID" ]; then
    echo -e "${YELLOW}⚠ 检测到显示管理器正在运行:${NC}"
    [ -n "$GDM_PID" ] && echo "  - GDM (PID: $GDM_PID)"
    [ -n "$XORG_PID" ] && echo "  - Xorg (PID: $XORG_PID)"
    echo ""
    echo -e "${YELLOW}显示管理器持有 DRM Master 权限，需要释放才能测试${NC}"
    echo ""
    read -p "是否临时停止 GDM 进行测试? (y/N): " STOP_GDM
    if [ "$STOP_GDM" != "y" ] && [ "$STOP_GDM" != "Y" ]; then
        echo -e "${YELLOW}跳过显示测试${NC}"
        echo ""
        echo -e "${BLUE}手动测试方法:${NC}"
        echo "1. 切换到虚拟终端 (Ctrl+Alt+F2)"
        echo "2. 登录并运行:"
        echo "   cd $(pwd)"
        echo "   sudo ./build/bin/linuxhud -t 'Hello LinuxHUD' -v"
        echo "3. 按 Ctrl+C 停止"
        echo "4. 切换回图形界面 (Ctrl+Alt+F7 或 Ctrl+Alt+F1)"
        exit 0
    fi
else
    echo -e "${GREEN}✓ 没有显示管理器占用 DRM${NC}"
fi

# 步骤 3: 停止 GDM (如果需要)
if [ -n "$GDM_PID" ] || [ -n "$XORG_PID" ]; then
    echo ""
    echo -e "${CYAN}[步骤 3] 临时停止 GDM...${NC}"
    echo -e "${YELLOW}警告：屏幕将变为文本模式${NC}"
    echo ""
    
    # 备份当前状态
    GDM_WAS_ACTIVE=1
    
    systemctl stop gdm 2>/dev/null || true
    sleep 2
    
    # 验证 GDM 已停止
    if pgrep -x gdm3 > /dev/null || pgrep -x Xorg > /dev/null; then
        echo -e "${RED}✗ 无法停止 GDM${NC}"
        echo "请手动停止: sudo systemctl stop gdm"
        exit 1
    fi
    echo -e "${GREEN}✓ GDM 已停止${NC}"
else
    GDM_WAS_ACTIVE=0
fi

# 步骤 4: 测试 DRM 显示
echo ""
echo -e "${CYAN}[步骤 4] 测试 DRM 显示...${NC}"
echo "将在屏幕上显示红色矩形 5 秒钟"
echo ""

# 编译测试程序
if [ ! -f "test_plane" ]; then
    echo "编译测试程序..."
    gcc -o test_plane test_plane.c -I/usr/include/libdrm -ldrm 2>/dev/null || {
        echo -e "${RED}✗ 编译测试程序失败${NC}"
        # 尝试使用已编译的 linuxhud
        if [ -f "build/bin/linuxhud" ]; then
            echo "使用 linuxhud 程序测试..."
            timeout 5 ./build/bin/linuxhud -t "Hello LinuxHUD!" -x 100 -y 100 -w 400 -h 150 -a 200 -s 28 -v 2>&1
        fi
    }
fi

if [ -f "test_plane" ]; then
    echo "运行 DRM 平面测试..."
    ./test_plane 2>&1
fi

# 步骤 5: 运行 linuxhud 测试
echo ""
echo -e "${CYAN}[步骤 5] 运行 LinuxHUD 测试...${NC}"
echo "按 Ctrl+C 可提前停止"
echo ""

if [ -f "build/bin/linuxhud" ]; then
    # 运行 5 秒后自动停止
    timeout 5 ./build/bin/linuxhud -t "Hello LinuxHUD!" -x 100 -y 100 -w 400 -h 150 -a 200 -s 28 -c "00FF00" -b "000000" -v 2>&1 || true
fi

# 步骤 6: 恢复 GDM
echo ""
if [ "$GDM_WAS_ACTIVE" -eq 1 ]; then
    echo -e "${CYAN}[步骤 6] 恢复 GDM...${NC}"
    systemctl start gdm 2>/dev/null || true
    sleep 2
    echo -e "${GREEN}✓ GDM 已恢复${NC}"
fi

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}   测试完成${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo -e "${GREEN}如果屏幕上显示了文本或图形，说明 LinuxHUD 可以在显示器上直接显示。${NC}"
echo ""
echo -e "${BLUE}其他测试方法:${NC}"
echo "1. 切换到虚拟终端 (Ctrl+Alt+F2)"
echo "2. 运行: sudo ./build/bin/linuxhud -t 'Hello' -v"
echo "3. 切换回图形界面 (Ctrl+Alt+F7)"
