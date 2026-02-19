#include "renderer.h"
#include "lcd.h"
#include "sprites.h"
#include "theme.h"
#include <string.h>

/* Nokia 3310 Snake II layout constants (in LCD pixels) */
#define ARENA_COLS   20
#define ARENA_ROWS    9
#define CELL_PX       4

/* LCD layout positions */
#define SCORE_X       1
#define SCORE_Y       0
#define DIVIDER_Y     6
#define ARENA_BOX_Y   8
#define ARENA_X       2
#define ARENA_Y      10
#define BORDER_B     47

/* Menu layout constants */
#define MENU_ITEM_H   10
#define MENU_PAD_X     3
#define MENU_PAD_Y     2
#define MENU_CHECK_W  (SFONT_W + 1)

/* Scrollbar */
#define SB_W           5
#define SB_X           (LCD_WIDTH - 1 - SB_W)
#define SB_GAP         1
#define CONTENT_RIGHT  (SB_X - SB_GAP)
#define ARROW_BOX_H    4

/* Pause menu */
#define PAUSE_HEADER_H   9
#define PAUSE_MAX_VIS    3

/* Level bar */
#define BAR_COLS    7
#define BAR_COL_W   3
#define BAR_COL_GAP 1
#define BAR_BASE_H  3
#define BAR_STEP_H  1
#define BAR_SIGN_PAD 2

/* --- Data --- */

const uint32_t SPEED_TICK_MS[SPEED_MAX] = {
    1000, 600, 400, 300, 200, 150, 100
};

const char *MAIN_ITEMS[MAIN_ITEM_COUNT] = {
    "New game", "Color", "Speed", "Sound", "Credits", "Exit"
};

const char *CREDITS_LINES[CREDITS_COUNT] = {
    "Retro Snake",
    "by Matias Krabzik",
    "A love letter to",
    "the Nokia 3310",
    "C99 + Raylib",
};

const char *PAUSE_ITEMS[PAUSE_ITEM_COUNT] = {
    "Resume", "Restart", "Menu", "Exit"
};

/* --- Sprite selection helpers --- */

static bool is_target_ahead(Point_t head, int dx, int dy,
                            int tx, int ty, int bw, int bh)
{
    if (dx > 0 && ty == head.y) {
        int dist = (tx - head.x + bw) % bw;
        return dist >= 1 && dist <= 5;
    } else if (dx < 0 && ty == head.y) {
        int dist = (head.x - tx + bw) % bw;
        return dist >= 1 && dist <= 5;
    } else if (dy > 0 && tx == head.x) {
        int dist = (ty - head.y + bh) % bh;
        return dist >= 1 && dist <= 5;
    } else if (dy < 0 && tx == head.x) {
        int dist = (head.y - ty + bh) % bh;
        return dist >= 1 && dist <= 5;
    }
    return false;
}

static bool is_head_eating(const Game_t *game)
{
    const Snake_t *snake = &game->snake;
    if (snake->length < 2) return false;

    Point_t head = snake->body[0];
    Point_t neck = snake->body[1];
    int bw = game->config.board_width;
    int bh = game->config.board_height;

    int dx = head.x - neck.x;
    int dy = head.y - neck.y;
    if (dx > 1) dx = -1; if (dx < -1) dx = 1;
    if (dy > 1) dy = -1; if (dy < -1) dy = 1;

    if (is_target_ahead(head, dx, dy, game->food.x, game->food.y, bw, bh))
        return true;

    if (game->bonus_active) {
        if (is_target_ahead(head, dx, dy, game->bonus.x, game->bonus.y, bw, bh))
            return true;
        if (is_target_ahead(head, dx, dy, game->bonus.x + 1, game->bonus.y, bw, bh))
            return true;
    }

    return false;
}

