#include "../../platform.h"
#include "theme.h"
#include "sprites.h"
#include "lcd.h"
#include "audio.h"
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

static void draw_game_frame(const Game_t *game, bool show_snake)
{
    SnakeStyle_t style = (SnakeStyle_t)current_theme.style_idx;

    /* Score (number only) */
    lcd_draw_number(SCORE_X, SCORE_Y, game->score);

    /* Bonus countdown in score bar (right-aligned): [sprite 8x4][1px][digit][digit] */
    if (game->bonus_active) {
        int countdown_w = BONUS_W + 1 + DIGIT_W + 1 + DIGIT_W; /* 8+1+3+1+3 = 16 */
        int cx = LCD_W - countdown_w - 1;
        int cy = SCORE_Y;
        lcd_draw_sprite(cx, cy, SPR_BONUSES[game->bonus_sprite], BONUS_W, BONUS_H);
        cx += BONUS_W + 1;
        lcd_draw_digit(cx, cy, game->bonus_steps / 10);
        cx += DIGIT_W + 1;
        lcd_draw_digit(cx, cy, game->bonus_steps % 10);
    }

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

    /* Bonus (8x4 sprite, 2 cells wide) */
    if (game->bonus_active) {
        int bx = ARENA_X + game->bonus.x * CELL_PX;
        int by = ARENA_Y + game->bonus.y * CELL_PX;
        lcd_draw_sprite(bx, by, SPR_BONUSES[game->bonus_sprite], BONUS_W, BONUS_H);
    }

    /* Snake (hidden when blinking off) */
    if (show_snake) {
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
}

static void draw_pause_overlay(void)
{
    /* Centered modal box on the LCD */
    int box_w = 78;
    int box_h = 27;
    int bx = (LCD_W - box_w) / 2;
    int by = (LCD_H - box_h) / 2;

    /* Clear box area and draw border */
    for (int row = by; row < by + box_h; row++) {
        for (int col = bx; col < bx + box_w; col++) {
            lcd_set_pixel(col, row, false);
        }
    }
    lcd_draw_rect(bx, by, box_w, box_h);

    /* "PAUSED" centered */
    const char *t1 = "PAUSED";
    lcd_draw_text((LCD_W - lcd_text_width(t1)) / 2, by + 2, t1);

    /* Options */
    const char *t2 = "ENTER:PLAY";
    lcd_draw_text((LCD_W - lcd_text_width(t2)) / 2, by + 10, t2);

    const char *t3 = "R:RST M:MENU";
    lcd_draw_text((LCD_W - lcd_text_width(t3)) / 2, by + 18, t3);
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
    audio_init();
    return 0;
}

void platform_shutdown(void)
{
    audio_shutdown();
    CloseWindow();
}

void platform_render(const Game_t *game)
{
    platform_apply_theme();
    lcd_clear();

    /* On game over, snake blinks: 1s visible, 1s hidden */
    bool show_snake = true;
    if (game->status == STATE_GAME_OVER) {
        show_snake = ((int)GetTime() % 2) == 0;
    }

    draw_game_frame(game, show_snake);

    BeginDrawing();
    ClearBackground(PALETTES[current_theme.palette_idx].gap);
    lcd_render();
    EndDrawing();
}

void platform_render_paused(const Game_t *game)
{
    platform_apply_theme();
    lcd_clear();
    draw_game_frame(game, true);
    draw_pause_overlay();

    BeginDrawing();
    ClearBackground(PALETTES[current_theme.palette_idx].gap);
    lcd_render();
    EndDrawing();
}

/* Menu views */
#define MENU_VIEW_MAIN     0
#define MENU_VIEW_COLORS   1
#define MENU_VIEW_VERSIONS 2
#define MENU_VIEW_CREDITS  3

#define MENU_ITEM_H   10
#define MENU_PAD_X     3
#define MENU_PAD_Y     2
#define MENU_CHECK_W  (SFONT_W + 1)   /* 6px: checkmark + 1px gap */
#define MAX_VISIBLE_ITEMS  4          /* floor((LCD_H - 2) / MENU_ITEM_H) */

/* Scrollbar: filled arrow boxes with white triangles, thumb with margins */
#define SB_W           5
#define SB_X           (LCD_W - 1 - SB_W)  /* 78 */
#define SB_GAP         1
#define CONTENT_RIGHT  (SB_X - SB_GAP)     /* 77 */
#define ARROW_BOX_H    4                    /* 1px pad + 2px triangle + 1px pad */

static const char *MAIN_ITEMS[] = {
    "New game", "Color", "Version", "Credits", "Exit"
};
#define MAIN_ITEM_COUNT 5

static const char *CREDITS_LINES[] = {
    "Retro Snake",
    "by Matias Krabzik",
    "A love letter to",
    "the Nokia 3310",
    "C99 + Raylib",
};
#define CREDITS_COUNT 5

void platform_render_menu(int view, int selected, int first_visible,
                          int scroll_px, const Theme_t *theme)
{
    lcd_set_colors(theme_lcd_colors(theme->palette_idx));
    lcd_clear();

    /* Border around entire LCD */
    lcd_draw_rect(0, 0, LCD_W, LCD_H);

    /* Determine items and checked index for current view */
    const char *items[16];
    int count = 0;
    int checked = -1;

    switch (view) {
    case MENU_VIEW_MAIN:
        for (int i = 0; i < MAIN_ITEM_COUNT; i++) items[i] = MAIN_ITEMS[i];
        count = MAIN_ITEM_COUNT;
        break;
    case MENU_VIEW_COLORS:
        for (int i = 0; i < PALETTE_COUNT; i++) items[i] = PALETTES[i].name;
        count = PALETTE_COUNT;
        checked = theme->palette_idx;
        break;
    case MENU_VIEW_VERSIONS:
        for (int i = 0; i < STYLE_COUNT; i++) items[i] = STYLE_NAMES[i];
        count = STYLE_COUNT;
        checked = theme->style_idx;
        break;
    case MENU_VIEW_CREDITS:
        for (int i = 0; i < CREDITS_COUNT; i++) items[i] = CREDITS_LINES[i];
        count = CREDITS_COUNT;
        break;
    }

    bool is_sub = (view == MENU_VIEW_COLORS || view == MENU_VIEW_VERSIONS);
    int max_vis = count < MAX_VISIBLE_ITEMS ? count : MAX_VISIBLE_ITEMS;

    /* Draw visible items */
    for (int vi = 0; vi < max_vis; vi++) {
        int i = first_visible + vi;
        if (i >= count) break;

        int iy = 1 + vi * MENU_ITEM_H;
        int tx = MENU_PAD_X + (is_sub ? MENU_CHECK_W : 0);
        int ty = iy + MENU_PAD_Y;

        if (i == selected) {
            /* Highlighted item: inverted bar with 1px margin on left, right, top */
            lcd_fill_rect(2, iy + 1, CONTENT_RIGHT - 2, MENU_ITEM_H - 1);

            if (is_sub && i == checked) {
                lcd_draw_sprite_inv(MENU_PAD_X, ty, SPR_CHECK, SFONT_W, SFONT_H);
            }

            /* Clip text to stay inside content area */
            lcd_set_clip(tx, iy, CONTENT_RIGHT, iy + MENU_ITEM_H);
            lcd_draw_text_inv(tx - scroll_px, ty, items[i]);
            lcd_clear_clip();
        } else {
            /* Normal item */
            if (is_sub && i == checked) {
                lcd_draw_sprite(MENU_PAD_X, ty, SPR_CHECK, SFONT_W, SFONT_H);
            }

            lcd_set_clip(tx, iy, CONTENT_RIGHT, iy + MENU_ITEM_H);
            lcd_draw_text(tx, ty, items[i]);
            lcd_clear_clip();
        }
    }

    /* Scrollbar: filled boxes with white triangles, thumb between them */
    {
        int box_top_y = 1;
        int box_bot_y = LCD_H - 1 - ARROW_BOX_H;  /* 43 */

        /* Up arrow box: filled rect, then carve white triangle */
        lcd_fill_rect(SB_X, box_top_y, SB_W, ARROW_BOX_H);
        lcd_set_pixel(SB_X + 2, box_top_y + 1, false);  /* tip */
        lcd_set_pixel(SB_X + 1, box_top_y + 2, false);  /* base L */
        lcd_set_pixel(SB_X + 2, box_top_y + 2, false);  /* base C */
        lcd_set_pixel(SB_X + 3, box_top_y + 2, false);  /* base R */

        /* Down arrow box: filled rect, then carve white triangle */
        lcd_fill_rect(SB_X, box_bot_y, SB_W, ARROW_BOX_H);
        lcd_set_pixel(SB_X + 1, box_bot_y + 1, false);  /* base L */
        lcd_set_pixel(SB_X + 2, box_bot_y + 1, false);  /* base C */
        lcd_set_pixel(SB_X + 3, box_bot_y + 1, false);  /* base R */
        lcd_set_pixel(SB_X + 2, box_bot_y + 2, false);  /* tip */

        if (count > MAX_VISIBLE_ITEMS) {
            /* Active: thumb with 1px side margins, between arrow boxes */
            int track_y = box_top_y + ARROW_BOX_H + 1;  /* 6 */
            int track_h = box_bot_y - 1 - track_y;      /* 36 */
            int thumb_h = track_h * MAX_VISIBLE_ITEMS / count;
            if (thumb_h < 4) thumb_h = 4;
            int thumb_y = track_y;
            if (count > 1)
                thumb_y = track_y + (track_h - thumb_h) * selected
                          / (count - 1);
            lcd_fill_rect(SB_X + 1, thumb_y, SB_W - 2, thumb_h);
        }
    }

    BeginDrawing();
    ClearBackground(PALETTES[theme->palette_idx].gap);
    lcd_render();
    EndDrawing();
}

InputEvent_t platform_get_input(void)
{
    if (WindowShouldClose())               return INPUT_QUIT;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) return INPUT_PAUSE;
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
