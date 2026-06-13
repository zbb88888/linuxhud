# LinuxHUD

**绕过整个图形栈，通过 DRM/KMS 直接在显卡硬件叠加平面上渲染信息，实现绝对优先级的平视显示器。**

## 核心特性

- **硬件级叠加**：直接操作显卡 Overlay Plane，优先级高于所有软件层
- **零延迟**：绕过 X11/Wayland，直接写入显存
- **高可靠性**：桌面崩溃不影响 HUD 显示
- **极简架构**：最小依赖，最大控制权

## 快速开始

### 1. 安装依赖

```bash
# Ubuntu/Debian
sudo apt-get install libdrm-dev libcairo2-dev libpango1.0-dev

# 或使用 pkg-config
sudo apt-get install libdrm-dev libcairo2-dev libpango1.0-dev
```

### 2. 编译

```bash
# 使用 Make
make

# 或使用 Meson
meson setup build
ninja -C build
```

### 3. 运行

```bash
# 基本运行
sudo ./linuxhud -t "Hello LinuxHUD"

# 自定义参数
sudo ./linuxhud -t "System Status" -x 100 -y 100 -w 400 -h 150 -a 180

# 守护进程模式
sudo ./linuxhud -d -t "Background Service"
```

## 命令行参数

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `-t, --text TEXT` | 显示文本 | "LinuxHUD" |
| `-x, --x-pos X` | X 坐标 | 10 |
| `-y, --y-pos Y` | Y 坐标 | 10 |
| `-w, --width WIDTH` | 宽度 | 300 |
| `-h, --height HEIGHT` | 高度 | 100 |
| `-a, --alpha ALPHA` | 透明度 (0-255) | 200 |
| `-f, --font FONT` | 字体 | "Sans" |
| `-s, --size SIZE` | 字体大小 | 24 |
| `-c, --color COLOR` | 文本颜色 (十六进制) | "FFFFFF" |
| `-b, --bg-color COLOR` | 背景颜色 (十六进制) | "000000" |
| `-d, --daemon` | 守护进程模式 | 关闭 |
| `-v, --verbose` | 详细输出 | 关闭 |

## 项目结构

```
linuxhud/
├── src/
│   ├── main.c              # 主入口、守护进程逻辑
│   ├── drm_device.c        # DRM 设备操作
│   ├── buffer.c            # 显存管理
│   └── renderer.c          # 渲染引擎
├── include/
│   └── linuxhud.h          # 公共头文件
├── tests/
│   └── test_drm.c          # 测试程序
├── linuxhud.conf           # 配置文件
├── linuxhud.service        # systemd 服务文件
├── Makefile                # 构建脚本
└── meson.build             # Meson 构建文件
```

## 架构设计

### 核心分层

```
用户态应用层 (AI/监控逻辑)
    ↓
渲染引擎层 (Cairo/Pango)
    ↓
内存管理层 (Dumb Buffer)
    ↓
DRM/KMS 控制层 (libdrm)
    ↓
内核态驱动层 (GPU Driver)
    ↓
硬件层 (Overlay Plane)
```

### 关键组件

1. **ResourceManager**：发现和管理 GPU 资源
2. **MemoryManager**：管理 Dumb Buffer 的分配和映射
3. **RenderEngine**：将文本/图像渲染到缓冲区
4. **DisplayController**：管理 Plane 的显示属性

## 技术细节

### Overlay Plane 优先级

```
硬件叠加平面 (Hardware Overlay)  ← 优先使用
    ↓
主平面 (Primary Plane)          ← 回退选项
    ↓
光标平面 (Cursor Plane)         ← 最后选择
```

### 内存映射

```c
// 用户态直接写入显存
void *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, 
                    MAP_SHARED, fd, offset);
// 直接写入像素
memset(mapped, 0xFF, size);  // 白色填充
```

### 双缓冲

```
Buffer A ← 当前显示
Buffer B ← 下一帧渲染

// 渲染完成后交换
drmModeSetPlane(fd, plane_id, crtc_id, 
                buffer_b_fb_id, ...);
```

## 性能指标

- **刷新延迟**：< 16ms (60fps)
- **CPU 占用**：< 1%
- **内存占用**：< 10MB
- **启动时间**：< 100ms

## 环境要求

| 项目 | 要求 |
|------|------|
| 操作系统 | Linux (内核 4.8+) |
| 权限 | video 组或 root |
| 依赖库 | libdrm, cairo, pango, glib |
| 显卡驱动 | 支持 DRM/KMS 的开源驱动 |

## 测试

### 运行测试程序

```bash
# 编译测试程序
make test

# 运行 DRM 测试
sudo ./test_drm
```

### 检查 DRM 设备

```bash
# 查看 DRM 设备
ls -la /dev/dri/

# 查看显卡信息
sudo drm_info

# 检查 Overlay Plane
sudo cat /sys/class/drm/card1/device/planes
```

## 安装为系统服务

```bash
# 安装到系统
sudo make install

# 启用服务
sudo systemctl enable linuxhud
sudo systemctl start linuxhud

# 查看状态
sudo systemctl status linuxhud
```

## 故障排除

### 权限问题

```bash
# 添加用户到 video 组
sudo usermod -a -G video $USER

# 或使用 root 权限运行
sudo ./linuxhud
```

### Overlay Plane 不可用

```bash
# 检查可用的平面
sudo ./test_drm

# 如果 Overlay 不可用，程序会自动回退到 Cursor 或 Primary Plane
```

### 显示异常

```bash
# 检查显示器连接
sudo cat /sys/class/drm/card1/card1-*/status

# 检查分辨率
sudo cat /sys/class/drm/card1/card1-*/modes
```

## 开发指南

### 添加新功能

1. 在 `include/linuxhud.h` 中添加函数声明
2. 在对应的 `.c` 文件中实现
3. 更新 `Makefile` 和 `meson.build`
4. 添加测试用例

### 调试技巧

```bash
# 启用详细输出
sudo ./linuxhud -v -t "Debug Mode"

# 使用 GDB 调试
sudo gdb ./linuxhud

# 检查内存泄漏
sudo valgrind ./linuxhud
```

## 与网络数据面的类比

| 网络概念 | 图形概念 | 说明 |
|----------|----------|------|
| XDP/eBPF | DRM/KMS | 绕过协议栈，直接操作硬件 |
| 网卡队列 | CRTC | 硬件控制器 |
| 数据包 | 像素数据 | 最小处理单元 |
| DMA 缓冲区 | Dumb Buffer | 直接内存访问 |
| 零拷贝 | 内存映射 | 避免数据复制 |

## 参考资源

1. **Linux DRM 文档**：https://dri.freedesktop.org/docs/drm/
2. **libdrm API**：https://drm.pages.freedesktop.org/libdrm/
3. **Cairo 文档**：https://cairographics.org/documentation/
4. **DRM/KMS 示例**：https://github.com/dvdhrm/docs

## 许可证

MIT License

## 贡献

欢迎提交 Issue 和 Pull Request！