static const uint8_t *get_snake_sprite(const Game_t *game, uint16_t i)
{
    const Snake_t *snake = &game->snake;
    Point_t cur = snake->body[i];

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

    if ((dpx != 0 && dnx != 0) && dpy == 0 && dny == 0)
        return snake->fat[i] ? SPR_BODY_FAT_H : SPR_STRAIGHT_H;
    if ((dpy != 0 && dny != 0) && dpx == 0 && dnx == 0)
        return snake->fat[i] ? SPR_BODY_FAT_V : SPR_STRAIGHT_V;

    int sx = dpx + dnx;
    int sy = dpy + dny;

    if (sx > 0 && sy > 0) return snake->fat[i] ? SPR_CORNER_FAT_RD : SPR_CORNER_RD;
    if (sx > 0 && sy < 0) return snake->fat[i] ? SPR_CORNER_FAT_RU : SPR_CORNER_RU;
    if (sx < 0 && sy > 0) return snake->fat[i] ? SPR_CORNER_FAT_LD : SPR_CORNER_LD;
    if (sx < 0 && sy < 0) return snake->fat[i] ? SPR_CORNER_FAT_LU : SPR_CORNER_LU;

    return SPR_STRAIGHT_H;
}

/* --- Drawing helpers --- */

static void draw_scrollbar(int items_top, int count, int max_vis, int selected)
{
    int box_top_y = items_top;
    int box_bot_y = LCD_HEIGHT - 1 - ARROW_BOX_H;

    /* Up arrow box */
    lcd_fill_rect(SB_X, box_top_y, SB_W, ARROW_BOX_H);
    lcd_set_pixel(SB_X + 2, box_top_y + 1, false);
    lcd_set_pixel(SB_X + 1, box_top_y + 2, false);
    lcd_set_pixel(SB_X + 2, box_top_y + 2, false);
    lcd_set_pixel(SB_X + 3, box_top_y + 2, false);

    /* Down arrow box */
    lcd_fill_rect(SB_X, box_bot_y, SB_W, ARROW_BOX_H);
    lcd_set_pixel(SB_X + 1, box_bot_y + 1, false);
    lcd_set_pixel(SB_X + 2, box_bot_y + 1, false);
    lcd_set_pixel(SB_X + 3, box_bot_y + 1, false);
    lcd_set_pixel(SB_X + 2, box_bot_y + 2, false);

    if (count > max_vis) {
        int track_y = box_top_y + ARROW_BOX_H + 1;
        int track_h = box_bot_y - 1 - track_y;
        int thumb_h = track_h * max_vis / count;
        if (thumb_h < 4) thumb_h = 4;
        int thumb_y = track_y;
        if (count > 1)
            thumb_y = track_y + (track_h - thumb_h) * selected
                      / (count - 1);
        lcd_fill_rect(SB_X + 1, thumb_y, SB_W - 2, thumb_h);
    }
}

static void draw_level_bar(int cx, int by, int level)
{
    int cols_w = BAR_COLS * BAR_COL_W + (BAR_COLS - 1) * BAR_COL_GAP;
    int total_w = SFONT_W + BAR_SIGN_PAD + cols_w + BAR_SIGN_PAD + SFONT_W;
    int x = cx - total_w / 2;

    lcd_draw_text(x, by - SFONT_H + 1, "-");
    x += SFONT_W + BAR_SIGN_PAD;

    for (int i = 0; i < BAR_COLS; i++) {
        int col_h = BAR_BASE_H + i * BAR_STEP_H;
        int col_x = x + i * (BAR_COL_W + BAR_COL_GAP);
        int col_y = by - col_h + 1;
        if (i < level) {
            lcd_fill_rect(col_x, col_y, BAR_COL_W, col_h);
        } else {
            lcd_draw_rect(col_x, col_y, BAR_COL_W, col_h);
        }
    }

    x += cols_w + BAR_SIGN_PAD;
    lcd_draw_text(x, by - SFONT_H + 1, "+");
}

/* --- Public API --- */

