# LinuxHUD 项目摘要

## 项目定位

**一句话定义**：绕过整个图形栈，通过 DRM/KMS 直接在显卡硬件叠加平面上渲染信息，实现绝对优先级的平视显示器。

**设计哲学**：
- 硬件优先：绕过 X11/Wayland，直接操作显卡
- 零妥协：桌面卡死也能瞬间刷新
- 极简主义：最小依赖，最大控制权

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

## 技术选型

### 编程语言：C
**理由**：
- 零开销抽象，直接调用内核接口
- 精确内存控制
- 与 Linux 内核 API 天然契合
- 最小依赖

### 核心依赖
- **libdrm**：DRM/KMS 接口
- **cairo**：2D 图形渲染
- **pango**：文本布局
- **glib**：基础数据结构

## 简要实现方案

### Phase 1：MVP（最小可行产品）
```c
// 核心流程
1. open("/dev/dri/card0")           // 打开 DRM 设备
2. drmSetClientCap(DRM_CLIENT_CAP_UNIVERSAL_PLANES)  // 启用所有平面
3. drmModeGetResources()            // 获取显示资源
4. find_overlay_plane()             // 找到 Overlay Plane
5. create_dumb_buffer()             // 创建哑缓冲区
6. mmap()                           // 映射到用户空间
7. cairo_create() + draw_text()     // 渲染文本
8. drmModeSetPlane()                // 提交到硬件
```

### Phase 2：动态更新
- 双缓冲机制
- 定时更新循环
- 事件驱动架构

### Phase 3：AI 集成
- 与 AI 服务通信（Unix Socket/DBus）
- 状态机管理
- 配置文件支持

## 代码结构
```
linuxhud/
├── src/
│   ├── main.c              # 主入口
│   ├── drm_device.c        # DRM 设备操作
│   ├── buffer.c            # 显存管理
│   └── renderer.c          # 渲染引擎
├── include/
│   └── linuxhud.h          # 公共头文件
└── tests/
    └── test_overlay.c      # 测试程序
```

## 性能目标
- 刷新延迟：< 16ms (60fps)
- CPU 占用：< 1%
- 内存占用：< 10MB
- 启动时间：< 100ms

## 验证方法
```bash
# 检查 DRM 设备
ls -la /dev/dri/

# 查看显卡信息
sudo drm_info

# 检查 Overlay Plane
sudo cat /sys/class/drm/card1/device/planes
```

## 核心优势
1. **绝对优先级**：硬件级叠加，高于所有软件层
2. **零延迟**：直接写入显存，无上下文切换
3. **高可靠性**：桌面崩溃不影响 HUD 显示
4. **低资源占用**：最小化系统开销

---

**下一步**：开始实现 Phase 1，创建最小可行的 Overlay Plane 显示。
