#include "lcd.h"
#include <string.h>

static uint8_t framebuffer[LCD_HEIGHT][LCD_WIDTH];
static int clip_x0, clip_y0;
static int clip_x1 = LCD_WIDTH, clip_y1 = LCD_HEIGHT;

const uint8_t (*lcd_get_framebuffer(void))[LCD_WIDTH]
{
    return (const uint8_t (*)[LCD_WIDTH])framebuffer;
}

void lcd_clear(void)
{
    memset(framebuffer, 0, sizeof(framebuffer));
}

void lcd_set_pixel(int x, int y, bool on)
{
    if (x >= clip_x0 && x < clip_x1 && y >= clip_y0 && y < clip_y1) {
        framebuffer[y][x] = on ? 1 : 0;
    }
}

void lcd_set_pixel_inv(int x, int y)
{
    if (x >= clip_x0 && x < clip_x1 && y >= clip_y0 && y < clip_y1) {
        framebuffer[y][x] ^= 1;
    }
}

void lcd_set_clip(int x0, int y0, int x1, int y1)
{
    clip_x0 = x0 < 0 ? 0 : x0;
    clip_y0 = y0 < 0 ? 0 : y0;
    clip_x1 = x1 > LCD_WIDTH ? LCD_WIDTH : x1;
    clip_y1 = y1 > LCD_HEIGHT ? LCD_HEIGHT : y1;
}

void lcd_clear_clip(void)
{
    clip_x0 = 0;
    clip_y0 = 0;
    clip_x1 = LCD_WIDTH;
    clip_y1 = LCD_HEIGHT;
}

void lcd_fill_rect(int x, int y, int w, int h)
{
    for (int row = y; row < y + h; row++) {
        for (int col = x; col < x + w; col++) {
            lcd_set_pixel(col, row, true);
        }
    }
}

void lcd_draw_rect(int x, int y, int w, int h)
{
    lcd_draw_hline(x, y, w);
    lcd_draw_hline(x, y + h - 1, w);
    lcd_draw_vline(x, y, h);
    lcd_draw_vline(x + w - 1, y, h);
}

void lcd_draw_hline(int x, int y, int w)
{
    for (int i = 0; i < w; i++) {
        lcd_set_pixel(x + i, y, true);
    }
}

void lcd_draw_vline(int x, int y, int h)
{
    for (int i = 0; i < h; i++) {
        lcd_set_pixel(x, y + i, true);
    }
}

void lcd_draw_sprite(int x, int y, const uint8_t *data, int w, int h)
{
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            if (data[row * w + col]) {
                lcd_set_pixel(x + col, y + row, true);
            }
        }
    }
}

void lcd_draw_sprite_inv(int x, int y, const uint8_t *data, int w, int h)
{
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            if (data[row * w + col]) {
                lcd_set_pixel(x + col, y + row, false);
            }
        }
    }
}

void lcd_invert_rect(int x, int y, int w, int h)
{
    for (int row = y; row < y + h; row++) {
        for (int col = x; col < x + w; col++) {
            lcd_set_pixel_inv(col, row);
        }
    }
}
