#include "linuxhud.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>

/* ========================================================================
 * 全局状态
 * ======================================================================== */

static volatile sig_atomic_t g_running = 1;

static void signal_handler(int sig) {
    (void)sig;
    g_running = 0;
}

/* ========================================================================
 * 命令行参数
 * ======================================================================== */

typedef struct {
    /* 文本 */
    char *text;
    char *font_family;
    uint32_t font_size;
    char *text_color_str;
    char *bg_color_str;

    /* 位置和大小 */
    int32_t x, y;
    uint32_t width, height;

    /* 外观 */
    uint8_t alpha;

    /* 运行模式 */
    bool verbose;
    bool daemon;
    bool show_help;
    bool show_version;
} cli_args_t;

static void cli_defaults(cli_args_t *args) {
    memset(args, 0, sizeof(*args));
    args->text         = "LinuxHUD";
    args->font_family  = "Sans";
    args->font_size    = 24;
    args->text_color_str = "FFFFFF";
    args->bg_color_str = "000000";
    args->x            = 10;
    args->y            = 10;
    args->width        = 400;
    args->height       = 120;
    args->alpha        = 200;
}

static void print_usage(const char *prog) {
    printf(
        "LinuxHUD v%s — 硬件级平视显示器\n"
        "\n"
        "用法: %s [选项]\n"
        "\n"
        "文本选项:\n"
        "  -t, --text TEXT         显示文本 (默认: 'LinuxHUD')\n"
        "  -f, --font FAMILY       字体族 (默认: 'Sans')\n"
        "  -s, --size SIZE         字号 (默认: 24)\n"
        "  -c, --color COLOR       文本颜色 (默认: FFFFFF)\n"
        "  -b, --bg-color COLOR    背景颜色 (默认: 000000)\n"
        "\n"
        "布局选项:\n"
        "  -x, --x-pos X           X 坐标 (默认: 10)\n"
        "  -y, --y-pos Y           Y 坐标 (默认: 10)\n"
        "  -w, --width WIDTH       宽度 (默认: 400)\n"
        "  -h, --height HEIGHT     高度 (默认: 120)\n"
        "  -a, --alpha ALPHA       透明度 0-255 (默认: 200)\n"
        "\n"
        "其他选项:\n"
        "  -d, --daemon            守护进程模式\n"
        "  -v, --verbose           详细输出\n"
        "  --help                  显示帮助\n"
        "  --version               显示版本\n"
        "\n"
        "示例:\n"
        "  %s -t 'CPU: 45%%' -x 50 -y 50 -c 00FF00\n"
        "  %s -t 'Warning!' -c FF0000 -s 32 -a 255\n",
        LINUXHUD_VERSION_STRING, prog, prog, prog);
}

static void print_version(void) {
    printf("LinuxHUD v%s\n", LINUXHUD_VERSION_STRING);
}

static int parse_args(cli_args_t *args, int argc, char *argv[]) {
    static struct option long_options[] = {
        {"text",      required_argument, 0, 't'},
        {"font",      required_argument, 0, 'f'},
        {"size",      required_argument, 0, 's'},
        {"color",     required_argument, 0, 'c'},
        {"bg-color",  required_argument, 0, 'b'},
        {"x-pos",     required_argument, 0, 'x'},
        {"y-pos",     required_argument, 0, 'y'},
        {"width",     required_argument, 0, 'w'},
        {"height",    required_argument, 0, 'h'},
        {"alpha",     required_argument, 0, 'a'},
        {"daemon",    no_argument,       0, 'd'},
        {"verbose",   no_argument,       0, 'v'},
        {"help",      no_argument,       0, 'H'},
        {"version",   no_argument,       0, 'V'},
        {0, 0, 0, 0}
    };

    int opt, idx = 0;
    while ((opt = getopt_long(argc, argv, "t:f:s:c:b:x:y:w:h:a:dv",
                              long_options, &idx)) != -1) {
        switch (opt) {
            case 't': args->text = optarg; break;
            case 'f': args->font_family = optarg; break;
            case 's': args->font_size = (uint32_t)atoi(optarg); break;
            case 'c': args->text_color_str = optarg; break;
            case 'b': args->bg_color_str = optarg; break;
            case 'x': args->x = atoi(optarg); break;
            case 'y': args->y = atoi(optarg); break;
            case 'w': args->width = (uint32_t)atoi(optarg); break;
            case 'h': args->height = (uint32_t)atoi(optarg); break;
            case 'a': args->alpha = (uint8_t)atoi(optarg); break;
            case 'd': args->daemon = true; break;
            case 'v': args->verbose = true; break;
            case 'H': args->show_help = true; break;
            case 'V': args->show_version = true; break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    return 0;
}

/* ========================================================================
 * 主函数
 * ======================================================================== */

int main(int argc, char *argv[]) {
    cli_args_t cli;
    cli_defaults(&cli);

    if (parse_args(&cli, argc, argv) != 0) {
        return 1;
    }

    if (cli.show_help) {
        print_usage(argv[0]);
        return 0;
    }
    if (cli.show_version) {
        print_version();
        return 0;
    }

    /* 设置日志级别 */
    linuxhud_log_set_level(cli.verbose ? LINUXHUD_LOG_DEBUG : LINUXHUD_LOG_INFO);

    /* 解析颜色 */
    linuxhud_color_t text_color, bg_color;
    if (linuxhud_color_parse(cli.text_color_str, &text_color) != LINUXHUD_OK) {
        fprintf(stderr, "Invalid text color: %s\n", cli.text_color_str);
        return 1;
    }
    if (linuxhud_color_parse(cli.bg_color_str, &bg_color) != LINUXHUD_OK) {
        fprintf(stderr, "Invalid background color: %s\n", cli.bg_color_str);
        return 1;
    }

    /* 创建 HUD */
    linuxhud_config_t cfg;
    linuxhud_config_defaults(&cfg);
    cfg.x         = cli.x;
    cfg.y         = cli.y;
    cfg.width     = cli.width;
    cfg.height    = cli.height;
    cfg.alpha     = cli.alpha;
    cfg.bg_color  = bg_color;

    linuxhud_t *hud = NULL;
    linuxhud_error_t err = linuxhud_create(&hud, &cfg);
    if (err != LINUXHUD_OK) {
        fprintf(stderr, "Failed to create HUD: %s\n", linuxhud_error_string(err));
        return 1;
    }

    /* 准备文本样式 */
    linuxhud_text_style_t style = {
        .color = text_color,
        .font  = {
            .family = cli.font_family,
            .size   = cli.font_size,
        },
        .align = LINUXHUD_ALIGN_LEFT,
    };

    /* 注册信号处理 */
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);

    /* 首次显示 */
    err = linuxhud_draw_text(hud, cli.text, &style);
    if (err != LINUXHUD_OK) {
        fprintf(stderr, "Failed to draw: %s\n", linuxhud_error_string(err));
        linuxhud_destroy(hud);
        return 1;
    }

    LINUXHUD_INFO("HUD running: '%s' at (%d,%d), press Ctrl+C to stop",
                  cli.text, cli.x, cli.y);

    /* 主循环 */
    if (cli.daemon) {
        while (g_running) {
            /* 守护进程模式：持续刷新 */
            linuxhud_draw_text(hud, cli.text, &style);
            usleep(16666);  /* ~60fps */
        }
    } else {
        /* 非守护进程模式：保持显示直到 Ctrl+C */
        while (g_running) {
            pause();
        }
    }

    /* 清理 */
    linuxhud_destroy(hud);
    LINUXHUD_INFO("HUD stopped");
    return 0;
}
