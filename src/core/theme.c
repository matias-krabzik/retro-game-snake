#include "theme.h"

const PaletteEntry_t PALETTES[PALETTE_COUNT] = {
    {
        "Green",
        {
            { 156, 189, 108, 255 },  /* backlight */
            {  15,  40,   8, 255 },  /* pixel on */
            { 132, 164,  88, 255 },  /* ghost */
            { 120, 150,  80, 255 },  /* gap */
        }
    },
    {
        "Grey",
        {
            { 180, 185, 175, 255 },  /* backlight */
            {  20,  22,  18, 255 },  /* pixel on */
            { 152, 158, 148, 255 },  /* ghost */
            { 138, 144, 134, 255 },  /* gap */
        }
    },
    {
        "Amber",
        {
            { 230, 168,  50, 255 },  /* backlight */
            {  60,  20,   0, 255 },  /* pixel on */
            { 198, 142,  40, 255 },  /* ghost */
            { 180, 128,  32, 255 },  /* gap */
        }
    },
};
