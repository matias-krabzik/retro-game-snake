#include "lcd.h"
#include <string.h>

static uint8_t framebuffer[LCD_H][LCD_W];
static int lcd_ox;
static int lcd_oy;
static LcdColors_t lcd_colors;
static int clip_x0, clip_y0;
static int clip_x1 = LCD_W, clip_y1 = LCD_H;

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
    clip_x1 = x1 > LCD_W ? LCD_W : x1;
    clip_y1 = y1 > LCD_H ? LCD_H : y1;
}

void lcd_clear_clip(void)
{
    clip_x0 = 0;
    clip_y0 = 0;
    clip_x1 = LCD_W;
    clip_y1 = LCD_H;
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
