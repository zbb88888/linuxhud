# LinuxHUD 测试指南

## 快速开始

```bash
# 运行所有测试
make test-all

# 运行特定测试
make test           # 运行测试程序
make test-project   # 运行项目结构测试
make test-build     # 运行构建系统测试
make test-integration  # 运行集成测试
```

## 测试目标列表

| 目标 | 说明 | 命令 |
|------|------|------|
| `test` | 运行测试程序 | `make test` |
| `test-all` | 运行所有测试（包括脚本） | `make test-all` |
| `test-drm` | 运行 DRM 测试 | `sudo make test-drm` |
| `test-quick` | 快速测试 | `make test-quick` |
| `test-project` | 运行项目结构测试 | `make test-project` |
| `test-build-system` | 运行构建系统测试 | `make test-build-system` |
| `test-integration` | 运行集成测试 | `sudo make test-integration` |

## 测试详情

### 1. test - 运行测试程序

运行编译后的测试程序，验证基本功能。

```bash
make test
```

**预期输出**：
```
==========================================
  运行测试
==========================================
运行: build/bin/test_drm
DRM device opened successfully
...
DRM test completed successfully
  ✓ 通过

==========================================
  所有测试通过
==========================================
```

### 2. test-all - 运行所有测试

运行所有测试，包括测试程序和测试脚本。

```bash
make test-all
```

**预期输出**：
```
==========================================
  运行测试
==========================================
运行: build/bin/test_drm
...
  ✓ 通过

==========================================
  所有测试通过
==========================================
运行测试脚本...

--- 项目结构测试 ---
...
=== 测试结果 ===
通过: 23
失败: 0
总计: 23

所有测试通过！

--- 构建系统测试 ---
...
=== 测试结果 ===
通过: 16
失败: 0
跳过: 1
总计: 17

✓ 所有测试通过！

==========================================
  所有测试完成
==========================================
```

### 3. test-drm - 运行 DRM 测试

运行 DRM 硬件测试，需要 root 权限。

```bash
sudo make test-drm
```

**预期输出**：
```
运行 DRM 测试...
DRM device opened successfully
Connectors: 5
CRTCs: 4
Encoders: 8
Framebuffers: 0
Universal planes capability enabled
Planes: 12
Plane 0: ID=51, CRTC_ID=0, FB_ID=0
  Type: PRIMARY (1)
...
Connector 1: ID=132, Status=CONNECTED
  Modes: 25
  Preferred mode: 1920x1080
...
DRM test completed successfully
```

### 4. test-quick - 快速测试

快速运行测试程序，不显示详细输出。

```bash
make test-quick
```

**预期输出**：
```
快速测试...
  ✓ build/bin/test_drm
```

### 5. test-project - 运行项目结构测试

运行项目结构测试脚本，验证文件和依赖。

```bash
make test-project
```

**预期输出**：
```
运行项目结构测试...
=== LinuxHUD 项目测试 ===

1. 检查项目结构
测试 源代码目录: 通过
测试 头文件目录: 通过
测试 主程序文件: 通过
测试 DRM设备模块: 通过
测试 缓冲区模块: 通过
测试 渲染器模块: 通过
测试 头文件: 通过
测试 Makefile: 通过
测试 Meson构建文件: 通过

2. 检查依赖
测试 GCC编译器: 通过
测试 libdrm开发包: 通过
测试 cairo开发包: 通过
测试 pango开发包: 通过

3. 编译测试
测试 清理构建: 通过
测试 编译项目: 通过
测试 编译测试程序: 通过

4. 功能测试
测试 帮助信息: 通过
测试 DRM测试程序: 通过

5. 检查生成的文件
测试 可执行文件: 通过
测试 测试可执行文件: 通过

6. 检查配置文件
测试 配置文件: 通过
测试 服务文件: 通过
测试 演示脚本: 通过

=== 测试结果 ===
通过: 23
失败: 0
总计: 23

所有测试通过！
```

### 6. test-build-system - 运行构建系统测试

运行构建系统测试脚本，验证 Makefile 功能。

```bash
make test-build-system
```

**预期输出**：
```
运行构建系统测试...
=== LinuxHUD Makefile 测试 ===

1. 基本构建测试
测试 清理构建: 通过
测试 构建项目: 通过
测试 检查可执行文件: 通过
测试 检查测试程序: 通过

2. 信息目标测试
测试 项目信息: 通过
测试 版本信息: 通过
测试 依赖检查: 通过

3. 代码质量测试
测试 语法检查: 通过
测试 代码检查: 跳过 (cppcheck 未安装)

4. 测试目标
测试 快速测试: 通过

5. 调试构建测试
测试 调试构建: 通过

6. 打包测试
测试 创建发布包: 通过
测试 创建安装包: 通过

7. 帮助测试
测试 帮助信息: 通过

8. 清理测试
测试 清理构建: 通过
测试 深度清理: 通过
测试 完全清理: 通过

==========================================
测试结果
==========================================
通过: 16
失败: 0
跳过: 1
总计: 17

✓ 所有测试通过！
```

### 7. test-integration - 运行集成测试

运行集成测试，验证 DRM 设备访问和基本功能。

```bash
sudo make test-integration
```

**预期输出**：
```
运行集成测试...

--- DRM 设备测试 ---
DRM device opened successfully
Connectors: 5
CRTCs: 4
...
DRM test completed successfully

--- 帮助信息测试 ---
  ✓ 帮助信息

--- 参数解析测试 ---
  ✓ 参数解析

集成测试完成
```

