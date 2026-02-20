# Source Code Guide

## Architecture

The codebase follows a strict **core/port separation**. All game logic, rendering, and UI lives in `core/` with zero platform dependencies. Ports are thin adapters that present the framebuffer, poll input, play audio, and provide timing.

```
src/
├── core/                  Platform-independent logic
│   ├── types.h            Shared types: Point_t, Direction_t, GameConfig_t
│   ├── snake.h/c          Snake data structure, movement, direction buffering
│   ├── game.h/c           Game state machine, collision, food/bonus, scoring
│   ├── lcd.h/c            84×48 1-bit framebuffer (Nokia 3310 LCD simulation)
│   ├── sprites.h          4×4 pixel sprites, font data, text drawing helpers
│   ├── theme.h/c          LcdColor_t, LcdPalette_t, palette definitions
│   ├── audio_types.h      SoundType_t enum, volume constants
│   ├── renderer.h/c       Game frame, menu, and pause rendering to LCD
│   └── ui.h/c             UI state machine (menu nav, pause, game-over)
├── platform.h             Abstract interface every port must implement
└── ports/
    ├── pc/                Terminal port (Unicode half-block, termios)
    └── pc-gui/            GUI port (Raylib, windowed pixel-grid LCD)
```

## Core Modules

### types.h

Foundation types shared across all core modules.

| Type | Description |
|------|-------------|
| `Point_t` | x, y coordinates (int16_t) |
| `Direction_t` | UP, DOWN, LEFT, RIGHT |
| `InputEvent_t` | Game-level input events |
| `GameConfig_t` | Board dimensions, initial length, tick rate |

### snake.h/c

Snake data structure with a 2-input direction buffer that prevents 180° reversals.

- `snake_init()` — Place snake at position with initial length
- `snake_move()` — Advance head, consume buffered direction, shrink tail (unless growing)
- `snake_grow()` — Schedule deferred growth (applied on next move)
- `snake_set_direction()` — Buffer up to 2 directions, rejects opposite/same
- `snake_collides_self()` / `snake_occupies()` — Collision queries
- Fat flags per segment track recent food consumption for sprite rendering

### game.h/c

Game state machine and update loop.

- `game_init()` — Initialize with config + RNG seed
- `game_update()` — One tick: move snake, check collisions, handle food/bonus
- `game_handle_input()` — Route directional input to snake
- Event flags (`evt_ate_food`, `evt_ate_bonus`) are set during update and cleared next tick
- Bonus spawns every 5 foods with a countdown timer

### lcd.h/c

84×48 pixel framebuffer. All rendering targets this buffer; ports read it via `lcd_get_framebuffer()`.

- Drawing primitives: `lcd_set_pixel()`, `lcd_fill_rect()`, `lcd_draw_rect()`, `lcd_draw_hline()`, `lcd_draw_vline()`
- Sprite rendering: `lcd_draw_sprite()`, `lcd_draw_sprite_inv()`
- `lcd_invert_rect()` — Used for menu selection highlight bars
- `lcd_set_clip()` / `lcd_clear_clip()` — Clipping region for scroll animations

Constants: `LCD_WIDTH` = 84, `LCD_HEIGHT` = 48.

### sprites.h

All sprite data is `static const` in the header (no .c file). Includes:

- **Snake parts** (4×4): head (4 dirs + eating variants), body straight/fat/corner, tail
- **Food** (4×4), **Bonus** (8×4, 6 variants)
- **Small font** (5×7, ASCII 32–122), **Digit font** (3×5)
- **Check mark** (5×7) for menu selections

Inline text helpers: `lcd_draw_text()`, `lcd_draw_number()`, `lcd_text_width()`.

### theme.h/c

Three Nokia-style palettes, each with 4 colors:

| Palette | Index |
|---------|-------|
| Green (classic Nokia) | `PALETTE_GREEN` |
| Grey | `PALETTE_GREY` |
| Amber | `PALETTE_AMBER` |

Each `LcdPalette_t` has: `backlight`, `pixel_on`, `pixel_off`, `gap`.

### renderer.h/c

Renders game state to the LCD framebuffer. Never touches platform APIs.

- `render_game_frame()` — Arena border, snake sprites, food, bonus, score, divider
- `render_menu()` — Main menu and submenus (colors, sound, speed, credits) with smooth scroll
- `render_pause_menu()` — Pause overlay with scrollbar

### ui.h/c

Top-level UI state machine that drives the entire app flow.

| State | Description |
|-------|-------------|
| `APP_MENU` | Main menu and submenus |
| `APP_PLAYING` | Active gameplay |
| `APP_PAUSED` | Pause overlay |
| `APP_GAME_OVER` | Game over (blinking snake) |
| `APP_QUIT` | Exit signal |

- `ui_handle_input()` → `UiResult_t` with action + sound for the port to execute
- `ui_tick_scroll()` — Drives smooth menu scroll animation
- `ui_input_to_game()` — Maps UI directional input to game input events

### audio_types.h

Sound event enum and volume constants. No implementation — each port synthesizes audio.

Sounds: `SND_NAV`, `SND_SELECT`, `SND_EAT`, `SND_BONUS`, `SND_GAME_OVER`.

## Platform Interface (platform.h)

Every port implements these 8 functions:

```c
int       platform_init(void);
void      platform_shutdown(void);
void      platform_present(const uint8_t (*fb)[LCD_WIDTH], const LcdPalette_t *palette);
UiInput_t platform_get_input(void);
double    platform_get_time(void);
bool      platform_should_close(void);
void      platform_play_sound(SoundType_t sound);
void      platform_set_volume(int level);
void      platform_sleep_ms(uint32_t ms);
```

## Data Flow

```
Input → platform_get_input() → ui_handle_input() → UiResult_t {action, sound}
                                                          │
                    ┌─────────────────────────────────────┘
                    ▼
          game_handle_input() / game_update()
                    │
                    ▼
          render_*() → lcd framebuffer (84×48)
                    │
                    ▼
          platform_present() → screen
```

## Adding a New Port

1. Create `src/ports/<name>/` with `main.c` and `Makefile`
2. Implement all 8 functions from `platform.h`
3. Copy `main.c` from an existing port — the game loop is identical across ports
4. Link against all `.c` files in `core/`
5. `platform_present()` reads `lcd_get_framebuffer()` and maps pixels using the palette

## Build

```bash
make -C src/ports/pc          # Terminal port
make -C src/ports/pc-gui      # GUI port (requires raylib)
make -C src/ports/pc-gui bundle  # macOS .app bundle with icon
make -C test                  # Run all tests
```
