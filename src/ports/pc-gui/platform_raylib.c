#include "../../platform.h"
#include "audio.h"
#include "icon_data.h"
#include <raylib.h>

/* LCD presentation constants */
#define PIXEL_SCALE  8
#define PIXEL_GAP    1
#define PIXEL_CELL   (PIXEL_SCALE + PIXEL_GAP)
#define BEZEL        30

static int lcd_ox;
static int lcd_oy;

/* Convert core LcdColor_t to Raylib Color */
static Color to_raylib(LcdColor_t c)
{
    return (Color){ c.r, c.g, c.b, c.a };
}

/* --- Platform interface --- */

int platform_init(void)
{
    int w = LCD_WIDTH * PIXEL_CELL + BEZEL * 2;
    int h = LCD_HEIGHT * PIXEL_CELL + BEZEL * 2;
    lcd_ox = BEZEL;
    lcd_oy = BEZEL;

    InitWindow(w, h, "Snake - Nokia 3310");
    SetExitKey(0); /* disable Raylib's default ESC-to-close */
    Image icon = LoadImageFromMemory(".png", public_icon_png,
                                     (int)public_icon_png_len);
    SetWindowIcon(icon);
    UnloadImage(icon);
    SetTargetFPS(60);
    audio_init();
    return 0;
}

void platform_shutdown(void)
{
    audio_shutdown();
    CloseWindow();
}

void platform_present(const uint8_t (*fb)[LCD_WIDTH],
                      const LcdPalette_t *palette)
{
    Color gap = to_raylib(palette->gap);
    Color on  = to_raylib(palette->pixel_on);
    Color off = to_raylib(palette->pixel_off);

    BeginDrawing();
    ClearBackground(gap);

    /* Fill LCD area with gap color (grid lines between pixels) */
    DrawRectangle(lcd_ox, lcd_oy,
                  LCD_WIDTH * PIXEL_CELL, LCD_HEIGHT * PIXEL_CELL, gap);

    /* Draw each pixel */
    for (int y = 0; y < LCD_HEIGHT; y++) {
        for (int x = 0; x < LCD_WIDTH; x++) {
            int sx = lcd_ox + x * PIXEL_CELL;
            int sy = lcd_oy + y * PIXEL_CELL;
            Color c = fb[y][x] ? on : off;
            DrawRectangle(sx, sy, PIXEL_SCALE, PIXEL_SCALE, c);
        }
    }

    EndDrawing();
}

UiInput_t platform_get_input(void)
{
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
        return UI_INPUT_CONFIRM;
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE))
        return UI_INPUT_BACK;
    if (IsKeyPressed(KEY_Q))
        return UI_INPUT_QUIT;
    if (IsKeyPressed(KEY_UP)    || IsKeyPressed(KEY_W))
        return UI_INPUT_UP;
    if (IsKeyPressed(KEY_DOWN)  || IsKeyPressed(KEY_S))
        return UI_INPUT_DOWN;
    if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A))
        return UI_INPUT_LEFT;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        return UI_INPUT_RIGHT;
    return UI_INPUT_NONE;
}

double platform_get_time(void)
{
    return GetTime();
}

bool platform_should_close(void)
{
    return WindowShouldClose();
}

void platform_play_sound(SoundType_t sound)
{
    audio_play(sound);
}

void platform_set_volume(int level)
{
    audio_set_volume(level);
}

void platform_sleep_ms(uint32_t ms)
{
    (void)ms;
}
