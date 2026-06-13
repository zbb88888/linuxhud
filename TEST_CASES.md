# LinuxHUD 测试用例预期

## 测试概览

本项目包含三类测试：
1. **DRM 硬件测试** (`tests/test_drm.c`) - 测试 DRM 设备访问和硬件发现
2. **项目结构测试** (`test_project.sh`) - 测试项目文件和依赖
3. **构建系统测试** (`test_makefile.sh`) - 测试 Makefile 功能

---

## 1. DRM 硬件测试 (tests/test_drm.c)

### 测试目标
验证 DRM 设备访问、资源发现和硬件平面检测。

### 预期输出

```
DRM device opened successfully
Connectors: 5
CRTCs: 4
Encoders: 8
Framebuffers: 0
Universal planes capability enabled
Planes: 12
Plane 0: ID=51, CRTC_ID=0, FB_ID=0
  Type: PRIMARY (1)
Plane 1: ID=56, CRTC_ID=0, FB_ID=0
  Type: CURSOR (2)
Plane 2: ID=63, CRTC_ID=0, FB_ID=0
  Type: OVERLAY (0)
...
Connector 0: ID=128, Status=DISCONNECTED
Connector 1: ID=132, Status=CONNECTED
  Modes: 25
  Preferred mode: 1920x1080
Connector 2: ID=135, Status=DISCONNECTED
...
DRM test completed successfully
```

### 验证点

| 验证项 | 预期值 | 说明 |
|--------|--------|------|
| DRM 设备打开 | 成功 | `/dev/dri/card1` 可访问 |
| 连接器数量 | 5 | 系统有 5 个显示接口 |
| CRTC 数量 | 4 | 系统有 4 个显示控制器 |
| 编码器数量 | 8 | 系统有 8 个编码器 |
| 平面数量 | 12 | 系统有 12 个硬件平面 |
| Overlay 平面 | ≥ 4 | 至少有 4 个叠加平面 |
| 已连接显示器 | ≥ 1 | 至少有 1 个显示器已连接 |
| 分辨率 | 1920x1080 | 主显示器分辨率 |

### 退出码
- `0` - 测试成功
- `1` - 测试失败（设备访问失败、资源获取失败等）

---

## 2. 项目结构测试 (test_project.sh)

### 测试目标
验证项目文件结构、依赖和基本编译功能。

### 预期输出

```
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

### 验证点

| 类别 | 测试项 | 预期 | 说明 |
|------|--------|------|------|
| **项目结构** | src/ 目录 | 存在 | 源代码目录 |
| | include/ 目录 | 存在 | 头文件目录 |
| | src/main.c | 存在 | 主程序文件 |
| | src/drm_device.c | 存在 | DRM 设备模块 |
| | src/buffer.c | 存在 | 缓冲区模块 |
| | src/renderer.c | 存在 | 渲染器模块 |
| | include/linuxhud.h | 存在 | 公共头文件 |
| | Makefile | 存在 | 构建脚本 |
| | meson.build | 存在 | Meson 构建文件 |
| **依赖检查** | gcc | 已安装 | 编译器 |
| | libdrm-dev | 已安装 | DRM 开发包 |
| | libcairo2-dev | 已安装 | Cairo 开发包 |
| | libpango1.0-dev | 已安装 | Pango 开发包 |
| **编译测试** | make clean | 成功 | 清理构建 |
| | make | 成功 | 编译项目 |
| | make test | 成功 | 编译测试程序 |
| **功能测试** | linuxhud --help | 成功 | 帮助信息显示 |
| | test_drm | 成功 | DRM 测试程序运行 |
| **生成文件** | linuxhud | 可执行 | 主程序可执行 |
| | test_drm | 可执行 | 测试程序可执行 |
| **配置文件** | linuxhud.conf | 存在 | 配置文件 |
| | linuxhud.service | 存在 | 服务文件 |
| | run_demo.sh | 存在 | 演示脚本 |

### 退出码
- `0` - 所有测试通过
- `1` - 有测试失败

---

## 3. 构建系统测试 (test_makefile.sh)

### 测试目标
验证 Makefile 的各种目标和功能。

### 预期输出

```
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

### 验证点

| 类别 | 测试项 | 预期 | 说明 |
|------|--------|------|------|
| **基本构建** | make clean | 成功 | 清理构建文件 |
| | make build | 成功 | 构建项目 |
| | build/bin/linuxhud | 可执行 | 主程序生成 |
| | build/bin/test_drm | 可执行 | 测试程序生成 |
| **信息目标** | make info | 成功 | 显示项目信息 |
| | make version | 成功 | 显示版本号 |
| | make check-deps | 成功 | 检查依赖 |
| **代码质量** | make check-syntax | 成功 | 语法检查通过 |
| | make lint | 跳过 | cppcheck 未安装 |
| **测试目标** | make test-quick | 成功 | 快速测试通过 |
| **调试构建** | make debug | 成功 | 调试构建成功 |
| **打包目标** | make dist | 成功 | 创建发布包 |
| | make package | 成功 | 创建安装包 |
| **帮助目标** | make help | 成功 | 显示帮助信息 |
| **清理目标** | make clean | 成功 | 清理构建文件 |
| | make clean-all | 成功 | 深度清理 |
| | make distclean | 成功 | 完全清理 |

