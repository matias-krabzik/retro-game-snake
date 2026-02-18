#include "../../platform.h"
#include "theme.h"
#include "sprites.h"
#include "lcd.h"
#include <raylib.h>
#include <string.h>
#include <stdio.h>

/* Nokia 3310 Snake II layout constants (in LCD pixels) */
#define ARENA_COLS   20
#define ARENA_ROWS    9
#define CELL_PX       4   /* each game cell is 4x4 LCD pixels */

/* LCD layout positions */
#define SCORE_X       1
#define SCORE_Y       0
#define DIVIDER_Y     6
#define ARENA_BOX_Y   8    /* top of arena rectangle */
#define ARENA_X       2    /* content x: 1px border + 1px padding */
#define ARENA_Y      10    /* content y: 1px border + 1px padding */
#define BORDER_B     47

/* Window bezel around the LCD */
#define BEZEL         30

static int window_w;
static int window_h;
static int lcd_offset_x;
static int lcd_offset_y;

static Theme_t current_theme = { PALETTE_GREEN, STYLE_SNAKE2 };

/* --- Theme accessors (used by main.c menu) --- */

Theme_t *platform_get_theme(void)
{
    return &current_theme;
}

void platform_apply_theme(void)
{
    lcd_set_colors(theme_lcd_colors(current_theme.palette_idx));
}

/* --- Sprite selection helpers for Snake II --- */

/* Check if food is ahead of the head, in the same line, within 5 cells */
static bool is_head_eating(const Game_t *game)
{
    const Snake_t *snake = &game->snake;
    if (snake->length < 2) return false;

    Point_t head = snake->body[0];
    Point_t neck = snake->body[1];
    Point_t food = game->food;
    int bw = game->config.board_width;
    int bh = game->config.board_height;

    int dx = head.x - neck.x;
    int dy = head.y - neck.y;
    if (dx > 1) dx = -1; if (dx < -1) dx = 1;
    if (dy > 1) dy = -1; if (dy < -1) dy = 1;

    if (dx > 0 && food.y == head.y) {          /* facing right */
        int dist = (food.x - head.x + bw) % bw;
        return dist >= 1 && dist <= 5;
    } else if (dx < 0 && food.y == head.y) {   /* facing left */
        int dist = (head.x - food.x + bw) % bw;
        return dist >= 1 && dist <= 5;
    } else if (dy > 0 && food.x == head.x) {   /* facing down */
        int dist = (food.y - head.y + bh) % bh;
        return dist >= 1 && dist <= 5;
    } else if (dy < 0 && food.x == head.x) {   /* facing up */
        int dist = (head.y - food.y + bh) % bh;
        return dist >= 1 && dist <= 5;
    }
    return false;
}