void render_game_frame(const Game_t *game, bool show_snake)
{
    /* Score: 4-digit with leading zeros */
    {
        uint32_t s = game->score;
        int sx = SCORE_X;
        lcd_draw_digit(sx, SCORE_Y, (int)(s / 1000) % 10); sx += DIGIT_W + 1;
        lcd_draw_digit(sx, SCORE_Y, (int)(s / 100) % 10);  sx += DIGIT_W + 1;
        lcd_draw_digit(sx, SCORE_Y, (int)(s / 10) % 10);   sx += DIGIT_W + 1;
        lcd_draw_digit(sx, SCORE_Y, (int)(s % 10));
    }

    /* Bonus countdown */
    if (game->bonus_active) {
        int countdown_w = BONUS_W + 1 + DIGIT_W + 1 + DIGIT_W;
        int cx = LCD_WIDTH - countdown_w - 1;
        int cy = SCORE_Y;
        lcd_draw_sprite(cx, cy, SPR_BONUSES[game->bonus_sprite], BONUS_W, BONUS_H);
        cx += BONUS_W + 1;
        lcd_draw_digit(cx, cy, game->bonus_steps / 10);
        cx += DIGIT_W + 1;
        lcd_draw_digit(cx, cy, game->bonus_steps % 10);
    }

    /* Divider line */
    lcd_draw_hline(0, DIVIDER_Y, LCD_WIDTH);

    /* Arena box */
    lcd_draw_rect(0, ARENA_BOX_Y, LCD_WIDTH, BORDER_B - ARENA_BOX_Y + 1);

    /* Food */
    {
        int fx = ARENA_X + game->food.x * CELL_PX;
        int fy = ARENA_Y + game->food.y * CELL_PX;
        lcd_draw_sprite(fx, fy, SPR_FOOD, SPRITE_W, SPRITE_H);
    }

    /* Bonus */
    if (game->bonus_active) {
        int bx = ARENA_X + game->bonus.x * CELL_PX;
        int by = ARENA_Y + game->bonus.y * CELL_PX;
        lcd_draw_sprite(bx, by, SPR_BONUSES[game->bonus_sprite], BONUS_W, BONUS_H);
    }

    /* Snake */
    if (show_snake) {
        for (uint16_t i = 0; i < game->snake.length; i++) {
            int sx = ARENA_X + game->snake.body[i].x * CELL_PX;
            int sy = ARENA_Y + game->snake.body[i].y * CELL_PX;
            lcd_draw_sprite(sx, sy, get_snake_sprite(game, i),
                            SPRITE_W, SPRITE_H);
        }
    }
}

