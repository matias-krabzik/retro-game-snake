#include <stdio.h>
#include <string.h>
#include "../../src/core/renderer.h"
#include "../../src/core/lcd.h"
#include "../../src/core/theme.h"
#include "../../src/core/sprites.h"

/* ---- Minimal test harness ---- */

static int tests_run, tests_passed;

#define RUN(fn)  do { \
    printf("  %-46s", #fn); \
    fn(); \
    printf("OK\n"); \
    tests_passed++; tests_run++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL  %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        tests_run++; return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        printf("FAIL  %s:%d: %d != %d\n", __FILE__, __LINE__, (int)(a), (int)(b)); \
        tests_run++; return; \
    } \
} while(0)

/* Helper: check if any pixel is on in a rectangular region */
static int region_has_pixels(int x, int y, int w, int h)
{
    const uint8_t (*fb)[LCD_WIDTH] = lcd_get_framebuffer();
    for (int row = y; row < y + h && row < LCD_HEIGHT; row++) {
        for (int col = x; col < x + w && col < LCD_WIDTH; col++) {
            if (row >= 0 && col >= 0 && fb[row][col])
                return 1;
        }
    }
    return 0;
}

/* Helper: count lit pixels in a region */
static int count_pixels(int x, int y, int w, int h)
{
    const uint8_t (*fb)[LCD_WIDTH] = lcd_get_framebuffer();
    int count = 0;
    for (int row = y; row < y + h && row < LCD_HEIGHT; row++) {
        for (int col = x; col < x + w && col < LCD_WIDTH; col++) {
            if (row >= 0 && col >= 0 && fb[row][col])
                count++;
        }
    }
    return count;
}

/* Helper: count total lit pixels */
static int count_all_pixels(void)
{
    return count_pixels(0, 0, LCD_WIDTH, LCD_HEIGHT);
}

/* Helper: create a minimal game state */
static Game_t make_game(void)
{
    GameConfig_t config = {
        .board_width = 20,
        .board_height = 9,
        .initial_length = 3,
        .tick_ms = 150
    };
    Game_t game;
    game_init(&game, config, 42);
    return game;
}

/* ---- Data tests ---- */

static void test_speed_tick_ms_length(void)
{
    /* SPEED_TICK_MS has SPEED_MAX entries, all positive */
    for (int i = 0; i < SPEED_MAX; i++) {
        ASSERT(SPEED_TICK_MS[i] > 0);
    }
}

static void test_speed_tick_ms_decreasing(void)
{
    /* Tick ms should decrease (faster speed = shorter interval) */
    for (int i = 1; i < SPEED_MAX; i++) {
        ASSERT(SPEED_TICK_MS[i] < SPEED_TICK_MS[i - 1]);
    }
}

static void test_main_items_count(void)
{
    ASSERT_EQ(MAIN_ITEM_COUNT, 6);
    for (int i = 0; i < MAIN_ITEM_COUNT; i++) {
        ASSERT(MAIN_ITEMS[i] != NULL);
        ASSERT(MAIN_ITEMS[i][0] != '\0');
    }
}

static void test_credits_count(void)
{
    ASSERT_EQ(CREDITS_COUNT, 5);
    for (int i = 0; i < CREDITS_COUNT; i++) {
        ASSERT(CREDITS_LINES[i] != NULL);
        ASSERT(CREDITS_LINES[i][0] != '\0');
    }
}

static void test_pause_items_count(void)
{
    ASSERT_EQ(PAUSE_ITEM_COUNT, 4);
    for (int i = 0; i < PAUSE_ITEM_COUNT; i++) {
        ASSERT(PAUSE_ITEMS[i] != NULL);
        ASSERT(PAUSE_ITEMS[i][0] != '\0');
    }
}

/* ---- render_game_frame tests ---- */

static void test_game_frame_draws_pixels(void)
{
    lcd_clear();
    Game_t game = make_game();
    render_game_frame(&game, true);
    ASSERT(count_all_pixels() > 0);
}

