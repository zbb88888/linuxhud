/**
 * test_widget.c — Widget API 测试
 *
 * 测试 Widget 系统的创建、属性设置和绘制
 */

#include "linuxhud.h"
#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  测试: %-40s ", name); \
    tests_run++; \
} while(0)

#define PASS() do { printf("✓\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("✗ %s\n", msg); } while(0)

/* ========================================================================
 * 类型系统测试
 * ======================================================================== */

static void test_color_parse(void) {
    linuxhud_color_t color;

    TEST("颜色解析 - RRGGBB");
    if (linuxhud_color_parse("FF0000", &color) == LINUXHUD_OK && color == 0xFFFF0000)
        PASS();
    else
        FAIL("expected 0xFFFF0000");

    TEST("颜色解析 - #RRGGBB");
    if (linuxhud_color_parse("#00FF00", &color) == LINUXHUD_OK && color == 0xFF00FF00)
        PASS();
    else
        FAIL("expected 0xFF00FF00");

    TEST("颜色解析 - AARRGGBB");
    if (linuxhud_color_parse("80FF0000", &color) == LINUXHUD_OK && color == 0x80FF0000)
        PASS();
    else
        FAIL("expected 0x80FF0000");

    TEST("颜色解析 - 无效输入");
    if (linuxhud_color_parse("ZZZZZZ", &color) == LINUXHUD_ERROR_INVAL)
        PASS();
    else
        FAIL("expected INVAL");

    TEST("颜色解析 - NULL");
    if (linuxhud_color_parse(NULL, &color) == LINUXHUD_ERROR_INVAL)
        PASS();
    else
        FAIL("expected INVAL");
}

static void test_color_constructors(void) {
    TEST("linuxhud_color_rgb");
    if (linuxhud_color_rgb(0xFF, 0x00, 0x00) == 0xFFFF0000)
        PASS();
    else
        FAIL("expected 0xFFFF0000");

    TEST("linuxhud_color_rgba");
    if (linuxhud_color_rgba(0x00, 0xFF, 0x00, 0x80) == 0x8000FF00)
        PASS();
    else
        FAIL("expected 0x8000FF00");
}

/* ========================================================================
 * Widget 测试
 * ======================================================================== */

static void test_label(void) {
    linuxhud_label_t *label = linuxhud_label_create("Hello");

    TEST("Label 创建");
    if (label && label->base.type == LINUXHUD_WIDGET_LABEL)
        PASS();
    else
        FAIL("creation failed");

    TEST("Label 文本设置");
    linuxhud_label_set_text(label, "World");
    if (strcmp(label->text, "World") == 0)
        PASS();
    else
        FAIL("text mismatch");

    TEST("Label 颜色设置");
    linuxhud_label_set_color(label, LINUXHUD_COLOR_RED);
    if (label->style.color == LINUXHUD_COLOR_RED)
        PASS();
    else
        FAIL("color mismatch");

    TEST("Label 背景设置");
    linuxhud_label_set_bg(label, LINUXHUD_COLOR_BLACK);
    if (label->bg_color == LINUXHUD_COLOR_BLACK)
        PASS();
    else
        FAIL("bg mismatch");

    TEST("Label 内边距设置");
    linuxhud_label_set_padding(label, 10);
    if (label->padding == 10)
        PASS();
    else
        FAIL("padding mismatch");

    TEST("Label 圆角设置");
    linuxhud_label_set_radius(label, 8);
    if (label->border_radius == 8)
        PASS();
    else
        FAIL("radius mismatch");

    linuxhud_widget_destroy((linuxhud_widget_t *)label);
}

static void test_rect(void) {
    linuxhud_rect_widget_t *rect = linuxhud_rect_create();

    TEST("Rect 创建");
    if (rect && rect->base.type == LINUXHUD_WIDGET_RECT)
        PASS();
    else
        FAIL("creation failed");

    TEST("Rect 填充色设置");
    linuxhud_rect_set_fill(rect, LINUXHUD_COLOR_BLUE);
    if (rect->fill_color == LINUXHUD_COLOR_BLUE)
        PASS();
    else
        FAIL("fill mismatch");

    TEST("Rect 描边设置");
    linuxhud_rect_set_stroke(rect, LINUXHUD_COLOR_WHITE, 2);
    if (rect->stroke_color == LINUXHUD_COLOR_WHITE && rect->stroke_width == 2)
        PASS();
    else
        FAIL("stroke mismatch");

    TEST("Rect 圆角设置");
    linuxhud_rect_set_radius(rect, 12);
    if (rect->border_radius == 12)
        PASS();
    else
        FAIL("radius mismatch");

    linuxhud_widget_destroy((linuxhud_widget_t *)rect);
}

static void test_progress(void) {
    linuxhud_progress_t *prog = linuxhud_progress_create();

    TEST("Progress 创建");
    if (prog && prog->base.type == LINUXHUD_WIDGET_PROGRESS)
        PASS();
    else
        FAIL("creation failed");

    TEST("Progress 值设置");
    linuxhud_progress_set_value(prog, 0.75f);
    if (prog->progress >= 0.74f && prog->progress <= 0.76f)
        PASS();
    else
        FAIL("value mismatch");

    TEST("Progress 值范围限制 (>1)");
    linuxhud_progress_set_value(prog, 1.5f);
    if (prog->progress <= 1.01f)
        PASS();
    else
        FAIL("not clamped");

    TEST("Progress 值范围限制 (<0)");
    linuxhud_progress_set_value(prog, -0.5f);
    if (prog->progress >= -0.01f)
        PASS();
    else
        FAIL("not clamped");

    TEST("Progress 颜色设置");
    linuxhud_progress_set_colors(prog, LINUXHUD_COLOR_GREEN, LINUXHUD_COLOR_BLACK);
    if (prog->bar_color == LINUXHUD_COLOR_GREEN && prog->track_color == LINUXHUD_COLOR_BLACK)
        PASS();
    else
        FAIL("colors mismatch");

    linuxhud_widget_destroy((linuxhud_widget_t *)prog);
}

