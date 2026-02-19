/* Mock raylib header for testing LCD/sprites without the real library */
#ifndef RAYLIB_H
#define RAYLIB_H

typedef struct Color {
    unsigned char r, g, b, a;
} Color;

/* Stubs for functions called by lcd.c and audio.c */
static inline void DrawRectangle(int x, int y, int w, int h, Color c)
{
    (void)x; (void)y; (void)w; (void)h; (void)c;
}

static inline void SetMasterVolume(float v)
{
    (void)v;
}

#endif /* RAYLIB_H */
