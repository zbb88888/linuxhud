# LinuxHUD Makefile 使用指南

## 概述

LinuxHUD 使用 Makefile 管理项目的构建、测试、清理和维护。本指南介绍如何使用 Makefile 的各种功能。

## 快速开始

### 1. 构建项目

```bash
# 构建项目（默认目标）
make

# 或者显式指定
make all
```

### 2. 运行程序

```bash
# 以普通用户运行
make run

# 以 root 权限运行（需要 sudo）
make run-sudo

# 运行演示
make demo
```

### 3. 运行测试

```bash
# 运行所有测试
make test

# 快速测试
make test-quick

# 测试 DRM
make test-drm
```

### 4. 清理构建

```bash
# 清理构建文件
make clean

# 深度清理
make clean-all

# 完全清理
make distclean
```

## 完整目标列表

### 基本目标

| 目标 | 说明 | 示例 |
|------|------|------|
| `all` | 构建项目（默认） | `make` 或 `make all` |
| `clean` | 清理构建文件 | `make clean` |
| `clean-all` | 深度清理 | `make clean-all` |
| `distclean` | 完全清理 | `make distclean` |

### 运行目标

| 目标 | 说明 | 示例 |
|------|------|------|
| `run` | 运行程序 | `make run` |
| `run-sudo` | 以 root 权限运行 | `make run-sudo` |
| `demo` | 运行演示 | `make demo` |

### 测试目标

| 目标 | 说明 | 示例 |
|------|------|------|
| `test` | 运行所有测试 | `make test` |
| `test-drm` | 运行 DRM 测试 | `make test-drm` |
| `test-quick` | 快速测试 | `make test-quick` |

### 代码质量

| 目标 | 说明 | 示例 |
|------|------|------|
| `lint` | 代码检查 | `make lint` |
| `format` | 格式化代码 | `make format` |
| `check-syntax` | 检查语法 | `make check-syntax` |

### 安装目标

| 目标 | 说明 | 示例 |
|------|------|------|
| `install` | 安装到系统 | `sudo make install` |
| `uninstall` | 从系统卸载 | `sudo make uninstall` |
| `install-dev` | 安装开发文件 | `sudo make install-dev` |

### 打包目标

| 目标 | 说明 | 示例 |
|------|------|------|
| `dist` | 创建发布包 | `make dist` |
| `package` | 创建安装包 | `make package` |

### 调试目标

| 目标 | 说明 | 示例 |
|------|------|------|
| `debug` | 调试构建 | `make debug` |
| `gdb` | 启动 GDB | `make gdb` |
| `valgrind` | 运行 Valgrind | `make valgrind` |
| `strace` | 运行 strace | `make strace` |

### 文档目标

| 目标 | 说明 | 示例 |
|------|------|------|
| `docs` | 生成文档 | `make docs` |
| `man` | 生成手册页 | `make man` |

### 信息目标

| 目标 | 说明 | 示例 |
|------|------|------|
| `info` | 显示项目信息 | `make info` |
| `version` | 显示版本 | `make version` |
| `check-deps` | 检查依赖 | `make check-deps` |
| `help` | 显示帮助 | `make help` |

## 构建配置

### 调试构建

```bash
# 启用调试模式
make DEBUG=1

# 或者使用 debug 目标
make debug
```

调试构建会：
- 启用调试符号 (`-g`)
- 禁用优化 (`-O0`)
- 定义 `DEBUG` 宏

### 优化构建

```bash
# 默认优化构建
make

# 或者显式指定
make OPTIMIZE=2
```

优化构建会：
- 启用优化 (`-O2`)
- 定义 `NDEBUG` 宏

## 项目结构

```
linuxhud/
├── src/                    # 源代码
│   ├── main.c              # 主程序
│   ├── drm_device.c        # DRM 设备操作
│   ├── buffer.c            # 缓冲区管理
│   └── renderer.c          # 渲染引擎
├── include/                # 头文件
│   └── linuxhud.h          # 公共接口
├── tests/                  # 测试程序
│   └── test_drm.c          # DRM 测试
├── build/                  # 构建目录（自动生成）
│   ├── obj/                # 目标文件
│   ├── lib/                # 库文件
│   └── bin/                # 可执行文件
├── dist/                   # 发布包（自动生成）
├── package/                # 安装包（自动生成）
├── Makefile                # 构建脚本
├── .gitignore              # Git 忽略文件
└── linuxhud.conf           # 配置文件
```

