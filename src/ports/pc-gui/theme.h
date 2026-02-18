#ifndef THEME_H
#define THEME_H

#include <raylib.h>
#include <stdint.h>
#include <stdbool.h>

/* --- Snake visual style --- */

typedef enum {
    STYLE_SNAKE1,   /* Solid square food, uniform body */
    STYLE_SNAKE2,   /* Cross/plus food (empty center), gapped segments */
    STYLE_COUNT
} SnakeStyle_t;

static const char *STYLE_NAMES[STYLE_COUNT] = {
    "Snake I",
    "Snake II"
};

/* --- Color palette --- */

typedef struct {
    const char *name;
    Color bg;
    Color fg;
} Palette_t;

enum {
    PALETTE_GREEN,
    PALETTE_GREY,
    PALETTE_AMBER,
    PALETTE_COUNT
};

static const Palette_t PALETTES[PALETTE_COUNT] = {
    { "Green",  { 199, 207, 161, 255 }, {  43,  63,   9, 255 } },
    { "Grey",   { 182, 182, 170, 255 }, {  32,  32,  32, 255 } },
    { "Amber",  { 255, 183,  77, 255 }, {  80,  32,   0, 255 } },
};

/* --- Active theme state --- */

typedef struct {
    int palette_idx;
    int style_idx;
} Theme_t;

#endif /* THEME_H */
