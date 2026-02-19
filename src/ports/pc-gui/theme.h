#ifndef THEME_H
#define THEME_H

#include "lcd.h"
#include <raylib.h>
#include <stdint.h>
#include <stdbool.h>

/* --- LCD Color palette --- */
/* Each palette defines the 4 colors needed for LCD simulation */

typedef struct {
    const char *name;
    Color backlight;   /* base LCD background (window fill) */
    Color pixel_on;    /* active pixel */
    Color pixel_off;   /* ghost pixel (faintly visible off-pixel) */
    Color gap;         /* gap between pixels (LCD mask) */
} LcdPalette_t;

enum {
    PALETTE_GREEN,
    PALETTE_GREY,
    PALETTE_AMBER,
    PALETTE_COUNT
};

static const LcdPalette_t PALETTES[PALETTE_COUNT] = {
    {
        "Green",
        { 199, 240, 216, 255 },  /* backlight: #c7f0d8 */
        {  67,  82,  61, 255 },  /* pixel on:  #43523d */
        { 185, 220, 199, 255 },  /* ghost:     slightly darker than backlight */
        { 170, 204, 182, 255 },  /* gap:       LCD mask between pixels */
    },
    {
        "Grey",
        { 195, 195, 185, 255 },  /* backlight */
        {  40,  40,  40, 255 },  /* pixel on */
        { 182, 182, 172, 255 },  /* ghost */
        { 168, 168, 158, 255 },  /* gap */
    },
    {
        "Amber",
        { 255, 200, 100, 255 },  /* backlight */
        {  90,  40,   0, 255 },  /* pixel on */
        { 240, 185,  90, 255 },  /* ghost */
        { 220, 170,  80, 255 },  /* gap */
    },
};

/* --- Active theme state --- */

typedef struct {
    int palette_idx;
} Theme_t;

/* Build LcdColors_t from a palette index */
static inline LcdColors_t theme_lcd_colors(int palette_idx)
{
    const LcdPalette_t *p = &PALETTES[palette_idx];
    return (LcdColors_t){
        .backlight = p->backlight,
        .pixel_on  = p->pixel_on,
        .pixel_off = p->pixel_off,
        .gap       = p->gap,
    };
}

#endif /* THEME_H */