/* Determine which sprite to use for a snake segment based on neighbors */
static const uint8_t *get_snake2_sprite(const Game_t *game, uint16_t i)
{
    const Snake_t *snake = &game->snake;
    Point_t cur = snake->body[i];

    /* Head: derive direction from actual position relative to body[1],
       so the sprite only changes on the next game tick (not on key press) */
    if (i == 0) {
        bool eating = is_head_eating(game);
        if (snake->length > 1) {
            Point_t next = snake->body[1];
            int dx = cur.x - next.x;
            int dy = cur.y - next.y;
            if (dx > 1) dx = -1; if (dx < -1) dx = 1;
            if (dy > 1) dy = -1; if (dy < -1) dy = 1;
            if (dx > 0) return eating ? SPR_HEAD_EAT_R : SPR_HEAD_R;
            if (dx < 0) return eating ? SPR_HEAD_EAT_L : SPR_HEAD_L;
            if (dy > 0) return eating ? SPR_HEAD_EAT_D : SPR_HEAD_D;
            if (dy < 0) return eating ? SPR_HEAD_EAT_U : SPR_HEAD_U;
        }
        switch (snake->direction) {
        case DIR_RIGHT: return eating ? SPR_HEAD_EAT_R : SPR_HEAD_R;
        case DIR_LEFT:  return eating ? SPR_HEAD_EAT_L : SPR_HEAD_L;
        case DIR_UP:    return eating ? SPR_HEAD_EAT_U : SPR_HEAD_U;
        case DIR_DOWN:  return eating ? SPR_HEAD_EAT_D : SPR_HEAD_D;
        }
    }

    /* Tail (last segment) */
    if (i == snake->length - 1) {
        Point_t prev = snake->body[i - 1];
        int dx = prev.x - cur.x;
        int dy = prev.y - cur.y;
        if (dx > 0 || dx < -1) return SPR_TAIL_R;
        if (dx < 0 || dx > 1)  return SPR_TAIL_L;
        if (dy > 0 || dy < -1) return SPR_TAIL_D;
        if (dy < 0 || dy > 1)  return SPR_TAIL_U;
        return SPR_TAIL_R;
    }

    /* Middle segment: check prev and next to determine straight or corner */
    Point_t prev = snake->body[i - 1];
    Point_t next = snake->body[i + 1];
    int dpx = prev.x - cur.x;
    int dpy = prev.y - cur.y;
    int dnx = next.x - cur.x;
    int dny = next.y - cur.y;

    if (dpx > 1)  dpx = -1; if (dpx < -1) dpx = 1;
    if (dpy > 1)  dpy = -1; if (dpy < -1) dpy = 1;
    if (dnx > 1)  dnx = -1; if (dnx < -1) dnx = 1;
    if (dny > 1)  dny = -1; if (dny < -1) dny = 1;

    /* Straight segments (use fat variant if segment has food bulge) */
    if ((dpx != 0 && dnx != 0) && dpy == 0 && dny == 0)
        return snake->fat[i] ? SPR_BODY_FAT_H : SPR_STRAIGHT_H;
    if ((dpy != 0 && dny != 0) && dpx == 0 && dnx == 0)
        return snake->fat[i] ? SPR_BODY_FAT_V : SPR_STRAIGHT_V;

    /* Corners (use fat variant if segment has food bulge) */
    int sx = dpx + dnx;
    int sy = dpy + dny;

    if (sx > 0 && sy > 0) return snake->fat[i] ? SPR_CORNER_FAT_RD : SPR_CORNER_RD;
    if (sx > 0 && sy < 0) return snake->fat[i] ? SPR_CORNER_FAT_RU : SPR_CORNER_RU;
    if (sx < 0 && sy > 0) return snake->fat[i] ? SPR_CORNER_FAT_LD : SPR_CORNER_LD;
    if (sx < 0 && sy < 0) return snake->fat[i] ? SPR_CORNER_FAT_LU : SPR_CORNER_LU;

    return SPR_STRAIGHT_H;
}

/* --- Drawing helpers --- */

static void draw_game_frame(const Game_t *game)
{
    SnakeStyle_t style = (SnakeStyle_t)current_theme.style_idx;

    /* Score (number only) */
    lcd_draw_number(SCORE_X, SCORE_Y, game->score);

    /* Divider line */
    lcd_draw_hline(0, DIVIDER_Y, LCD_W);

    /* Arena box (1px border with 1px internal padding) */
    lcd_draw_rect(0, ARENA_BOX_Y, LCD_W, BORDER_B - ARENA_BOX_Y + 1);

    /* Food */
    {
        int fx = ARENA_X + game->food.x * CELL_PX;
        int fy = ARENA_Y + game->food.y * CELL_PX;
        if (style == STYLE_SNAKE1) {
            lcd_draw_sprite(fx, fy, SPR_FOOD_SIMPLE, SPRITE_W, SPRITE_H);
        } else {
            lcd_draw_sprite(fx, fy, SPR_FOOD, SPRITE_W, SPRITE_H);
        }
    }

    /* Snake */
    for (uint16_t i = 0; i < game->snake.length; i++) {
        int sx = ARENA_X + game->snake.body[i].x * CELL_PX;
        int sy = ARENA_Y + game->snake.body[i].y * CELL_PX;

        if (style == STYLE_SNAKE1) {
            lcd_draw_sprite(sx, sy, SPR_BLOCK, SPRITE_W, SPRITE_H);
        } else {
            lcd_draw_sprite(sx, sy, get_snake2_sprite(game, i),
                            SPRITE_W, SPRITE_H);
        }
    }
}