static void test_game_frame_draws_score(void)
{
    lcd_clear();
    Game_t game = make_game();
    render_game_frame(&game, true);
    /* Score area: top-left, first few pixels should be lit (digit 0) */
    ASSERT(region_has_pixels(1, 0, 20, 6));
}

static void test_game_frame_draws_divider(void)
{
    lcd_clear();
    Game_t game = make_game();
    render_game_frame(&game, true);
    /* Divider at y=6, full width */
    const uint8_t (*fb)[LCD_WIDTH] = lcd_get_framebuffer();
    int lit = 0;
    for (int x = 0; x < LCD_WIDTH; x++) {
        if (fb[6][x]) lit++;
    }
    ASSERT_EQ(lit, LCD_WIDTH);
}

static void test_game_frame_draws_arena_border(void)
{
    lcd_clear();
    Game_t game = make_game();
    render_game_frame(&game, true);
    /* Arena box: top border at y=8 should have lit pixels */
    ASSERT(region_has_pixels(0, 8, LCD_WIDTH, 1));
    /* Bottom border at y=47 */
    ASSERT(region_has_pixels(0, 47, LCD_WIDTH, 1));
}

static void test_game_frame_draws_snake(void)
{
    lcd_clear();
    Game_t game = make_game();

    /* Render with snake visible */
    render_game_frame(&game, true);
    int with_snake = count_all_pixels();

    /* Render without snake (blink off) */
    lcd_clear();
    render_game_frame(&game, false);
    int without_snake = count_all_pixels();

    /* With snake should have more pixels */
    ASSERT(with_snake > without_snake);
}

static void test_game_frame_draws_food(void)
{
    lcd_clear();
    Game_t game = make_game();
    render_game_frame(&game, true);
    /* Food is within the arena area (starting at x=2, y=10) */
    int fx = 2 + game.food.x * 4;
    int fy = 10 + game.food.y * 4;
    ASSERT(region_has_pixels(fx, fy, 4, 4));
}

static void test_game_frame_bonus_countdown(void)
{
    lcd_clear();
    Game_t game = make_game();

    /* No bonus: render and note pixel count in top-right */
    render_game_frame(&game, true);
    int no_bonus = count_pixels(60, 0, 24, 6);

    /* Activate bonus and re-render */
    game.bonus_active = true;
    game.bonus.x = 5;
    game.bonus.y = 5;
    game.bonus_sprite = 0;
    game.bonus_steps = 15;
    lcd_clear();
    render_game_frame(&game, true);
    int with_bonus = count_pixels(60, 0, 24, 6);

    /* Bonus countdown should add pixels in top-right area */
    ASSERT(with_bonus > no_bonus);
}

/* ---- render_menu tests ---- */

static void test_menu_main_draws_border(void)
{
    render_menu(VIEW_MAIN, 0, 0, 0, -1, 0);
    /* Border: top row should be lit */
    const uint8_t (*fb)[LCD_WIDTH] = lcd_get_framebuffer();
    int lit = 0;
    for (int x = 0; x < LCD_WIDTH; x++) {
        if (fb[0][x]) lit++;
    }
    ASSERT_EQ(lit, LCD_WIDTH);
}

static void test_menu_main_has_header(void)
{
    render_menu(VIEW_MAIN, 0, 0, 0, -1, 0);
    /* Header: "Snake II" inverted text in top area (rows 1..9) */
    /* The header fill should produce lots of lit pixels */
    int header_pixels = count_pixels(1, 1, LCD_WIDTH - 2, 9);
    ASSERT(header_pixels > 100);
}

static void test_menu_main_highlights_selected(void)
{
    /* Render with item 0 selected */
    render_menu(VIEW_MAIN, 0, 0, 0, -1, 0);
    int sel0_pixels = count_all_pixels();

    /* Render with item 1 selected - pixel count should differ */
    render_menu(VIEW_MAIN, 1, 0, 0, -1, 0);
    int sel1_pixels = count_all_pixels();

    /* Different selections produce different framebuffers */
    ASSERT(sel0_pixels != sel1_pixels);
}

static void test_menu_colors_shows_items(void)
{
    render_menu(VIEW_COLORS, 0, 0, 0, 0, 0);
    /* Should have visible content */
    ASSERT(count_all_pixels() > 0);
}