### 退出码
- `0` - 所有测试通过
- `1` - 有测试失败

---

## 4. 手动功能测试

### 4.1 基本功能测试

```bash
# 测试 1: 显示帮助信息
./build/bin/linuxhud --help
```

**预期输出**：
```
Usage: ./build/bin/linuxhud [OPTIONS]

Options:
  -t, --text TEXT      Text to display (default: 'LinuxHUD')
  -x, --x-pos X        X position (default: 10)
  -y, --y-pos Y        Y position (default: 10)
  -w, --width WIDTH    Width (default: 300)
  -h, --height HEIGHT  Height (default: 100)
  -a, --alpha ALPHA    Alpha value 0-255 (default: 200)
  -f, --font FONT      Font family (default: 'Sans')
  -s, --size SIZE      Font size (default: 24)
  -c, --color COLOR    Text color in hex (default: 'FFFFFF')
  -b, --bg-color COLOR Background color in hex (default: '000000')
  -d, --daemon         Run as daemon
  -v, --verbose        Verbose output
  --help               Show this help

Examples:
  ./build/bin/linuxhud -t 'Hello World' -x 100 -y 100
  ./build/bin/linuxhud -t 'System Status' -w 400 -h 150 -a 180
  ./build/bin/linuxhud -d -t 'Background Service'
```

**验证点**：
- ✓ 帮助信息正确显示
- ✓ 所有参数选项都有说明
- ✓ 示例命令正确

---

### 4.2 DRM 设备测试

```bash
# 测试 2: DRM 设备检测
sudo ./build/bin/test_drm
```

**预期输出**：
```
DRM device opened successfully
Connectors: 5
CRTCs: 4
Encoders: 8
Framebuffers: 0
Universal planes capability enabled
Planes: 12
Plane 0: ID=51, CRTC_ID=0, FB_ID=0
  Type: PRIMARY (1)
Plane 1: ID=56, CRTC_ID=0, FB_ID=0
  Type: CURSOR (2)
Plane 2: ID=63, CRTC_ID=0, FB_ID=0
  Type: OVERLAY (0)
...
Connector 0: ID=128, Status=DISCONNECTED
Connector 1: ID=132, Status=CONNECTED
  Modes: 25
  Preferred mode: 1920x1080
...
DRM test completed successfully
```

**验证点**：
- ✓ DRM 设备成功打开
- ✓ 能够获取显示资源
- ✓ 能够发现 Overlay 平面
- ✓ 能够检测已连接的显示器
- ✓ 程序成功退出

---

### 4.3 HUD 显示测试（需要 root 权限）

```bash
# 测试 3: 基本 HUD 显示
sudo ./build/bin/linuxhud -t "Hello LinuxHUD" -v
```

**预期输出**：
```
DRM initialized: Connector=132, CRTC=..., Plane=63, Resolution=1920x1080
Buffer created: FB=..., Size=..., Stride=...
Renderer initialized: 300x100
HUD initialized successfully
Starting HUD with text: 'Hello LinuxHUD'
Press Ctrl+C to stop
Frame 1 rendered
Frame 2 rendered
...
```

**验证点**：
- ✓ DRM 设备成功初始化
- ✓ 缓冲区成功创建
- ✓ 渲染器成功初始化
- ✓ HUD 正常显示
- ✓ 按 Ctrl+C 可以正常退出

---

### 4.4 参数测试

```bash
# 测试 4: 自定义参数
sudo ./build/bin/linuxhud -t "Test" -x 100 -y 100 -w 400 -h 150 -a 180 -s 28 -c "00FF00" -b "000000" -v
```

**预期输出**：
```
DRM initialized: Connector=132, CRTC=..., Plane=63, Resolution=1920x1080
Buffer created: FB=..., Size=..., Stride=...
Renderer initialized: 400x150
HUD initialized successfully
Starting HUD with text: 'Test'
Press Ctrl+C to stop
Frame 1 rendered
...
```

**验证点**：
- ✓ 自定义位置生效 (x=100, y=100)
- ✓ 自定义大小生效 (400x150)
- ✓ 自定义透明度生效 (alpha=180)
- ✓ 自定义字体大小生效 (size=28)
- ✓ 自定义颜色生效 (绿色文字，黑色背景)

---

## 5. 边界条件测试

### 5.1 权限测试

```bash
# 测试 5: 无权限运行
./build/bin/linuxhud -t "Test" 2>&1
```

