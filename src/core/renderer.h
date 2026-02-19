#ifndef CORE_RENDERER_H
#define CORE_RENDERER_H

#include "game.h"
#include <stdbool.h>
#include <stdint.h>

/* Menu view constants */
#define VIEW_MAIN     0
#define VIEW_COLORS   1
#define VIEW_CREDITS  2
#define VIEW_SOUND    3
#define VIEW_SPEED    4

/* Speed settings */
#define SPEED_MAX     7
#define SPEED_DEFAULT 6

extern const uint32_t SPEED_TICK_MS[SPEED_MAX];

/* Main menu items */
#define MAIN_ITEM_COUNT 6
extern const char *MAIN_ITEMS[MAIN_ITEM_COUNT];

/* Credits lines */
#define CREDITS_COUNT 5
extern const char *CREDITS_LINES[CREDITS_COUNT];

/* Pause menu items */
#define PAUSE_ITEM_COUNT 4
extern const char *PAUSE_ITEMS[PAUSE_ITEM_COUNT];

/* Render game frame to LCD framebuffer.
 * show_snake=false for blink effect on game over. */
void render_game_frame(const Game_t *game, bool show_snake);

/* Render main/submenu to LCD framebuffer.
 * For sound/speed views: level = current level (1..7)
 * For list views: selected, first_visible, scroll_px control highlight/scroll
 * checked_idx = palette index with checkmark (-1 for none) */
void render_menu(int view, int selected, int first_visible,
                 int scroll_px, int checked_idx, int level);

/* Render pause menu overlay to LCD framebuffer */
void render_pause_menu(int selected, int first_visible);

#endif /* CORE_RENDERER_H */