## 依赖管理

### 检查依赖

```bash
make check-deps
```

输出示例：
```
检查依赖...
✓ libdrm 已安装
✓ cairo 已安装
✓ pangocairo 已安装

所有依赖已满足
```

### 安装依赖

```bash
# Ubuntu/Debian
sudo apt-get install libdrm-dev libcairo2-dev libpango1.0-dev

# CentOS/RHEL
sudo yum install libdrm-devel cairo-devel pango-devel

# Fedora
sudo dnf install libdrm-devel cairo-devel pango-devel
```

## 安装和部署

### 安装到系统

```bash
# 安装
sudo make install

# 启用服务
sudo systemctl enable linuxhud

# 启动服务
sudo systemctl start linuxhud

# 查看状态
sudo systemctl status linuxhud
```

### 卸载

```bash
sudo make uninstall
```

## 调试和诊断

### 使用 GDB 调试

```bash
# 启动 GDB
make gdb

# 在 GDB 中
(gdb) break main
(gdb) run -t "Test"
(gdb) next
(gdb) print variable
```

### 使用 Valgrind 检查内存

```bash
make valgrind
```

输出示例：
```
==12345== Memcheck, a memory error detector
==12345== HEAP SUMMARY:
==12345==     in use at exit: 0 bytes in 0 blocks
==12345==   total heap usage: 1,234 allocs, 1,234 frees, 123,456 bytes allocated
==12345== All heap blocks were freed -- no leaks are possible
```

### 使用 strace 跟踪系统调用

```bash
make strace
```

## 打包和分发

### 创建发布包

```bash
make dist
```

生成文件：`dist/linuxhud-0.1.0.tar.gz`

### 创建安装包

```bash
make package
```

生成目录：`package/`，包含所有需要的文件。

## 代码质量

### 静态分析

```bash
# 运行 cppcheck
make lint

# 输出示例
# Checking src/main.c...
# Checking src/drm_device.c...
# ...
```

### 代码格式化

```bash
# 使用 clang-format 格式化
make format
```

### 语法检查

```bash
# 检查所有源文件语法
make check-syntax
```

## 常见问题

### Q: 如何重新构建所有文件？

```bash
make clean
make
```

### Q: 如何只重新构建某个文件？

```bash
# 删除特定目标文件
rm build/obj/main.o

# 重新构建
make
```

### Q: 如何查看详细的构建命令？

```bash
make V=1
```

### Q: 如何并行构建？

```bash
# 使用 4 个并行任务
make -j4

# 使用所有 CPU 核心
make -j$(nproc)
```

### Q: 如何跳过某个测试？

```bash
# 在 test_makefile.sh 中设置跳过原因
run_test "测试名称" "测试命令" "跳过原因"
```

## 最佳实践

1. **定期清理**：使用 `make clean` 保持构建目录整洁
2. **运行测试**：在提交代码前运行 `make test`
3. **检查依赖**：在新环境中运行 `make check-deps`
4. **使用调试构建**：开发时使用 `make DEBUG=1`
5. **格式化代码**：使用 `make format` 保持代码风格一致

## 扩展 Makefile

### 添加新的源文件

1. 在 `src/` 目录创建新文件
2. Makefile 会自动检测新的 `.c` 文件

### 添加新的测试

1. 在 `tests/` 目录创建新文件（如 `test_new.c`）
2. Makefile 会自动检测并构建

### 添加新的目标

在 Makefile 中添加：

```makefile
.PHONY: new-target
new-target: dependencies
	@echo "执行新目标"
	@command
```

## 示例工作流

### 开发工作流

```bash
# 1. 检查依赖
make check-deps

# 2. 清理旧构建
make clean

# 3. 调试构建
make DEBUG=1

# 4. 运行测试
make test

# 5. 运行程序
make run-sudo

# 6. 格式化代码
make format

# 7. 最终构建
make

# 8. 安装
sudo make install
```

### 发布工作流

```bash
# 1. 完全清理
make distclean

# 2. 运行所有测试
make test

# 3. 创建发布包
make dist

# 4. 创建安装包
make package

# 5. 测试安装包
cd package && ./linuxhud --help
```

---

更多信息请运行 `make help` 查看完整帮助。
