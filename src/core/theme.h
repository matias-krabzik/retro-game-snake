#ifndef CORE_THEME_H
#define CORE_THEME_H

#include <stdint.h>

/* Platform-independent RGBA color */
typedef struct {
    uint8_t r, g, b, a;
} LcdColor_t;

/* LCD palette: 4 colors needed for Nokia 3310 simulation */
typedef struct {
    LcdColor_t backlight;
    LcdColor_t pixel_on;
    LcdColor_t pixel_off;
    LcdColor_t gap;
} LcdPalette_t;

typedef struct {
    const char *name;
    LcdPalette_t colors;
} PaletteEntry_t;

enum {
    PALETTE_GREEN,
    PALETTE_GREY,
    PALETTE_AMBER,
    PALETTE_COUNT
};

extern const PaletteEntry_t PALETTES[PALETTE_COUNT];

#endif /* CORE_THEME_H */