static void test_group(void) {
    linuxhud_group_t *group = linuxhud_group_create();

    TEST("Group 创建");
    if (group && group->base.type == LINUXHUD_WIDGET_GROUP)
        PASS();
    else
        FAIL("creation failed");

    /* 添加子元素 */
    linuxhud_label_t *label1 = linuxhud_label_create("Label 1");
    linuxhud_label_t *label2 = linuxhud_label_create("Label 2");
    linuxhud_rect_widget_t *bg = linuxhud_rect_create();

    linuxhud_group_add(group, (linuxhud_widget_t *)bg);
    linuxhud_group_add(group, (linuxhud_widget_t *)label1);
    linuxhud_group_add(group, (linuxhud_widget_t *)label2);

    TEST("Group 子元素链表");
    if (group->children == (linuxhud_widget_t *)bg &&
        bg->base.next == (linuxhud_widget_t *)label1 &&
        label1->base.next == (linuxhud_widget_t *)label2 &&
        label2->base.next == NULL)
        PASS();
    else
        FAIL("children list mismatch");

    TEST("Group 移除子元素");
    linuxhud_group_remove(group, (linuxhud_widget_t *)label1);
    if (group->children == (linuxhud_widget_t *)bg &&
        bg->base.next == (linuxhud_widget_t *)label2)
        PASS();
    else
        FAIL("remove failed");

    TEST("Group 背景设置");
    linuxhud_group_set_bg(group, linuxhud_color_rgba(0, 0, 0, 128));
    if (group->bg_color == linuxhud_color_rgba(0, 0, 0, 128))
        PASS();
    else
        FAIL("bg mismatch");

    TEST("Group 间距设置");
    linuxhud_group_set_gap(group, 8);
    if (group->gap == 8)
        PASS();
    else
        FAIL("gap mismatch");

    /* 垂直布局 */
    group->base.bounds.x = 0;
    group->base.bounds.y = 0;
    group->base.bounds.w = 200;
    group->base.bounds.h = 100;
    linuxhud_group_set_padding(group, 10);
    linuxhud_group_set_gap(group, 5);
    linuxhud_label_set_text(label1, "Relabel");  /* 已被移除，不影响 */
    linuxhud_label_set_text(label2, "Label 2");
    label2->base.bounds.h = 30;

    linuxhud_group_layout_vertical(group);

    TEST("Group 垂直布局");
    if (bg->base.bounds.x == 10 && bg->base.bounds.y == 10)
        PASS();
    else
        FAIL("layout mismatch");

    /* 销毁 group 会递归销毁所有子元素 */
    linuxhud_widget_destroy((linuxhud_widget_t *)group);
}

static void test_widget_visibility(void) {
    linuxhud_label_t *label = linuxhud_label_create("Visible");

    TEST("Widget 默认可见");
    if (label->base.visible == true)
        PASS();
    else
        FAIL("not visible by default");

    TEST("Widget 设置不可见");
    linuxhud_widget_set_visible((linuxhud_widget_t *)label, false);
    if (label->base.visible == false)
        PASS();
    else
        FAIL("still visible");

    linuxhud_widget_destroy((linuxhud_widget_t *)label);
}

static void test_widget_bounds(void) {
    linuxhud_rect_widget_t *rect = linuxhud_rect_create();

    TEST("Widget 设置位置");
    linuxhud_widget_set_position((linuxhud_widget_t *)rect, 50, 100);
    if (rect->base.bounds.x == 50 && rect->base.bounds.y == 100)
        PASS();
    else
        FAIL("position mismatch");

    TEST("Widget 设置大小");
    linuxhud_widget_set_size((linuxhud_widget_t *)rect, 200, 150);
    if (rect->base.bounds.w == 200 && rect->base.bounds.h == 150)
        PASS();
    else
        FAIL("size mismatch");

    linuxhud_rect_t bounds = { .x = 10, .y = 20, .w = 300, .h = 250 };
    TEST("Widget 设置边界");
    linuxhud_widget_set_bounds((linuxhud_widget_t *)rect, &bounds);
    if (rect->base.bounds.x == 10 && rect->base.bounds.w == 300)
        PASS();
    else
        FAIL("bounds mismatch");

    linuxhud_widget_destroy((linuxhud_widget_t *)rect);
}

/* ========================================================================
 * 主函数
 * ======================================================================== */

int main(void) {
    printf("==========================================\n");
    printf("  LinuxHUD Widget 测试\n");
    printf("==========================================\n\n");

    printf("类型系统测试:\n");
    test_color_parse();
    test_color_constructors();

    printf("\nWidget 基础测试:\n");
    test_label();
    test_rect();
    test_progress();

    printf("\nWidget 属性测试:\n");
    test_widget_visibility();
    test_widget_bounds();

    printf("\nGroup 测试:\n");
    test_group();

    printf("\n==========================================\n");
    printf("  结果: %d/%d 测试通过\n", tests_passed, tests_run);
    printf("==========================================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
