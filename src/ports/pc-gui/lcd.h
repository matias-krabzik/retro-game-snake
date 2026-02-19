#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <raylib.h>

#define LCD_W         84
#define LCD_H         48
#define PIXEL_SCALE   8
#define PIXEL_GAP     1
#define PIXEL_CELL    (PIXEL_SCALE + PIXEL_GAP)

/* LCD color set */
typedef struct {
    Color backlight;   /* base LCD background color */
    Color pixel_on;    /* active pixel color */
    Color pixel_off;   /* ghost pixel color (subtle hint of grid) */
    Color gap;         /* color between pixels (LCD mask) */
} LcdColors_t;

/* Initialize LCD rendering (call after InitWindow) */
void lcd_init(int offset_x, int offset_y, LcdColors_t colors);

/* Set LCD color palette at runtime */
void lcd_set_colors(LcdColors_t colors);

/* Clear framebuffer (all pixels off) */
void lcd_clear(void);

/* Pixel operations */
void lcd_set_pixel(int x, int y, bool on);
void lcd_set_pixel_inv(int x, int y);  /* invert pixel */

/* Drawing primitives (all draw "on" pixels) */
void lcd_fill_rect(int x, int y, int w, int h);
void lcd_draw_rect(int x, int y, int w, int h);  /* outline only */
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

/* Render the framebuffer to screen via Raylib */
void lcd_render(void);

#endif /* LCD_DISPLAY_H */
