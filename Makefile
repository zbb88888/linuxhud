# LinuxHUD Makefile
# 硬件级平视显示器

# ============================================================================
# 项目配置
# ============================================================================

PROJECT_NAME := linuxhud
VERSION := 0.2.0

# ============================================================================
# 编译器和工具
# ============================================================================

CC := gcc
AR := ar
INSTALL := install
MKDIR := mkdir -p
RM := rm -f
RMDIR := rm -rf

# ============================================================================
# 目录定义
# ============================================================================

SRCDIR := src
INCDIR := include
BUILDDIR := build
OBJDIR := $(BUILDDIR)/obj
LIBDIR := $(BUILDDIR)/lib
BINDIR := $(BUILDDIR)/bin
TESTDIR := tests

# ============================================================================
# 编译选项
# ============================================================================

CFLAGS := -Wall -Wextra -Werror -Wpedantic
CFLAGS += -std=c11
CFLAGS += -D_GNU_SOURCE
CFLAGS += -DPROJECT_VERSION=\"$(VERSION)\"
CFLAGS += -I$(INCDIR)

ifdef DEBUG
    CFLAGS += -g -O0 -DDEBUG
else
    CFLAGS += -O2 -DNDEBUG
endif

CFLAGS += $(shell pkg-config --cflags libdrm cairo pangocairo 2>/dev/null)
LDFLAGS := $(shell pkg-config --libs libdrm cairo pangocairo 2>/dev/null)

# ============================================================================
# 源文件（按模块组织）
# ============================================================================

SRCS := $(SRCDIR)/core/types.c \
        $(SRCDIR)/core/log.c \
        $(SRCDIR)/core/hud.c \
        $(SRCDIR)/drm/device.c \
        $(SRCDIR)/render/renderer.c \
        $(SRCDIR)/widget/widget.c \
        $(SRCDIR)/main.c

OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

TARGET := $(BINDIR)/$(PROJECT_NAME)

