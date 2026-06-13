#include "linuxhud.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

// 全局变量用于信号处理
static linuxhud_t *g_hud = NULL;
static volatile sig_atomic_t g_running = 1;

// 错误码到字符串的转换
const char *linuxhud_error_string(linuxhud_error_t error) {
    switch (error) {
        case LINUXHUD_OK:
            return "Success";
        case LINUXHUD_ERROR_DEVICE:
            return "Device error";
        case LINUXHUD_ERROR_RESOURCE:
            return "Resource error";
        case LINUXHUD_ERROR_PLANE:
            return "Plane error";
        case LINUXHUD_ERROR_BUFFER:
            return "Buffer error";
        case LINUXHUD_ERROR_RENDER:
            return "Render error";
        case LINUXHUD_ERROR_PERMISSION:
            return "Permission error";
        default:
            return "Unknown error";
    }
}

// 信号处理函数
static void signal_handler(int sig) {
    (void)sig;
    g_running = 0;
    if (g_hud) {
        g_hud->running = 0;
    }
}

// 显示帮助信息
static void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("\nOptions:\n");
    printf("  -t, --text TEXT      Text to display (default: 'LinuxHUD')\n");
    printf("  -x, --x-pos X        X position (default: 10)\n");
    printf("  -y, --y-pos Y        Y position (default: 10)\n");
    printf("  -w, --width WIDTH    Width (default: 300)\n");
    printf("  -h, --height HEIGHT  Height (default: 100)\n");
    printf("  -a, --alpha ALPHA    Alpha value 0-255 (default: 200)\n");
    printf("  -f, --font FONT      Font family (default: 'Sans')\n");
    printf("  -s, --size SIZE      Font size (default: 24)\n");
    printf("  -c, --color COLOR    Text color in hex (default: 'FFFFFF')\n");
    printf("  -b, --bg-color COLOR Background color in hex (default: '000000')\n");
    printf("  -d, --daemon         Run as daemon\n");
    printf("  -v, --verbose        Verbose output\n");
    printf("  --help               Show this help\n");
    printf("\nExamples:\n");
    printf("  %s -t 'Hello World' -x 100 -y 100\n", program_name);
    printf("  %s -t 'System Status' -w 400 -h 150 -a 180\n", program_name);
    printf("  %s -d -t 'Background Service'\n", program_name);
}

// 解析十六进制颜色
static uint32_t parse_color(const char *color_str) {
    if (!color_str) return 0xFFFFFFFF;
    
    char *endptr;
    uint32_t color = strtoul(color_str, &endptr, 16);
    
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid color: %s\n", color_str);
        return 0xFFFFFFFF;
    }
    
    // 如果只有6位，添加Alpha通道
    if (color <= 0xFFFFFF) {
        color |= 0xFF000000;
    }
    
    return color;
}

// 初始化 HUD
static linuxhud_error_t init_hud(linuxhud_t *hud, const linuxhud_config_t *config) {
    // 初始化 DRM
    linuxhud_error_t err = linuxhud_drm_init(&hud->drm);
    if (err != LINUXHUD_OK) {
        fprintf(stderr, "Failed to initialize DRM: %s\n", linuxhud_error_string(err));
        return err;
    }
    
    // 创建缓冲区
    err = linuxhud_buffer_create(&hud->drm, config->width, config->height);
    if (err != LINUXHUD_OK) {
        fprintf(stderr, "Failed to create buffer: %s\n", linuxhud_error_string(err));
        linuxhud_drm_cleanup(&hud->drm);
        return err;
    }
    
    // 初始化渲染器
    err = linuxhud_renderer_init(&hud->render, hud->drm.mapped_buffer,
                                 config->width, config->height, hud->drm.stride);
    if (err != LINUXHUD_OK) {
        fprintf(stderr, "Failed to initialize renderer: %s\n", linuxhud_error_string(err));
        linuxhud_buffer_destroy(&hud->drm);
        linuxhud_drm_cleanup(&hud->drm);
        return err;
    }
    
    // 设置渲染器属性
    linuxhud_renderer_set_text_color(&hud->render, config->alpha << 24 | 0xFFFFFF);
    linuxhud_renderer_set_bg_color(&hud->render, config->alpha << 24 | 0x000000);
    linuxhud_renderer_set_font(&hud->render, "Sans", 24);
    
    // 保存配置
    hud->config = *config;
    hud->state = LINUXHUD_STATE_READY;
    hud->running = 1;
    
    printf("HUD initialized successfully\n");
    return LINUXHUD_OK;
}

