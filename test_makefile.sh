#!/bin/bash

# LinuxHUD Makefile 测试脚本

echo "=== LinuxHUD Makefile 测试 ==="
echo ""

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 测试结果统计
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_SKIPPED=0

# 测试函数
run_test() {
    local test_name=$1
    local test_command=$2
    local skip_reason=$3
    
    echo -n "测试 $test_name: "
    
    if [ -n "$skip_reason" ]; then
        echo -e "${YELLOW}跳过${NC} ($skip_reason)"
        ((TESTS_SKIPPED++))
        return 0
    fi
    
    if eval $test_command > /dev/null 2>&1; then
        echo -e "${GREEN}通过${NC}"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}失败${NC}"
        ((TESTS_FAILED++))
    fi
}

# 1. 基本构建测试
echo -e "${BLUE}1. 基本构建测试${NC}"
run_test "清理构建" "make clean"
run_test "构建项目" "make build"
run_test "检查可执行文件" "test -x build/bin/linuxhud"
run_test "检查测试程序" "test -x build/bin/test_drm"
echo ""

# 2. 信息目标测试
echo -e "${BLUE}2. 信息目标测试${NC}"
run_test "项目信息" "make info"
run_test "版本信息" "make version"
run_test "依赖检查" "make check-deps"
echo ""

# 3. 代码质量测试
echo -e "${BLUE}3. 代码质量测试${NC}"
run_test "语法检查" "make check-syntax"
run_test "代码检查" "make lint" "cppcheck 未安装"
echo ""

# 4. 测试目标
echo -e "${BLUE}4. 测试目标${NC}"
run_test "快速测试" "make test-quick"
echo ""

# 5. 调试构建测试
echo -e "${BLUE}5. 调试构建测试${NC}"
run_test "调试构建" "make debug"
echo ""

# 6. 打包测试
echo -e "${BLUE}6. 打包测试${NC}"
run_test "创建发布包" "make dist"
run_test "创建安装包" "make package"
echo ""

# 7. 帮助测试
echo -e "${BLUE}7. 帮助测试${NC}"
run_test "帮助信息" "make help"
echo ""

# 8. 清理测试
echo -e "${BLUE}8. 清理测试${NC}"
run_test "清理构建" "make clean"
run_test "深度清理" "make clean-all"
run_test "完全清理" "make distclean"
echo ""

# 显示测试结果
echo "=========================================="
echo "测试结果"
echo "=========================================="
echo -e "通过: ${GREEN}$TESTS_PASSED${NC}"
echo -e "失败: ${RED}$TESTS_FAILED${NC}"
echo -e "跳过: ${YELLOW}$TESTS_SKIPPED${NC}"
echo "总计: $((TESTS_PASSED + TESTS_FAILED + TESTS_SKIPPED))"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ 所有测试通过！${NC}"
    echo ""
    echo "下一步："
    echo "  1. 查看帮助: make"
    echo "  2. 构建项目: make build"
    echo "  3. 运行测试: make test"
    echo "  4. 安装项目: sudo make install"
    exit 0
else
    echo -e "${RED}✗ 有测试失败，请检查错误信息。${NC}"
    exit 1
fi