**预期输出**：
```
Failed to open DRM device: Permission denied
Failed to initialize DRM: Device error
```

**验证点**：
- ✓ 正确检测权限问题
- ✓ 给出清晰的错误信息
- ✓ 程序正常退出（非崩溃）

---

### 5.2 无效参数测试

```bash
# 测试 6: 无效参数
./build/bin/linuxhud --invalid-option 2>&1
```

**预期输出**：
```
./build/bin/linuxhud: unrecognized option '--invalid-option'
Usage: ./build/bin/linuxhud [OPTIONS]
...
```

**验证点**：
- ✓ 正确识别无效参数
- ✓ 显示帮助信息
- ✓ 程序正常退出

---

## 6. 性能测试

### 6.1 刷新率测试

```bash
# 测试 7: 高刷新率测试
sudo ./build/bin/linuxhud -t "Performance Test" -v 2>&1 | head -100
```

**预期输出**：
```
...
Frame 1 rendered
Frame 2 rendered
...
Frame 60 rendered (1秒内)
...
```

**验证点**：
- ✓ 帧率接近 60fps
- ✓ CPU 占用率低
- ✓ 无明显延迟

---

## 7. 测试执行指南

### 7.1 自动测试

```bash
# 运行所有自动测试
./test_project.sh
./test_makefile.sh

# 运行 DRM 测试
sudo ./build/bin/test_drm
```

### 7.2 手动测试

```bash
# 1. 构建项目
make build

# 2. 运行测试
sudo ./build/bin/test_drm

# 3. 测试 HUD
sudo ./build/bin/linuxhud -t "Hello World" -v

# 4. 测试参数
sudo ./build/bin/linuxhud -t "Test" -x 100 -y 100 -w 400 -h 150

# 5. 测试错误处理
./build/bin/linuxhud --help
./build/bin/linuxhud --invalid
```

### 7.3 清理测试

```bash
# 清理构建
make clean

# 深度清理
make clean-all

# 完全清理
make distclean
```

---

## 8. 测试结果判定

### 成功标准

| 类别 | 成功条件 |
|------|----------|
| DRM 测试 | 成功打开设备，发现资源，检测到平面 |
| 项目测试 | 23/23 测试通过 |
| 构建测试 | 16/16 测试通过，1 个跳过 |
| 功能测试 | HUD 正常显示，参数生效 |
| 错误处理 | 正确识别错误，给出清晰提示 |

### 失败条件

| 类别 | 失败条件 |
|------|----------|
| DRM 测试 | 无法打开设备，资源获取失败 |
| 项目测试 | 任何文件缺失或依赖未安装 |
| 构建测试 | 编译失败，链接错误 |
| 功能测试 | HUD 无法显示，参数不生效 |
| 错误处理 | 程序崩溃，无错误提示 |

---

## 9. 已知问题

### 9.1 环境依赖

- **DRM 设备**：需要 `/dev/dri/card1` 存在
- **权限要求**：需要 root 权限或 video 组
- **硬件要求**：需要支持 DRM/KMS 的显卡驱动

### 9.2 已知限制

- **显示器连接**：需要至少一个显示器已连接
- **分辨率检测**：依赖显示器 EDID 信息
- **平面可用性**：Overlay 平面可能被其他程序占用

---

## 10. 测试覆盖范围

### 已覆盖功能

- ✓ DRM 设备访问
- ✓ 资源发现（连接器、CRTC、平面）
- ✓ 平面类型检测
- ✓ 缓冲区创建
- ✓ 渲染器初始化
- ✓ 命令行参数解析
- ✓ 错误处理
- ✓ 信号处理

### 待覆盖功能

- □ 双缓冲机制
- □ 动态更新
- □ 透明度控制
- □ 多显示器支持
- □ 性能优化
- □ 内存泄漏检测

---

## 11. 测试工具

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

---

## 12. 总结

### 测试矩阵

| 测试类型 | 测试数量 | 通过 | 失败 | 跳过 |
|----------|----------|------|------|------|
| DRM 测试 | 1 | 1 | 0 | 0 |
| 项目测试 | 23 | 23 | 0 | 0 |
| 构建测试 | 17 | 16 | 0 | 1 |
| **总计** | **41** | **40** | **0** | **1** |

### 测试状态

```
✓ DRM 硬件测试 - 通过
✓ 项目结构测试 - 通过
✓ 构建系统测试 - 通过
✓ 基本功能测试 - 待手动验证
✓ 参数测试 - 待手动验证
✓ 错误处理测试 - 待手动验证
```

### 下一步

1. 运行自动测试：`./test_project.sh && ./test_makefile.sh`
2. 运行 DRM 测试：`sudo ./build/bin/test_drm`
3. 手动测试 HUD：`sudo ./build/bin/linuxhud -t "Hello World"`
4. 验证参数功能：`sudo ./build/bin/linuxhud -t "Test" -x 100 -y 100 -w 400 -h 150`