static void test_menu_colors_checkmark(void)
{
    /* Render with checked_idx=0 (first item checked) */
    render_menu(VIEW_COLORS, 1, 0, 0, 0, 0);
    int with_check = count_all_pixels();

    /* Render with checked_idx=-1 (no checkmark) */
    render_menu(VIEW_COLORS, 1, 0, 0, -1, 0);
    int without_check = count_all_pixels();

    /* The checkmark should add pixels */
    ASSERT(with_check != without_check);
}

static void test_menu_sound_view(void)
{
    render_menu(VIEW_SOUND, 0, 0, 0, -1, 5);
    /* Should draw level bar and border */
    ASSERT(count_all_pixels() > 0);
    /* "Sound" title should be visible */
    ASSERT(region_has_pixels(20, 0, 40, 12));
}

static void test_menu_speed_view(void)
{
    render_menu(VIEW_SPEED, 0, 0, 0, -1, 3);
    /* Should draw level bar */
    ASSERT(count_all_pixels() > 0);
}

static void test_menu_level_bar_varies(void)
{
    /* Level 1 vs level 7 should produce different pixel counts */
    render_menu(VIEW_SOUND, 0, 0, 0, -1, 1);
    int low = count_all_pixels();

    render_menu(VIEW_SOUND, 0, 0, 0, -1, 7);
    int high = count_all_pixels();

    ASSERT(high > low);
}

static void test_menu_credits_view(void)
{
    render_menu(VIEW_CREDITS, 0, 0, 0, -1, 0);
    ASSERT(count_all_pixels() > 0);
}

/* ---- render_pause_menu tests ---- */

static void test_pause_menu_draws_header(void)
{
    render_pause_menu(0, 0);
    /* "PAUSED" header: inverted area rows 1..9 */
    int header_pixels = count_pixels(1, 1, LCD_WIDTH - 2, 9);
    ASSERT(header_pixels > 100);
}

static void test_pause_menu_shows_items(void)
{
    render_pause_menu(0, 0);
    /* Items area below header (y=10+) should have content */
    ASSERT(region_has_pixels(3, 10, 60, 30));
}

static void test_pause_menu_highlight_changes(void)
{
    render_pause_menu(0, 0);
    int sel0 = count_all_pixels();

    render_pause_menu(1, 0);
    int sel1 = count_all_pixels();

    /* Different selection = different framebuffer */
    ASSERT(sel0 != sel1);
}

static void test_pause_menu_scrollbar(void)
{
    render_pause_menu(0, 0);
    /* Scrollbar is at right edge (x=78..82, full height) */
    ASSERT(region_has_pixels(78, 0, 5, LCD_HEIGHT));
}

/* ---- main ---- */

int main(void)
{
    printf("[test_renderer]\n");

    /* Data tests */
    RUN(test_speed_tick_ms_length);
    RUN(test_speed_tick_ms_decreasing);
    RUN(test_main_items_count);
    RUN(test_credits_count);
    RUN(test_pause_items_count);

    /* Game frame rendering */
    RUN(test_game_frame_draws_pixels);
    RUN(test_game_frame_draws_score);
    RUN(test_game_frame_draws_divider);
    RUN(test_game_frame_draws_arena_border);
    RUN(test_game_frame_draws_snake);
    RUN(test_game_frame_draws_food);
    RUN(test_game_frame_bonus_countdown);

    /* Menu rendering */
    RUN(test_menu_main_draws_border);
    RUN(test_menu_main_has_header);
    RUN(test_menu_main_highlights_selected);
    RUN(test_menu_colors_shows_items);
    RUN(test_menu_colors_checkmark);
    RUN(test_menu_sound_view);
    RUN(test_menu_speed_view);
    RUN(test_menu_level_bar_varies);
    RUN(test_menu_credits_view);

    /* Pause menu rendering */
    RUN(test_pause_menu_draws_header);
    RUN(test_pause_menu_shows_items);
    RUN(test_pause_menu_highlight_changes);
    RUN(test_pause_menu_scrollbar);

    printf("  %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