// 更新 HUD 显示
static linuxhud_error_t update_hud(linuxhud_t *hud, const char *text) {
    if (!hud || (hud->state != LINUXHUD_STATE_READY && hud->state != LINUXHUD_STATE_RUNNING)) {
        return LINUXHUD_ERROR_RENDER;
    }
    
    // 清除背景
    linuxhud_renderer_clear(&hud->render, hud->config.width, hud->config.height);
    
    // 绘制圆角矩形背景
    linuxhud_renderer_draw_rounded_rect(&hud->render, 0, 0, 
                                        hud->config.width, hud->config.height,
                                        10, hud->config.alpha << 24 | 0x000000, true);
    
    // 绘制文本
    linuxhud_renderer_draw_text(&hud->render, text, 10, 10, 
                                hud->config.width - 20, hud->config.height - 20);
    
    // 提交到显示
    int ret = drmModeSetPlane(hud->drm.fd, hud->drm.plane_id, hud->drm.crtc_id,
                              hud->drm.fb_id, 0,
                              hud->config.x, hud->config.y,
                              hud->config.width, hud->config.height,
                              0, 0, hud->config.width << 16, hud->config.height << 16);
    
    if (ret) {
        fprintf(stderr, "Failed to set plane: %s\n", strerror(errno));
        return LINUXHUD_ERROR_RENDER;
    }
    
    hud->frame_count++;
    return LINUXHUD_OK;
}

// 清理 HUD
static void cleanup_hud(linuxhud_t *hud) {
    if (!hud) return;
    
    printf("Cleaning up HUD...\n");
    
    // 清理渲染器
    linuxhud_renderer_cleanup(&hud->render);
    
    // 清理缓冲区
    linuxhud_buffer_destroy(&hud->drm);
    
    // 清理 DRM
    linuxhud_drm_cleanup(&hud->drm);
    
    hud->state = LINUXHUD_STATE_INIT;
    printf("HUD cleaned up\n");
}

int main(int argc, char *argv[]) {
    // 默认配置
    linuxhud_config_t config = {
        .width = 300,
        .height = 100,
        .x = 10,
        .y = 10,
        .alpha = 200,
        .z_order = 1,
        .preferred_plane = LINUXHUD_PLANE_OVERLAY
    };
    
    char *text = "LinuxHUD";
    char *font_family = "Sans";
    uint32_t font_size = 24;
    uint32_t text_color = 0xFFFFFFFF;
    uint32_t bg_color = 0xFF000000;
    int daemon_mode = 0;
    int verbose = 0;
    
    // 命令行选项
    static struct option long_options[] = {
        {"text", required_argument, 0, 't'},
        {"x-pos", required_argument, 0, 'x'},
        {"y-pos", required_argument, 0, 'y'},
        {"width", required_argument, 0, 'w'},
        {"height", required_argument, 0, 'h'},
        {"alpha", required_argument, 0, 'a'},
        {"font", required_argument, 0, 'f'},
        {"size", required_argument, 0, 's'},
        {"color", required_argument, 0, 'c'},
        {"bg-color", required_argument, 0, 'b'},
        {"daemon", no_argument, 0, 'd'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'H'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "t:x:y:w:h:a:f:s:c:b:dvH", long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                text = optarg;
                break;
            case 'x':
                config.x = atoi(optarg);
                break;
            case 'y':
                config.y = atoi(optarg);
                break;
            case 'w':
                config.width = atoi(optarg);
                break;
            case 'h':
                config.height = atoi(optarg);
                break;
            case 'a':
                config.alpha = atoi(optarg);
                break;
            case 'f':
                font_family = optarg;
                break;
            case 's':
                font_size = atoi(optarg);
                break;
            case 'c':
                text_color = parse_color(optarg);
                break;
            case 'b':
                bg_color = parse_color(optarg);
                break;
            case 'd':
                daemon_mode = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'H':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 初始化 HUD
    linuxhud_t hud = {0};
    g_hud = &hud;
    
    linuxhud_error_t err = init_hud(&hud, &config);
    if (err != LINUXHUD_OK) {
        fprintf(stderr, "Failed to initialize HUD: %s\n", linuxhud_error_string(err));
        return 1;
    }
    
    // 设置渲染器属性
    linuxhud_renderer_set_text_color(&hud.render, text_color);
    linuxhud_renderer_set_bg_color(&hud.render, bg_color);
    linuxhud_renderer_set_font(&hud.render, font_family, font_size);
    
    // 守护进程模式
    if (daemon_mode) {
        printf("Running as daemon...\n");
        // 这里可以添加守护进程化代码
    }
    
    printf("Starting HUD with text: '%s'\n", text);
    printf("Press Ctrl+C to stop\n");
    
    // 主循环
    hud.state = LINUXHUD_STATE_RUNNING;
    
    while (g_running && hud.running) {
        err = update_hud(&hud, text);
        if (err != LINUXHUD_OK) {
            fprintf(stderr, "Failed to update HUD: %s\n", linuxhud_error_string(err));
            break;
        }
        
        if (verbose) {
            printf("Frame %u rendered\n", hud.frame_count);
        }
        
        // 控制刷新率 (约60fps)
        usleep(16666);
    }
    
    // 清理
    cleanup_hud(&hud);
    g_hud = NULL;
    
    printf("HUD stopped\n");
    return 0;
}
