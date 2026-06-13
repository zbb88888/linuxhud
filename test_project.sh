#!/bin/bash

# LinuxHUD 项目测试脚本

echo "=== LinuxHUD 项目测试 ==="
echo ""

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 测试结果统计
TESTS_PASSED=0
TESTS_FAILED=0

# 测试函数
run_test() {
    local test_name=$1
    local test_command=$2
    
    echo -n "测试 $test_name: "
    
    if eval $test_command > /dev/null 2>&1; then
        echo -e "${GREEN}通过${NC}"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}失败${NC}"
        ((TESTS_FAILED++))
    fi
}

# 1. 检查项目结构
echo "1. 检查项目结构"
run_test "源代码目录" "test -d src"
run_test "头文件目录" "test -d include"
run_test "主程序文件" "test -f src/main.c"
run_test "DRM设备模块" "test -f src/drm_device.c"
run_test "缓冲区模块" "test -f src/buffer.c"
run_test "渲染器模块" "test -f src/renderer.c"
run_test "头文件" "test -f include/linuxhud.h"
run_test "Makefile" "test -f Makefile"
run_test "Meson构建文件" "test -f meson.build"
echo ""

# 2. 检查依赖
echo "2. 检查依赖"
run_test "GCC编译器" "which gcc"
run_test "libdrm开发包" "dpkg -l | grep libdrm-dev"
run_test "cairo开发包" "dpkg -l | grep libcairo2-dev"
run_test "pango开发包" "dpkg -l | grep libpango1.0-dev"
echo ""

# 3. 编译测试
echo "3. 编译测试"
run_test "清理构建" "make clean"
run_test "编译项目" "make build"
run_test "编译测试程序" "make test"
echo ""

# 4. 功能测试
echo "4. 功能测试"
run_test "帮助信息" "./build/bin/linuxhud --help"
run_test "DRM测试程序" "./build/bin/test_drm"
echo ""

# 5. 检查生成的文件
echo "5. 检查生成的文件"
run_test "可执行文件" "test -x build/bin/linuxhud"
run_test "测试可执行文件" "test -x build/bin/test_drm"
echo ""

# 6. 检查配置文件
echo "6. 检查配置文件"
run_test "配置文件" "test -f linuxhud.conf"
run_test "服务文件" "test -f linuxhud.service"
run_test "演示脚本" "test -f run_demo.sh"
echo ""

# 显示测试结果
echo "=== 测试结果 ==="
echo "通过: $TESTS_PASSED"
echo "失败: $TESTS_FAILED"
echo "总计: $((TESTS_PASSED + TESTS_FAILED))"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}所有测试通过！${NC}"
    echo ""
    echo "下一步："
    echo "  1. 运行演示: sudo ./run_demo.sh"
    echo "  2. 手动测试: sudo ./build/bin/linuxhud -t 'Hello World'"
    echo "  3. 查看文档: cat README.md"
else
    echo -e "${RED}有测试失败，请检查错误信息。${NC}"
fi
