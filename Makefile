# LinuxHUD Makefile
# 绕过图形栈的硬件级平视显示器

# ============================================================================
# 项目配置
# ============================================================================

PROJECT_NAME := linuxhud
VERSION := 0.1.0

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
DOCDIR := docs

# ============================================================================
# 编译选项
# ============================================================================

# 基础编译标志
CFLAGS := -Wall -Wextra -Werror -Wpedantic
CFLAGS += -std=c11
CFLAGS += -D_GNU_SOURCE
CFLAGS += -DPROJECT_VERSION=\"$(VERSION)\"

# 优化级别
ifdef DEBUG
    CFLAGS += -g -O0 -DDEBUG
else
    CFLAGS += -O2 -DNDEBUG
endif

# 包含路径
CFLAGS += -I$(INCDIR)
CFLAGS += $(shell pkg-config --cflags libdrm cairo pangocairo 2>/dev/null)

# 链接标志
LDFLAGS := $(shell pkg-config --libs libdrm cairo pangocairo 2>/dev/null)

# ============================================================================
# 源文件和目标文件
# ============================================================================

# 主程序源文件
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# 主程序目标
TARGET := $(BINDIR)/$(PROJECT_NAME)

# 测试程序
TEST_SRCS := $(wildcard $(TESTDIR)/*.c)
TEST_BINS := $(TEST_SRCS:$(TESTDIR)/%.c=$(BINDIR)/%)

# 库文件（如果需要）
LIB_NAME := lib$(PROJECT_NAME).a
LIB_TARGET := $(LIBDIR)/$(LIB_NAME)

# ============================================================================
# 安装路径
# ============================================================================

PREFIX := /usr/local
BINDIR_INSTALL := $(PREFIX)/bin
LIBDIR_INSTALL := $(PREFIX)/lib
INCDIR_INSTALL := $(PREFIX)/include/$(PROJECT_NAME)
CONFDIR_INSTALL := /etc
SERVICEDIR_INSTALL := /etc/systemd/system
MANDIR_INSTALL := $(PREFIX)/share/man/man1

# ============================================================================
# 默认目标
# ============================================================================

# 默认显示帮助信息
.PHONY: all
all: help

# 构建目标
.PHONY: build
build: dirs $(TARGET) $(TEST_BINS)
	@echo ""
	@echo "=========================================="
	@echo "  $(PROJECT_NAME) v$(VERSION) 构建完成"
	@echo "=========================================="
	@echo "  可执行文件: $(TARGET)"
	@echo "  测试程序:   $(TEST_BINS)"
	@echo "=========================================="

# ============================================================================
# 目录创建
# ============================================================================

.PHONY: dirs
dirs:
	@$(MKDIR) $(OBJDIR)
	@$(MKDIR) $(LIBDIR)
	@$(MKDIR) $(BINDIR)
	@$(MKDIR) $(TESTDIR)

# ============================================================================
# 主程序构建
# ============================================================================

$(TARGET): $(OBJS)
	@echo "  LINK    $@"
	@$(MKDIR) $(dir $@)
	@$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# ============================================================================
# 测试程序构建
# ============================================================================

$(BINDIR)/%: $(TESTDIR)/%.c
	@echo "  TEST    $@"
	@$(MKDIR) $(dir $@)
	@$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# ============================================================================
# 库文件构建（可选）
# ============================================================================

.PHONY: lib
lib: dirs $(LIB_TARGET)
	@echo "  库文件构建完成: $(LIB_TARGET)"

$(LIB_TARGET): $(filter-out $(OBJDIR)/main.o, $(OBJS))
	@echo "  AR      $@"
	@$(MKDIR) $(dir $@)
	@$(AR) rcs $@ $^

# ============================================================================
# 运行目标
# ============================================================================

.PHONY: run
run: $(TARGET)
	@echo "运行 $(PROJECT_NAME)..."
	@./$(TARGET) -t "Hello LinuxHUD" -v

.PHONY: run-sudo
run-sudo: $(TARGET)
	@echo "以 root 权限运行 $(PROJECT_NAME)..."
	@sudo ./$(TARGET) -t "Hello LinuxHUD" -v

.PHONY: demo
demo: $(TARGET)
	@echo "运行演示..."
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
		clang-format -i $(SRCDIR)/*.c $(INCDIR)/*.h; \
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
	@$(RM) $(SRCDIR)/*.o
	@$(RM) *.o
	@echo "清理完成"

.PHONY: clean-all
clean-all: clean
	@echo "深度清理..."
	@$(RM) $(PROJECT_NAME)
	@$(RM) test_drm
	@$(RM) *.log
	@$(RM) core.*
	@$(RMDIR) *.dSYM
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

.PHONY: install-dev
install-dev: lib
	@echo "安装开发文件..."
	@$(INSTALL) -d $(INCDIR_INSTALL)
	@$(INSTALL) -m 644 $(INCDIR)/*.h $(INCDIR_INSTALL)/
	@$(INSTALL) -d $(LIBDIR_INSTALL)
	@$(INSTALL) -m 644 $(LIB_TARGET) $(LIBDIR_INSTALL)/
	@echo "开发文件安装完成"

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

.PHONY: package
package: build
	@echo "创建安装包..."
	@$(MKDIR) package
	@$(INSTALL) -m 755 $(TARGET) package/
	@$(INSTALL) -m 644 $(PROJECT_NAME).conf package/
	@$(INSTALL) -m 644 $(PROJECT_NAME).service package/
	@$(INSTALL) -m 755 run_demo.sh package/
	@echo "安装包创建完成: package/"

# ============================================================================
# 文档目标
# ============================================================================

.PHONY: docs
docs:
	@echo "生成文档..."
	@if command -v doxygen > /dev/null; then \
		doxygen; \
	else \
		echo "doxygen 未安装，跳过文档生成"; \
	fi

.PHONY: man
man:
	@echo "生成手册页..."
	@$(MKDIR) man
	@echo ".TH $(PROJECT_NAME) 1 \"$(VERSION)\" \"LinuxHUD Manual\"" > man/$(PROJECT_NAME).1
	@echo ".SH NAME" >> man/$(PROJECT_NAME).1
	@echo "$(PROJECT_NAME) \\- 硬件级平视显示器" >> man/$(PROJECT_NAME).1
	@echo ".SH SYNOPSIS" >> man/$(PROJECT_NAME).1
	@echo ".B $(PROJECT_NAME)" >> man/$(PROJECT_NAME).1
	@echo ".RI [ options ]" >> man/$(PROJECT_NAME).1
	@echo "手册页生成完成"

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
	@echo "  测试:     $(TEST_BINS)"
	@echo "=========================================="
	@echo ""
	@echo "依赖检查:"
	@echo -n "  libdrm:     "; pkg-config --exists libdrm && echo "✓ 已安装" || echo "✗ 未安装"
	@echo -n "  cairo:      "; pkg-config --exists cairo && echo "✓ 已安装" || echo "✗ 未安装"
	@echo -n "  pangocairo: "; pkg-config --exists pangocairo && echo "✓ 已安装" || echo "✗ 未安装"
	@echo ""

.PHONY: version
version:
	@echo "$(VERSION)"

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
	@echo "  test         运行所有测试"
	@echo "  test-drm     运行 DRM 测试"
	@echo "  test-quick   快速测试"
	@echo ""
	@echo "代码质量:"
	@echo "  lint         代码检查"
	@echo "  format       格式化代码"
	@echo "  check-syntax 检查语法"
	@echo ""
	@echo "安装目标:"
	@echo "  install      安装到系统"
	@echo "  uninstall    从系统卸载"
	@echo "  install-dev  安装开发文件"
	@echo ""
	@echo "打包目标:"
	@echo "  dist         创建发布包"
	@echo "  package      创建安装包"
	@echo ""
	@echo "调试目标:"
	@echo "  debug        调试构建"
	@echo "  gdb          启动 GDB"
	@echo "  valgrind     运行 Valgrind"
	@echo "  strace       运行 strace"
	@echo ""
	@echo "文档目标:"
	@echo "  docs         生成文档"
	@echo "  man          生成手册页"
	@echo ""
	@echo "信息目标:"
	@echo "  info         显示项目信息"
	@echo "  version      显示版本"
	@echo "  check-deps   检查依赖"
	@echo "  help         显示帮助"
	@echo ""
	@echo "示例:"
	@echo "  make                    # 显示帮助"
	@echo "  make build              # 构建项目"
	@echo "  make DEBUG=1            # 调试构建"
	@echo "  make test               # 运行测试"
	@echo "  make run-sudo           # 以 root 运行"
	@echo "  make install            # 安装到系统"
	@echo ""
