/*
 * Sprite/font tests — tests core sprites and text helpers.
 * Uses lcd_get_framebuffer() to inspect pixel state.
 */
#include "../test.h"
#include "../../src/core/lcd.h"
#include "../../src/core/sprites.h"

/* Helper: read a pixel from the framebuffer */
static int px(int x, int y)
{
    const uint8_t (*fb)[LCD_WIDTH] = lcd_get_framebuffer();
    return fb[y][x];
}

/* --- lcd_text_width --- */

static void test_text_width_empty(void)
{
    ASSERT_EQ(lcd_text_width(""), 0);
}

static void test_text_width_single_char(void)
{
    ASSERT_EQ(lcd_text_width("A"), SFONT_W);
}

static void test_text_width_two_chars(void)
{
    ASSERT_EQ(lcd_text_width("AB"), SFONT_W * 2 + 1);
}

static void test_text_width_with_spaces(void)
{
    ASSERT_EQ(lcd_text_width("A B"), SFONT_W * 3 + 2);
}

/* --- lcd_draw_text --- */

static void test_draw_text_A(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_draw_text(0, 0, "A");
    /* 'A' glyph top row: 0,1,1,1,0 */
    ASSERT_EQ(px(0, 0), 0);
    ASSERT_EQ(px(1, 0), 1);
    ASSERT_EQ(px(2, 0), 1);
    ASSERT_EQ(px(3, 0), 1);
    ASSERT_EQ(px(4, 0), 0);
}

static void test_draw_text_offset(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_draw_text(10, 5, "A");
    ASSERT_EQ(px(10, 5), 0);
    ASSERT_EQ(px(11, 5), 1);
    ASSERT_EQ(px(12, 5), 1);
}

static void test_draw_text_lowercase(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_draw_text(0, 0, "a");
    /* 'a' glyph: rows 0-1 are blank, row 2 is 0,1,1,1,0 */
    ASSERT_EQ(px(0, 0), 0);
    ASSERT_EQ(px(1, 0), 0);
    ASSERT_EQ(px(1, 2), 1);
    ASSERT_EQ(px(2, 2), 1);
    ASSERT_EQ(px(3, 2), 1);
}

/* --- lcd_draw_text_inv --- */

static void test_draw_text_inv(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_fill_rect(0, 0, SFONT_W, SFONT_H);
    lcd_draw_text_inv(0, 0, "I");
    /* 'I' glyph row 0: 0,1,1,1,0 -> inverted: those pixels turn off */
    ASSERT_EQ(px(1, 0), 0);
    ASSERT_EQ(px(2, 0), 0);
    ASSERT_EQ(px(3, 0), 0);
    /* Pixels NOT in glyph stay on */
    ASSERT_EQ(px(0, 0), 1);
    ASSERT_EQ(px(4, 0), 1);
}

/* --- lcd_draw_digit --- */

static void test_draw_digit_zero(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_draw_digit(0, 0, 0);
    /* '0' top row: 1,1,1 */
    ASSERT_EQ(px(0, 0), 1);
    ASSERT_EQ(px(1, 0), 1);
    ASSERT_EQ(px(2, 0), 1);
    /* '0' middle row (row 1): 1,0,1 */
    ASSERT_EQ(px(0, 1), 1);
    ASSERT_EQ(px(1, 1), 0);
    ASSERT_EQ(px(2, 1), 1);
}

static void test_draw_digit_invalid(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_draw_digit(0, 0, -1);
    lcd_draw_digit(0, 0, 10);
    ASSERT_EQ(px(0, 0), 0);
}

/* --- lcd_draw_number --- */

static void test_draw_number_zero(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_draw_number(0, 0, 0);
    ASSERT_EQ(px(0, 0), 1);
}

static void test_draw_number_multi_digit(void)
{
    lcd_clear();
    lcd_clear_clip();
    lcd_draw_number(0, 0, 12);
    /* First digit '1' at x=0: top row 0,1,0 */
    ASSERT_EQ(px(0, 0), 0);
    ASSERT_EQ(px(1, 0), 1);
    /* Second digit '2' at x=4 (DIGIT_W+1): top row 1,1,1 */
    ASSERT_EQ(px(4, 0), 1);
    ASSERT_EQ(px(5, 0), 1);
    ASSERT_EQ(px(6, 0), 1);
}

/* --- font coverage --- */

static void test_font_uppercase_range(void)
{
    for (char c = 'A'; c <= 'Z'; c++) {
        int idx = c - SFONT_FIRST;
        int sum = 0;
        for (int i = 0; i < SFONT_H * SFONT_W; i++) {
            sum += FONT_SMALL[idx][i];
        }
        ASSERT_TRUE(sum > 0);
    }
}

static void test_font_lowercase_range(void)
{
    for (char c = 'a'; c <= 'z'; c++) {
        int idx = c - SFONT_FIRST;
        int sum = 0;
        for (int i = 0; i < SFONT_H * SFONT_W; i++) {
            sum += FONT_SMALL[idx][i];
        }
        ASSERT_TRUE(sum > 0);
    }
}

static void test_font_digits_range(void)
{
    for (char c = '0'; c <= '9'; c++) {
        int idx = c - SFONT_FIRST;
        int sum = 0;
        for (int i = 0; i < SFONT_H * SFONT_W; i++) {
            sum += FONT_SMALL[idx][i];
        }
        ASSERT_TRUE(sum > 0);
    }
}

static void test_space_is_blank(void)
{
    int idx = ' ' - SFONT_FIRST;
    int sum = 0;
    for (int i = 0; i < SFONT_H * SFONT_W; i++) {
        sum += FONT_SMALL[idx][i];
    }
    ASSERT_EQ(sum, 0);
}

/* --- checkmark sprite --- */

static void test_checkmark_not_empty(void)
{
    int sum = 0;
    for (int i = 0; i < SFONT_H * SFONT_W; i++) {
        sum += SPR_CHECK[i];
    }
    ASSERT_TRUE(sum > 0);
}

int main(void)
{
    printf("[test_sprites]\n");

    RUN(test_text_width_empty);
    RUN(test_text_width_single_char);
    RUN(test_text_width_two_chars);
    RUN(test_text_width_with_spaces);
    RUN(test_draw_text_A);
    RUN(test_draw_text_offset);
    RUN(test_draw_text_lowercase);
    RUN(test_draw_text_inv);
    RUN(test_draw_digit_zero);
    RUN(test_draw_digit_invalid);
    RUN(test_draw_number_zero);
    RUN(test_draw_number_multi_digit);
    RUN(test_font_uppercase_range);
    RUN(test_font_lowercase_range);
    RUN(test_font_digits_range);
    RUN(test_space_is_blank);
    RUN(test_checkmark_not_empty);

    TEST_REPORT();
}