# 测试程序 (链接库对象)
TEST_SRCS := $(wildcard $(TESTDIR)/*.c)
TEST_BINS := $(patsubst $(TESTDIR)/%.c,$(BINDIR)/%,$(TEST_SRCS))
TEST_OBJS := $(filter-out $(OBJDIR)/main.o, $(OBJS))

# ============================================================================
# 安装路径
# ============================================================================

PREFIX := /usr/local
BINDIR_INSTALL := $(PREFIX)/bin
CONFDIR_INSTALL := /etc
SERVICEDIR_INSTALL := /etc/systemd/system

# ============================================================================
# 默认目标
# ============================================================================

.PHONY: all
all: help

.PHONY: build
build: dirs $(TARGET) $(TEST_BINS)
	@echo ""
	@echo "=========================================="
	@echo "  $(PROJECT_NAME) v$(VERSION) 构建完成"
	@echo "=========================================="
	@echo "  可执行文件: $(TARGET)"
	@echo "=========================================="

# ============================================================================
# 目录创建
# ============================================================================

.PHONY: dirs
dirs:
	@$(MKDIR) $(OBJDIR)/core $(OBJDIR)/drm $(OBJDIR)/render $(OBJDIR)/widget
	@$(MKDIR) $(LIBDIR) $(BINDIR) $(TESTDIR)

# ============================================================================
# 主程序构建
# ============================================================================

$(TARGET): $(OBJS)
	@echo "  LINK    $@"
	@$(MKDIR) $(dir $@)
	@$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "  CC      $<"
	@$(MKDIR) $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

# ============================================================================
# 测试程序构建
# ============================================================================

$(BINDIR)/test_%: $(TESTDIR)/test_%.c $(TEST_OBJS)
	@echo "  TEST    $@"
	@$(MKDIR) $(dir $@)
	@$(CC) $(CFLAGS) $< $(TEST_OBJS) -o $@ $(LDFLAGS)

# ============================================================================
# 运行目标
# ============================================================================

.PHONY: run
run: $(TARGET)
	@./$(TARGET) -t "Hello LinuxHUD" -v

.PHONY: run-sudo
run-sudo: $(TARGET)
	@sudo ./$(TARGET) -t "Hello LinuxHUD" -v

.PHONY: demo
demo: $(TARGET)
	@sudo ./run_demo.sh

# ============================================================================
# 测试目标
# ============================================================================

.PHONY: test
test: dirs $(TEST_BINS)
	@echo ""
	@echo "=========================================="
	@echo "  运行测试"
	@echo "=========================================="
	@for test_bin in $(TEST_BINS); do \
		echo "运行: $$test_bin"; \
		if ./$$test_bin; then \
			echo "  ✓ 通过"; \
		else \
			echo "  ✗ 失败"; \
			exit 1; \
		fi; \
		echo ""; \
	done
	@echo "=========================================="
	@echo "  所有测试通过"
	@echo "=========================================="

.PHONY: test-drm
test-drm: $(BINDIR)/test_drm
	@echo "运行 DRM 测试..."
	@sudo ./$(BINDIR)/test_drm

.PHONY: test-quick
test-quick: dirs $(TEST_BINS)
	@echo "快速测试..."
	@for test_bin in $(TEST_BINS); do \
		./$$test_bin > /dev/null 2>&1 && echo "  ✓ $$test_bin" || echo "  ✗ $$test_bin"; \
	done

.PHONY: test-all
test-all: build test test-scripts
	@echo ""
	@echo "=========================================="
	@echo "  所有测试完成"
	@echo "=========================================="

.PHONY: test-scripts
test-scripts:
	@echo "运行测试脚本..."
	@chmod +x test_project.sh test_makefile.sh 2>/dev/null || true
	@echo ""
	@echo "--- 项目结构测试 ---"
	@./test_project.sh
	@echo ""
	@echo "--- 构建系统测试 ---"
	@./test_makefile.sh

.PHONY: test-project
test-project:
	@echo "运行项目结构测试..."
	@chmod +x test_project.sh 2>/dev/null || true
	@./test_project.sh

.PHONY: test-build-system
test-build-system:
	@echo "运行构建系统测试..."
	@chmod +x test_makefile.sh 2>/dev/null || true
	@./test_makefile.sh

.PHONY: test-integration
test-integration: build
	@echo "运行集成测试..."
	@echo ""
	@echo "--- DRM 设备测试 ---"
	@sudo ./$(BINDIR)/test_drm
	@echo ""
	@echo "--- 帮助信息测试 ---"
	@./$(TARGET) --help > /dev/null && echo "  ✓ 帮助信息"
	@echo ""
	@echo "--- 参数解析测试 ---"
	@./$(TARGET) --help 2>&1 | grep -q "用法:" && echo "  ✓ 参数解析"
	@echo ""
	@echo "集成测试完成"

.PHONY: test-e2e
test-e2e: build
	@echo "运行 E2E 测试..."
	@chmod +x test_e2e.sh 2>/dev/null || true
	@./test_e2e.sh

.PHONY: test-display
test-display: build
	@echo "运行显示器测试..."
	@echo ""
	@echo "--- 检测显示器 ---"
	@sudo ./$(BINDIR)/test_drm 2>&1 | grep -E "(CONNECTED|Preferred)"
	@echo ""
	@echo "--- 显示 HUD (5秒) ---"
	@echo "请观察屏幕左上角是否显示文本"
	@echo ""
	@timeout 5 sudo ./$(TARGET) -t "Hello LinuxHUD!" -x 50 -y 50 -w 400 -h 100 -a 200 -s 28 -v 2>&1 || true
	@echo ""
	@echo "显示器测试完成"

# ============================================================================
# 代码质量
# ============================================================================

.PHONY: lint
lint:
	@echo "运行代码检查..."
	@if command -v cppcheck > /dev/null; then \
		cppcheck --enable=all --suppress=missingIncludeSystem \
			-I$(INCDIR) $(SRCDIR)/*.c 2>&1; \
	else \
		echo "cppcheck 未安装，跳过静态分析"; \
	fi

.PHONY: format
format:
	@echo "格式化代码..."
	@if command -v clang-format > /dev/null; then \
		find $(SRCDIR) $(INCDIR) -name "*.c" -o -name "*.h" | xargs clang-format -i; \
		echo "代码格式化完成"; \
	else \
		echo "clang-format 未安装"; \
	fi

.PHONY: check-syntax
check-syntax: dirs
	@echo "检查语法..."
	@for src in $(SRCS); do \
		echo "检查: $$src"; \
		$(CC) $(CFLAGS) -fsyntax-only $$src; \
	done
	@echo "语法检查完成"

# ============================================================================
# 清理目标
# ============================================================================

.PHONY: clean
clean:
	@echo "清理构建文件..."
	@$(RMDIR) $(BUILDDIR)
	@echo "清理完成"

.PHONY: clean-all
clean-all: clean
	@echo "深度清理..."
	@$(RM) $(PROJECT_NAME)
	@$(RM) *.log
	@$(RM) core.*
	@echo "深度清理完成"

.PHONY: distclean
distclean: clean-all
	@echo "完全清理..."
	@$(RM) .cache
	@$(RM) compile_commands.json
	@echo "完全清理完成"

# ============================================================================
# 安装目标
# ============================================================================

.PHONY: install
install: $(TARGET)
	@echo "安装 $(PROJECT_NAME) v$(VERSION)..."
	@$(INSTALL) -d $(BINDIR_INSTALL)
	@$(INSTALL) -m 755 $(TARGET) $(BINDIR_INSTALL)/$(PROJECT_NAME)
	@$(INSTALL) -d $(CONFDIR_INSTALL)
	@$(INSTALL) -m 644 $(PROJECT_NAME).conf $(CONFDIR_INSTALL)/
	@$(INSTALL) -d $(SERVICEDIR_INSTALL)
	@$(INSTALL) -m 644 $(PROJECT_NAME).service $(SERVICEDIR_INSTALL)/
	@systemctl daemon-reload
	@echo ""
	@echo "=========================================="
	@echo "  安装完成"
	@echo "=========================================="
	@echo "  可执行文件: $(BINDIR_INSTALL)/$(PROJECT_NAME)"
	@echo "  配置文件:   $(CONFDIR_INSTALL)/$(PROJECT_NAME).conf"
	@echo "  服务文件:   $(SERVICEDIR_INSTALL)/$(PROJECT_NAME).service"
	@echo ""
	@echo "  启用服务: sudo systemctl enable $(PROJECT_NAME)"
	@echo "  启动服务: sudo systemctl start $(PROJECT_NAME)"
	@echo "=========================================="

.PHONY: uninstall
uninstall:
	@echo "卸载 $(PROJECT_NAME)..."
	@$(RM) $(BINDIR_INSTALL)/$(PROJECT_NAME)
	@$(RM) $(CONFDIR_INSTALL)/$(PROJECT_NAME).conf
	@$(RM) $(SERVICEDIR_INSTALL)/$(PROJECT_NAME).service
	@systemctl daemon-reload
	@echo "卸载完成"

# ============================================================================
# 打包目标
# ============================================================================

.PHONY: dist
dist: clean-all
	@echo "创建发布包..."
	@$(MKDIR) dist
	@tar -czf dist/$(PROJECT_NAME)-$(VERSION).tar.gz \
		--exclude='.git' \
		--exclude='dist' \
		--exclude='build' \
		.
	@echo "发布包创建完成: dist/$(PROJECT_NAME)-$(VERSION).tar.gz"

# ============================================================================
# 调试目标
# ============================================================================

.PHONY: debug
debug:
	@$(MAKE) DEBUG=1 all

.PHONY: gdb
gdb: debug
	@echo "启动 GDB..."
	@gdb ./$(TARGET)

.PHONY: valgrind
valgrind: debug
	@echo "运行 Valgrind..."
	@valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) -t "Test"

.PHONY: strace
strace: $(TARGET)
	@echo "运行 strace..."
	@strace -f ./$(TARGET) -t "Test" 2>&1 | head -100

# ============================================================================
# 信息目标
# ============================================================================

.PHONY: info
info:
	@echo ""
	@echo "=========================================="
	@echo "  $(PROJECT_NAME) v$(VERSION) 项目信息"
	@echo "=========================================="
	@echo "  项目名称: $(PROJECT_NAME)"
	@echo "  版本:     $(VERSION)"
	@echo "  编译器:   $(CC)"
	@echo "  编译标志: $(CFLAGS)"
	@echo "  链接标志: $(LDFLAGS)"
	@echo "  源文件:   $(SRCS)"
	@echo "  目标文件: $(OBJS)"
	@echo "  目标:     $(TARGET)"
	@echo "=========================================="
	@echo ""
	@echo "依赖检查:"
	@echo -n "  libdrm:     "; pkg-config --exists libdrm && echo "✓ 已安装" || echo "✗ 未安装"
	@echo -n "  cairo:      "; pkg-config --exists cairo && echo "✓ 已安装" || echo "✗ 未安装"
	@echo -n "  pangocairo: "; pkg-config --exists pangocairo && echo "✓ 已安装" || echo "✗ 未安装"
	@echo ""

.PHONY: check-deps
check-deps:
	@echo "检查依赖..."
	@MISSING=0; \
	if ! pkg-config --exists libdrm; then \
		echo "✗ libdrm 未安装"; \
		MISSING=1; \
	else \
		echo "✓ libdrm 已安装"; \
	fi; \
	if ! pkg-config --exists cairo; then \
		echo "✗ cairo 未安装"; \
		MISSING=1; \
	else \
		echo "✓ cairo 已安装"; \
	fi; \
	if ! pkg-config --exists pangocairo; then \
		echo "✗ pangocairo 未安装"; \
		MISSING=1; \
	else \
		echo "✓ pangocairo 已安装"; \
	fi; \
	if [ $$MISSING -eq 1 ]; then \
		echo ""; \
		echo "安装缺失的依赖:"; \
		echo "  sudo apt-get install libdrm-dev libcairo2-dev libpango1.0-dev"; \
		exit 1; \
	fi
	@echo ""
	@echo "所有依赖已满足"

# ============================================================================
# 帮助目标
# ============================================================================

.PHONY: help
help:
	@echo ""
	@echo "=========================================="
	@echo "  $(PROJECT_NAME) v$(VERSION) 构建系统"
	@echo "=========================================="
	@echo ""
	@echo "基本目标:"
	@echo "  build        构建项目"
	@echo "  clean        清理构建文件"
	@echo "  clean-all    深度清理"
	@echo "  distclean    完全清理"
	@echo ""
	@echo "运行目标:"
	@echo "  run          运行程序"
	@echo "  run-sudo     以 root 权限运行"
	@echo "  demo         运行演示"
	@echo ""
	@echo "测试目标:"
	@echo "  test              运行测试程序"
	@echo "  test-all          运行所有测试（包括脚本）"
	@echo "  test-drm          运行 DRM 测试"
	@echo "  test-quick        快速测试"
	@echo "  test-integration  运行集成测试"
	@echo "  test-e2e          运行端到端测试"
	@echo "  test-display      运行显示器测试"
	@echo ""
	@echo "代码质量:"
	@echo "  lint         代码检查"
	@echo "  format       格式化代码"
	@echo "  check-syntax 检查语法"
	@echo ""
	@echo "安装目标:"
	@echo "  install      安装到系统"
	@echo "  uninstall    从系统卸载"
	@echo ""
	@echo "调试目标:"
	@echo "  debug        调试构建"
	@echo "  gdb          启动 GDB"
	@echo "  valgrind     运行 Valgrind"
	@echo "  strace       运行 strace"
	@echo ""
	@echo "信息目标:"
	@echo "  info         显示项目信息"
	@echo "  check-deps   检查依赖"
	@echo "  help         显示帮助"
	@echo ""
	@echo "示例:"
	@echo "  make                    # 显示帮助"
	@echo "  make build              # 构建项目"
	@echo "  make test               # 运行测试"
	@echo "  make run-sudo           # 以 root 运行"
	@echo "  make install            # 安装到系统"
	@echo ""