void render_menu(int view, int selected, int first_visible,
                 int scroll_px, int checked_idx, int level)
{
    lcd_clear();

    /* Sound / Speed view: level bar with title */
    if (view == VIEW_SOUND || view == VIEW_SPEED) {
        const char *title = (view == VIEW_SOUND) ? "Sound" : "Speed";

        lcd_draw_rect(0, 0, LCD_WIDTH, LCD_HEIGHT);
        lcd_draw_text((LCD_WIDTH - lcd_text_width(title)) / 2, 4, title);
        lcd_draw_hline(1, 12, LCD_WIDTH - 2);
        draw_level_bar(LCD_WIDTH / 2, 32, level);

        const char *hint = "< >";
        lcd_draw_text((LCD_WIDTH - lcd_text_width(hint)) / 2,
                      LCD_HEIGHT - SFONT_H - 3, hint);
        return;
    }

    /* Border */
    lcd_draw_rect(0, 0, LCD_WIDTH, LCD_HEIGHT);

    /* Header for main view */
    bool has_header = (view == VIEW_MAIN);
    int items_top = 1;
    if (has_header) {
        lcd_fill_rect(1, 1, LCD_WIDTH - 2, PAUSE_HEADER_H);
        const char *htitle = "Snake II";
        int htw = lcd_text_width(htitle);
        lcd_draw_text_inv((LCD_WIDTH - htw) / 2, 1, htitle);
        items_top = 1 + PAUSE_HEADER_H;
    }

    /* Determine items */
    const char *items[16];
    int count = 0;

    switch (view) {
    case VIEW_MAIN:
        for (int i = 0; i < MAIN_ITEM_COUNT; i++) items[i] = MAIN_ITEMS[i];
        count = MAIN_ITEM_COUNT;
        break;
    case VIEW_COLORS:
        for (int i = 0; i < PALETTE_COUNT; i++) items[i] = PALETTES[i].name;
        count = PALETTE_COUNT;
        break;
    case VIEW_CREDITS:
        for (int i = 0; i < CREDITS_COUNT; i++) items[i] = CREDITS_LINES[i];
        count = CREDITS_COUNT;
        break;
    }

    bool is_sub = (view == VIEW_COLORS);
    int avail_h = LCD_HEIGHT - 1 - items_top;
    int max_fit = avail_h / MENU_ITEM_H;
    int max_vis = count < max_fit ? count : max_fit;

    /* Draw visible items */
    for (int vi = 0; vi < max_vis; vi++) {
        int i = first_visible + vi;
        if (i >= count) break;

        int iy = items_top + vi * MENU_ITEM_H;
        int tx = MENU_PAD_X + (is_sub ? MENU_CHECK_W : 0);
        int ty = iy + MENU_PAD_Y;

        if (i == selected) {
            lcd_fill_rect(2, iy + 1, CONTENT_RIGHT - 2, MENU_ITEM_H - 1);

            if (is_sub && i == checked_idx) {
                lcd_draw_sprite_inv(MENU_PAD_X, ty, SPR_CHECK, SFONT_W, SFONT_H);
            }

            lcd_set_clip(tx, iy, CONTENT_RIGHT, iy + MENU_ITEM_H);
            lcd_draw_text_inv(tx - scroll_px, ty, items[i]);
            lcd_clear_clip();
        } else {
            if (is_sub && i == checked_idx) {
                lcd_draw_sprite(MENU_PAD_X, ty, SPR_CHECK, SFONT_W, SFONT_H);
            }

            lcd_set_clip(tx, iy, CONTENT_RIGHT, iy + MENU_ITEM_H);
            lcd_draw_text(tx, ty, items[i]);
            lcd_clear_clip();
        }
    }

    /* Scrollbar */
    draw_scrollbar(items_top, count, max_vis, selected);
}

void render_pause_menu(int selected, int first_visible)
{
    lcd_clear();

    /* Border */
    lcd_draw_rect(0, 0, LCD_WIDTH, LCD_HEIGHT);

    /* Header */
    lcd_fill_rect(1, 1, LCD_WIDTH - 2, PAUSE_HEADER_H);
    {
        const char *title = "PAUSED";
        int tw = lcd_text_width(title);
        lcd_draw_text_inv((LCD_WIDTH - tw) / 2, 1, title);
    }

    /* Items */
    int items_y = 1 + PAUSE_HEADER_H;
    int max_vis = PAUSE_ITEM_COUNT < PAUSE_MAX_VIS
                  ? PAUSE_ITEM_COUNT : PAUSE_MAX_VIS;

    for (int vi = 0; vi < max_vis; vi++) {
        int i = first_visible + vi;
        if (i >= PAUSE_ITEM_COUNT) break;

        int iy = items_y + vi * MENU_ITEM_H;
        int tx = MENU_PAD_X;
        int ty = iy + MENU_PAD_Y;

        if (i == selected) {
            lcd_fill_rect(2, iy + 1, CONTENT_RIGHT - 2, MENU_ITEM_H - 1);
            lcd_set_clip(tx, iy, CONTENT_RIGHT, iy + MENU_ITEM_H);
            lcd_draw_text_inv(tx, ty, PAUSE_ITEMS[i]);
            lcd_clear_clip();
        } else {
            lcd_set_clip(tx, iy, CONTENT_RIGHT, iy + MENU_ITEM_H);
            lcd_draw_text(tx, ty, PAUSE_ITEMS[i]);
            lcd_clear_clip();
        }
    }

    /* Scrollbar */
    draw_scrollbar(items_y, PAUSE_ITEM_COUNT, max_vis, selected);
}
