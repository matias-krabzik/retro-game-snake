#ifndef CORE_LCD_H
#define CORE_LCD_H

#include <stdint.h>
#include <stdbool.h>

#define LCD_WIDTH  84
#define LCD_HEIGHT 48

/* Clear framebuffer (all pixels off) */
void lcd_clear(void);

/* Pixel operations */
void lcd_set_pixel(int x, int y, bool on);
void lcd_set_pixel_inv(int x, int y);

/* Drawing primitives (all draw "on" pixels) */
void lcd_fill_rect(int x, int y, int w, int h);
void lcd_draw_rect(int x, int y, int w, int h);
void lcd_draw_hline(int x, int y, int w);
void lcd_draw_vline(int x, int y, int h);

/* Sprite rendering: data is row-major, 1=on 0=off */
void lcd_draw_sprite(int x, int y, const uint8_t *data, int w, int h);
void lcd_draw_sprite_inv(int x, int y, const uint8_t *data, int w, int h);

/* Invert a rectangular region (for Nokia-style selection bars) */
void lcd_invert_rect(int x, int y, int w, int h);

/* Clipping: restrict pixel writes to a sub-region */
void lcd_set_clip(int x0, int y0, int x1, int y1);
void lcd_clear_clip(void);

/* Access the raw framebuffer for presentation by ports */
const uint8_t (*lcd_get_framebuffer(void))[LCD_WIDTH];

#endif /* CORE_LCD_H */
