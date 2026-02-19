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
        { 156, 189, 108, 255 },  /* backlight: bright Nokia green */
        {  15,  40,   8, 255 },  /* pixel on:  deep dark green */
        { 132, 164,  88, 255 },  /* ghost:     darker than backlight */
        { 120, 150,  80, 255 },  /* gap:       LCD mask between pixels */
    },
    {
        "Grey",
        { 180, 185, 175, 255 },  /* backlight: cool grey LCD */
        {  20,  22,  18, 255 },  /* pixel on:  near black */
        { 152, 158, 148, 255 },  /* ghost:     darker than backlight */
        { 138, 144, 134, 255 },  /* gap:       LCD mask */
    },
    {
        "Amber",
        { 230, 168,  50, 255 },  /* backlight: warm amber LCD */
        {  60,  20,   0, 255 },  /* pixel on:  deep brown */
        { 198, 142,  40, 255 },  /* ghost:     darker than backlight */
        { 180, 128,  32, 255 },  /* gap:       LCD mask */
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