static void draw_game_over_overlay(void)
{
    /* Centered box on the LCD (1px border + 1px padding) */
    int box_w = 60;
    int box_h = 19;
    int bx = (LCD_W - box_w) / 2;
    int by = (LCD_H - box_h) / 2;

    /* Clear box area and draw border */
    for (int row = by; row < by + box_h; row++) {
        for (int col = bx; col < bx + box_w; col++) {
            lcd_set_pixel(col, row, false);
        }
    }
    lcd_draw_rect(bx, by, box_w, box_h);

    /* "GAME OVER" centered (border=1 + padding=1 → offset 2) */
    const char *text = "GAME OVER";
    int tw = lcd_text_width(text);
    lcd_draw_text((LCD_W - tw) / 2, by + 2, text);

    /* Hint */
    const char *hint = "R:. M:. Q:.";
    int hw = lcd_text_width(hint);
    lcd_draw_text((LCD_W - hw) / 2, by + 10, hint);
}

/* --- Platform interface --- */

int platform_init(const GameConfig_t *config)
{
    (void)config;

    window_w = LCD_W * PIXEL_CELL + BEZEL * 2;
    window_h = LCD_H * PIXEL_CELL + BEZEL * 2;
    lcd_offset_x = BEZEL;
    lcd_offset_y = BEZEL;

    InitWindow(window_w, window_h, "Snake - Nokia 3310");
    SetTargetFPS(60);

    lcd_init(lcd_offset_x, lcd_offset_y, theme_lcd_colors(current_theme.palette_idx));
    return 0;
}

void platform_shutdown(void)
{
    CloseWindow();
}

void platform_render(const Game_t *game)
{
    platform_apply_theme();
    lcd_clear();
    draw_game_frame(game);

    if (game->status == STATE_GAME_OVER) {
        draw_game_over_overlay();
    }

    BeginDrawing();
    ClearBackground(PALETTES[current_theme.palette_idx].gap);
    lcd_render();
    EndDrawing();
}

void platform_render_menu(const Theme_t *theme, int selected)
{
    lcd_set_colors(theme_lcd_colors(theme->palette_idx));
    lcd_clear();

    /* Title "SNAKE" centered */
    const char *title = "SNAKE";
    int tw = lcd_text_width(title);
    lcd_draw_text((LCD_W - tw) / 2, 4, title);

    /* Divider under title */
    lcd_draw_hline(4, 14, LCD_W - 8);

    /* Menu items with Nokia-style inverted selection */
    int y = 18;
    int item_h = 10;

    /* Item 0: Color */
    {
        const char *label = PALETTES[theme->palette_idx].name;
        char buf[32];
        snprintf(buf, sizeof(buf), "COLOR: <%s>", label);
        if (selected == 0) {
            lcd_fill_rect(0, y, LCD_W, item_h);
            lcd_draw_text_inv(2, y + 1, buf);
        } else {
            lcd_draw_text(2, y + 1, buf);
        }
    }

    /* Item 1: Style */
    y += item_h;
    {
        const char *label = STYLE_NAMES[theme->style_idx];
        char buf[32];
        snprintf(buf, sizeof(buf), "STYLE: <%s>", label);
        if (selected == 1) {
            lcd_fill_rect(0, y, LCD_W, item_h);
            lcd_draw_text_inv(2, y + 1, buf);
        } else {
            lcd_draw_text(2, y + 1, buf);
        }
    }

    /* Start hint at bottom */
    const char *hint = "ENTER:PLAY  Q:QUIT";
    int hw = lcd_text_width(hint);
    lcd_draw_text((LCD_W - hw) / 2, 40, hint);

    BeginDrawing();
    ClearBackground(PALETTES[theme->palette_idx].gap);
    lcd_render();
    EndDrawing();
}

InputEvent_t platform_get_input(void)
{
    if (WindowShouldClose())               return INPUT_QUIT;
    if (IsKeyPressed(KEY_R))               return INPUT_RESTART;
    if (IsKeyPressed(KEY_Q))               return INPUT_QUIT;
    if (IsKeyPressed(KEY_ESCAPE))          return INPUT_QUIT;
    if (IsKeyPressed(KEY_UP)    || IsKeyPressed(KEY_W)) return INPUT_UP;
    if (IsKeyPressed(KEY_DOWN)  || IsKeyPressed(KEY_S)) return INPUT_DOWN;
    if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A)) return INPUT_LEFT;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) return INPUT_RIGHT;
    return INPUT_NONE;
}

void platform_sleep_ms(uint32_t ms)
{
    (void)ms;
}
