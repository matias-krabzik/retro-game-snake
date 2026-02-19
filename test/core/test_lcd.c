/*
 * LCD framebuffer tests — tests the core LCD module.
 * Uses lcd_get_framebuffer() to inspect pixel state.
 */
#include "../test.h"
#include "../../src/core/lcd.h"

/* Helper: read a pixel from the framebuffer */
static int px(int x, int y)
{
    const uint8_t (*fb)[LCD_WIDTH] = lcd_get_framebuffer();
    return fb[y][x];
}

/* --- lcd_clear --- */

static void test_clear(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_set_pixel(0, 0, true);
    lcd_set_pixel(20, 10, true);
    lcd_clear();
    ASSERT_EQ(px(0, 0), 0);
    ASSERT_EQ(px(20, 10), 0);
}

/* --- lcd_set_pixel --- */

static void test_set_pixel_on(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_set_pixel(5, 3, true);
    ASSERT_EQ(px(5, 3), 1);
}

static void test_set_pixel_off(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_set_pixel(5, 3, true);
    lcd_set_pixel(5, 3, false);
    ASSERT_EQ(px(5, 3), 0);
}

static void test_set_pixel_out_of_bounds(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_set_pixel(-1, 0, true);
    lcd_set_pixel(0, -1, true);
    lcd_set_pixel(LCD_WIDTH, 0, true);
    lcd_set_pixel(0, LCD_HEIGHT, true);
    ASSERT_EQ(px(0, 0), 0);
    ASSERT_EQ(px(LCD_WIDTH - 1, 0), 0);
    ASSERT_EQ(px(0, LCD_HEIGHT - 1), 0);
}

/* --- lcd_set_pixel_inv --- */

static void test_pixel_invert(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_set_pixel_inv(3, 3);
    ASSERT_EQ(px(3, 3), 1);
    lcd_set_pixel_inv(3, 3);
    ASSERT_EQ(px(3, 3), 0);
}

/* --- lcd_fill_rect --- */

static void test_fill_rect(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_fill_rect(2, 2, 3, 3);
    ASSERT_EQ(px(2, 2), 1);
    ASSERT_EQ(px(4, 4), 1);
    ASSERT_EQ(px(1, 2), 0);
    ASSERT_EQ(px(5, 2), 0);
    ASSERT_EQ(px(2, 1), 0);
    ASSERT_EQ(px(2, 5), 0);
}

/* --- lcd_draw_rect --- */

static void test_draw_rect_outline(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_draw_rect(0, 0, 5, 5);
    ASSERT_EQ(px(0, 0), 1);
    ASSERT_EQ(px(4, 0), 1);
    ASSERT_EQ(px(0, 4), 1);
    ASSERT_EQ(px(4, 4), 1);
    ASSERT_EQ(px(2, 2), 0);
    ASSERT_EQ(px(2, 0), 1);
    ASSERT_EQ(px(0, 2), 1);
}

/* --- lcd_draw_hline / lcd_draw_vline --- */

static void test_hline(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_draw_hline(1, 0, 4);
    ASSERT_EQ(px(0, 0), 0);
    ASSERT_EQ(px(1, 0), 1);
    ASSERT_EQ(px(4, 0), 1);
    ASSERT_EQ(px(5, 0), 0);
}

static void test_vline(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_draw_vline(0, 1, 4);
    ASSERT_EQ(px(0, 0), 0);
    ASSERT_EQ(px(0, 1), 1);
    ASSERT_EQ(px(0, 4), 1);
    ASSERT_EQ(px(0, 5), 0);
}

/* --- lcd_draw_sprite --- */

static void test_draw_sprite(void)
{
    lcd_clear();
    lcd_clear_clip();
    uint8_t data[] = {1,0, 1,1};
    lcd_draw_sprite(10, 10, data, 2, 2);
    ASSERT_EQ(px(10, 10), 1);
    ASSERT_EQ(px(11, 10), 0);
    ASSERT_EQ(px(10, 11), 1);
    ASSERT_EQ(px(11, 11), 1);
}