## 测试覆盖范围

### 自动测试

| 测试类型 | 测试文件 | 测试数量 | 通过 | 失败 | 跳过 |
|----------|----------|----------|------|------|------|
| 测试程序 | tests/test_drm.c | 1 | 1 | 0 | 0 |
| 项目结构 | test_project.sh | 23 | 23 | 0 | 0 |
| 构建系统 | test_makefile.sh | 17 | 16 | 0 | 1 |
| **总计** | - | **41** | **40** | **0** | **1** |

### 测试内容

| 类别 | 测试项 | 说明 |
|------|--------|------|
| **DRM 测试** | 设备访问 | 打开 /dev/dri/card1 |
| | 资源发现 | 连接器、CRTC、平面 |
| | 平面类型 | PRIMARY、CURSOR、OVERLAY |
| | 显示器检测 | 已连接显示器 |
| **项目测试** | 文件结构 | src/、include/、tests/ |
| | 依赖检查 | gcc、libdrm、cairo、pango |
| | 编译测试 | make clean、make build |
| | 功能测试 | 帮助信息、DRM 测试 |
| | 配置文件 | linuxhud.conf、linuxhud.service |
| **构建测试** | 基本构建 | make clean、make build |
| | 信息目标 | make info、make version |
| | 代码质量 | make check-syntax |
| | 测试目标 | make test-quick |
| | 调试构建 | make debug |
| | 打包目标 | make dist、make package |
| | 清理目标 | make clean、make clean-all |
| **集成测试** | DRM 设备 | 硬件访问和资源发现 |
| | 帮助信息 | 命令行参数解析 |
| | 参数验证 | 无效参数处理 |

## 手动测试

### 基本功能测试

```bash
# 1. 显示帮助信息
./build/bin/linuxhud --help

# 2. DRM 设备测试（需要 root）
sudo ./build/bin/test_drm

# 3. HUD 显示测试（需要 root）
sudo ./build/bin/linuxhud -t "Hello World" -v

# 4. 参数测试
sudo ./build/bin/linuxhud -t "Test" -x 100 -y 100 -w 400 -h 150 -a 180 -s 28 -c "00FF00" -b "000000"
```

### 错误处理测试

```bash
# 1. 权限错误
./build/bin/linuxhud -t "Test" 2>&1

# 2. 无效参数
./build/bin/linuxhud --invalid 2>&1

# 3. 缺少参数
./build/bin/linuxhud -t 2>&1
```

### 性能测试

```bash
# 1. 高刷新率测试
sudo ./build/bin/linuxhud -t "Performance" -v 2>&1 | head -100

# 2. 内存使用测试
sudo valgrind ./build/bin/linuxhud -t "Memory Test"
```

## 测试结果判定

### 成功标准

| 测试类型 | 成功条件 |
|----------|----------|
| DRM 测试 | 成功打开设备，发现资源，检测到平面 |
| 项目测试 | 23/23 测试通过 |
| 构建测试 | 16/16 测试通过，1 个跳过 |
| 集成测试 | 所有验证点通过 |

### 失败条件

| 测试类型 | 失败条件 |
|----------|----------|
| DRM 测试 | 无法打开设备，资源获取失败 |
| 项目测试 | 任何文件缺失或依赖未安装 |
| 构建测试 | 编译失败，链接错误 |
| 集成测试 | 任何验证点失败 |

## 常见问题

### Q: 测试失败怎么办？

```bash
# 1. 检查依赖
make check-deps

# 2. 清理并重新构建
make clean
make build

# 3. 查看详细错误
make test 2>&1
```

### Q: 如何跳过某个测试？

在测试脚本中，可以通过修改跳过条件来跳过特定测试。

### Q: 如何添加新的测试？

1. 在 `tests/` 目录创建新的测试文件
2. 在 Makefile 中添加新的测试目标
3. 更新测试脚本

## 测试工具

### 自动化测试

- `test_project.sh` - 项目结构测试
- `test_makefile.sh` - 构建系统测试

### 手动测试

- `build/bin/test_drm` - DRM 硬件测试
- `build/bin/linuxhud` - 主程序测试

### 辅助工具

- `make test-quick` - 快速测试
- `make check-syntax` - 语法检查
- `make info` - 项目信息

## 总结

### 测试矩阵

| 测试类型 | 测试数量 | 通过 | 失败 | 跳过 |
|----------|----------|------|------|------|
| DRM 测试 | 1 | 1 | 0 | 0 |
| 项目测试 | 23 | 23 | 0 | 0 |
| 构建测试 | 17 | 16 | 0 | 1 |
| 集成测试 | 3 | 3 | 0 | 0 |
| **总计** | **44** | **43** | **0** | **1** |

### 测试状态

```
✓ DRM 硬件测试 - 通过
✓ 项目结构测试 - 通过
✓ 构建系统测试 - 通过
✓ 集成测试 - 通过
✓ 基本功能测试 - 待手动验证
✓ 参数测试 - 待手动验证
✓ 错误处理测试 - 待手动验证
```

### 快速命令

```bash
# 运行所有测试
make test-all

# 运行特定测试
make test           # 测试程序
make test-project   # 项目结构
make test-build     # 构建系统
make test-integration  # 集成测试

# 快速测试
make test-quick

# 手动测试
sudo ./build/bin/linuxhud -t "Hello World"
```
