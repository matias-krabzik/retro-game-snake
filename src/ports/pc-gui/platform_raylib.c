#include "../../platform.h"
#include "theme.h"
#include <raylib.h>

#define CELL_SIZE   16
#define PADDING     20
#define HEADER_H    28
#define PX          (CELL_SIZE / 3)  /* sub-pixel unit for food sprite */

static int window_w;
static int window_h;
static int board_x;
static int board_y;

static Theme_t current_theme = { PALETTE_GREEN, STYLE_SNAKE2 };

/* --- Theme accessors (used by main.c menu) --- */

Theme_t *platform_get_theme(void)
{
    return &current_theme;
}

/* --- Platform interface --- */

int platform_init(const GameConfig_t *config)
{
    window_w = config->board_width  * CELL_SIZE + PADDING * 2;
    window_h = config->board_height * CELL_SIZE + PADDING * 2 + HEADER_H;
    board_x  = PADDING;
    board_y  = PADDING + HEADER_H;

    InitWindow(window_w, window_h, "Snake - Nokia Edition");
    SetTargetFPS(60);
    return 0;
}

void platform_shutdown(void)
{
    CloseWindow();
}

static void draw_food_snake1(int fx, int fy, Color fg)
{
    /* Solid square, slightly inset */
    DrawRectangle(fx + 2, fy + 2, CELL_SIZE - 4, CELL_SIZE - 4, fg);
}

static void draw_food_snake2(int fx, int fy, Color fg)
{
    /* Cross/plus with empty center */
    DrawRectangle(fx + PX, fy,          PX, PX, fg);  /* top    */
    DrawRectangle(fx,      fy + PX,     PX, PX, fg);  /* left   */
    DrawRectangle(fx + PX * 2, fy + PX, PX, PX, fg);  /* right  */
    DrawRectangle(fx + PX, fy + PX * 2, PX, PX, fg);  /* bottom */
}

static void draw_snake_segment(int sx, int sy, SnakeStyle_t style, Color fg)
{
    int gap = (style == STYLE_SNAKE2) ? 1 : 0;
    DrawRectangle(sx + gap, sy + gap,
                  CELL_SIZE - gap * 2, CELL_SIZE - gap * 2, fg);
}

void platform_render(const Game_t *game)
{
    uint16_t w = game->config.board_width;
    uint16_t h = game->config.board_height;
    Color bg = PALETTES[current_theme.palette_idx].bg;
    Color fg = PALETTES[current_theme.palette_idx].fg;
    SnakeStyle_t style = (SnakeStyle_t)current_theme.style_idx;

    BeginDrawing();
    ClearBackground(bg);

    /* Score bar */
    DrawText(TextFormat("Score:%u", game->score),
             board_x, PADDING / 2, 16, fg);

    /* Board border */
    DrawRectangleLines(board_x - 1, board_y - 1,
                       w * CELL_SIZE + 2, h * CELL_SIZE + 2, fg);

    /* Food */
    {
        int fx = board_x + game->food.x * CELL_SIZE;
        int fy = board_y + game->food.y * CELL_SIZE;
        if (style == STYLE_SNAKE1) {
            draw_food_snake1(fx, fy, fg);
        } else {
            draw_food_snake2(fx, fy, fg);
        }
    }

    /* Snake */
    for (uint16_t i = 0; i < game->snake.length; i++) {
        int sx = board_x + game->snake.body[i].x * CELL_SIZE;
        int sy = board_y + game->snake.body[i].y * CELL_SIZE;
        draw_snake_segment(sx, sy, style, fg);
    }

    /* Game over */
    if (game->status == STATE_GAME_OVER) {
        int box_w = 200;
        int box_h = 60;
        int bx = (window_w - box_w) / 2;
        int by = (window_h - box_h) / 2;
        DrawRectangle(bx, by, box_w, box_h, bg);
        DrawRectangleLines(bx, by, box_w, box_h, fg);

        const char *text = "GAME OVER";
        int tw = MeasureText(text, 20);
        DrawText(text, (window_w - tw) / 2, by + 8, 20, fg);

        const char *sub = "R:again  Q:quit  M:menu";
        int sw = MeasureText(sub, 14);
        DrawText(sub, (window_w - sw) / 2, by + 36, 14, fg);
    }

    EndDrawing();
}

void platform_render_menu(const Theme_t *theme, int selected)
{
    Color bg = PALETTES[theme->palette_idx].bg;
    Color fg = PALETTES[theme->palette_idx].fg;

    BeginDrawing();
    ClearBackground(bg);

    /* Title */
    const char *title = "S N A K E";
    int tw = MeasureText(title, 28);
    DrawText(title, (window_w - tw) / 2, 30, 28, fg);

    /* Menu items */
    int y_start = 80;
    int spacing = 30;

    /* Option 0: Palette */
    {
        const char *label = TextFormat("Color:  < %s >",
                                       PALETTES[theme->palette_idx].name);
        int lw = MeasureText(label, 18);
        int x = (window_w - lw) / 2;
        DrawText(label, x, y_start, 18, fg);
        if (selected == 0) {
            DrawText(">", x - 18, y_start, 18, fg);
        }
    }

    /* Option 1: Style */
    {
        const char *label = TextFormat("Style:  < %s >",
                                       STYLE_NAMES[theme->style_idx]);
        int lw = MeasureText(label, 18);
        int x = (window_w - lw) / 2;
        DrawText(label, x, y_start + spacing, 18, fg);
        if (selected == 1) {
            DrawText(">", x - 18, y_start + spacing, 18, fg);
        }
    }

    /* Start hint */
    const char *hint = "ENTER: start  Q: quit";
    int hw = MeasureText(hint, 14);
    DrawText(hint, (window_w - hw) / 2, y_start + spacing * 3, 14, fg);

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
    /* Raylib handles frame timing via SetTargetFPS */
}