/* --- lcd_draw_sprite_inv --- */

static void test_draw_sprite_inv(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_fill_rect(10, 10, 2, 2);
    uint8_t data[] = {1,0, 0,1};
    lcd_draw_sprite_inv(10, 10, data, 2, 2);
    ASSERT_EQ(px(10, 10), 0);
    ASSERT_EQ(px(11, 10), 1);
    ASSERT_EQ(px(10, 11), 1);
    ASSERT_EQ(px(11, 11), 0);
}

/* --- lcd_invert_rect --- */

static void test_invert_rect(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_set_pixel(1, 1, true);
    lcd_invert_rect(0, 0, 3, 3);
    ASSERT_EQ(px(0, 0), 1);
    ASSERT_EQ(px(1, 1), 0);
    ASSERT_EQ(px(2, 2), 1);
}

/* --- clipping --- */

static void test_clip_blocks_outside(void)
{
    lcd_clear();
    lcd_set_clip(5, 5, 10, 10);
    lcd_set_pixel(4, 5, true);
    lcd_set_pixel(10, 5, true);
    lcd_set_pixel(5, 4, true);
    lcd_set_pixel(5, 10, true);
    ASSERT_EQ(px(4, 5), 0);
    ASSERT_EQ(px(10, 5), 0);
    ASSERT_EQ(px(5, 4), 0);
    ASSERT_EQ(px(5, 10), 0);
    lcd_clear_clip();
}

static void test_clip_allows_inside(void)
{
    lcd_clear();
    lcd_set_clip(5, 5, 10, 10);
    lcd_set_pixel(5, 5, true);
    lcd_set_pixel(9, 9, true);
    ASSERT_EQ(px(5, 5), 1);
    ASSERT_EQ(px(9, 9), 1);
    lcd_clear_clip();
}

static void test_clear_clip_restores(void)
{
    lcd_clear();
    lcd_set_clip(5, 5, 10, 10);
    lcd_clear_clip();
    lcd_set_pixel(0, 0, true);
    ASSERT_EQ(px(0, 0), 1);
}

static void test_clip_clamps_negative(void)
{
    lcd_clear();
    lcd_set_clip(-5, -5, 10, 10);
    lcd_set_pixel(0, 0, true);
    ASSERT_EQ(px(0, 0), 1);
    lcd_clear_clip();
}

static void test_fill_rect_respects_clip(void)
{
    lcd_clear();
    lcd_set_clip(5, 5, 8, 8);
    lcd_fill_rect(0, 0, 20, 20);
    ASSERT_EQ(px(4, 5), 0);
    ASSERT_EQ(px(8, 5), 0);
    ASSERT_EQ(px(5, 5), 1);
    ASSERT_EQ(px(7, 7), 1);
    lcd_clear_clip();
}

/* --- lcd_get_framebuffer --- */

static void test_get_framebuffer(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_set_pixel(42, 24, true);
    const uint8_t (*fb)[LCD_WIDTH] = lcd_get_framebuffer();
    ASSERT_EQ(fb[24][42], 1);
    ASSERT_EQ(fb[0][0], 0);
}

int main(void)
{
    printf("[test_lcd]\n");

    RUN(test_clear);
    RUN(test_set_pixel_on);
    RUN(test_set_pixel_off);
    RUN(test_set_pixel_out_of_bounds);
    RUN(test_pixel_invert);
    RUN(test_fill_rect);
    RUN(test_draw_rect_outline);
    RUN(test_hline);
    RUN(test_vline);
    RUN(test_draw_sprite);
    RUN(test_draw_sprite_inv);
    RUN(test_invert_rect);
    RUN(test_clip_blocks_outside);
    RUN(test_clip_allows_inside);
    RUN(test_clear_clip_restores);
    RUN(test_clip_clamps_negative);
    RUN(test_fill_rect_respects_clip);
    RUN(test_get_framebuffer);

    TEST_REPORT();
}
