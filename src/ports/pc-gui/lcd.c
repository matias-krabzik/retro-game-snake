#include "lcd.h"
#include <string.h>

static uint8_t framebuffer[LCD_H][LCD_W];
static int lcd_ox;
static int lcd_oy;
static LcdColors_t lcd_colors;

void lcd_init(int offset_x, int offset_y, LcdColors_t colors)
{
    lcd_ox = offset_x;
    lcd_oy = offset_y;
    lcd_colors = colors;
    lcd_clear();
}

void lcd_set_colors(LcdColors_t colors)
{
    lcd_colors = colors;
}

void lcd_clear(void)
{
    memset(framebuffer, 0, sizeof(framebuffer));
}

void lcd_set_pixel(int x, int y, bool on)
{
    if (x >= 0 && x < LCD_W && y >= 0 && y < LCD_H) {
        framebuffer[y][x] = on ? 1 : 0;
    }
}

void lcd_set_pixel_inv(int x, int y)
{
    if (x >= 0 && x < LCD_W && y >= 0 && y < LCD_H) {
        framebuffer[y][x] ^= 1;
    }
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

void lcd_render(void)
{
    /* Fill entire LCD area with gap color (grid lines between pixels) */
    DrawRectangle(lcd_ox, lcd_oy,
                  LCD_W * PIXEL_CELL, LCD_H * PIXEL_CELL,
                  lcd_colors.gap);

    /* Draw each pixel */
    for (int y = 0; y < LCD_H; y++) {
        for (int x = 0; x < LCD_W; x++) {
            int sx = lcd_ox + x * PIXEL_CELL;
            int sy = lcd_oy + y * PIXEL_CELL;
            Color c = framebuffer[y][x] ? lcd_colors.pixel_on : lcd_colors.pixel_off;
            DrawRectangle(sx, sy, PIXEL_SCALE, PIXEL_SCALE, c);
        }
    }
}
