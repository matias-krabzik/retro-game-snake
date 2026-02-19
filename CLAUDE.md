# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Retro Snake game written in C, simulating the Nokia 3310 LCD aesthetic. The architecture strictly separates platform-independent core logic (game, rendering, UI, LCD framebuffer) from platform-specific adapters (ports). Ports are thin UI library adapters that only provide: framebuffer presentation, input polling, audio playback, and timing.

## Architecture

```
src/
  core/               -- Platform-independent logic (zero OS/library deps)
    types.h           -- Shared types: Point_t, Direction_t, GameConfig_t
    snake.h/c         -- Snake data structure and movement
    game.h/c          -- Game state machine, update loop, collision, food, scoring
    lcd.h/c           -- 84x48 1-bit framebuffer (Nokia 3310 LCD simulation)
    sprites.h         -- 4x4 pixel sprites, font data, text drawing helpers
    theme.h/c         -- LcdColor_t, LcdPalette_t, palette definitions (Green/Grey/Amber)
    audio_types.h     -- SoundType_t enum and volume constants
    renderer.h/c      -- Game frame, menu, and pause menu rendering to LCD framebuffer
    ui.h/c            -- UI state machine (menu navigation, pause, game-over transitions)
  platform.h          -- Abstract interface that every port must implement
  ports/
    pc/               -- Terminal port (Unicode half-block rendering, termios)
    pc-gui/           -- GUI port using Raylib (windowed, pixel-grid LCD presentation)
```

### Separation Rules

- **core/** contains all game logic, rendering, and UI with zero platform dependencies. It must never `#include` platform or OS headers (`<termios.h>`, `<windows.h>`, `<raylib.h>`, etc.).
- **platform.h** defines the thin contract: `platform_init()`, `platform_present()`, `platform_get_input()`, `platform_get_time()`, `platform_should_close()`, `platform_play_sound()`, `platform_set_volume()`, `platform_sleep_ms()`, `platform_shutdown()`.
- **ports/<platform>/** contains `main.c` (unified game loop), platform implementation, and port-specific code (e.g., audio synthesis). Each port has its own `Makefile`.
- Core renders to an 84x48 framebuffer. Ports read it via `lcd_get_framebuffer()` and present it using their UI library.

### Key Types

- `UiInput_t` — abstract input events (UP/DOWN/LEFT/RIGHT/CONFIRM/BACK/QUIT)
- `UiState_t` — full UI state machine (app state, menu nav, pause nav, settings)
- `UiResult_t` — action + sound returned from `ui_handle_input()`
- `LcdPalette_t` — 4-color palette (backlight, pixel_on, pixel_off, gap)

## Build Commands

```bash
# PC terminal port
make -C src/ports/pc
./src/ports/pc/snake

# PC GUI port (requires raylib: brew install raylib)
make -C src/ports/pc-gui
./src/ports/pc-gui/snake-gui

# Clean any port
make -C src/ports/<port> clean

# Run tests (no external dependencies needed)
make -C test
```

## C Conventions

- **Standard:** C99 (`-std=c99`)
- **Warnings:** `-Wall -Wextra -Werror`
- **Naming:** `snake_case` for functions/variables, `UPPER_SNAKE_CASE` for macros/constants, `TypeName_t` for typedefs
- **Headers:** Include guards (`#ifndef HEADER_NAME_H` / `#define HEADER_NAME_H`)
- **Memory:** Prefer stack allocation and fixed-size arrays; document ownership when heap is necessary

## Dependencies

- **pc port:** None (POSIX only — termios, ANSI/Unicode terminal)
- **pc-gui port:** [Raylib](https://www.raylib.com/) 5.x (`brew install raylib`)

## Workflow Rules

- **All tests must pass before a task is considered complete.** Run `make -C test` and verify 0 failures before finishing any change.
- When modifying core or port code, add or update tests to cover the change.

## Adding a New Port

1. Create `src/ports/<platform>/` with `main.c` and `Makefile`
2. Implement all functions declared in `src/platform.h`
3. Link against all compiled objects from `src/core/`
4. The `main.c` game loop uses the core UI state machine — copy from pc or pc-gui as a starting point
5. `platform_present()` reads the framebuffer via `lcd_get_framebuffer()` and renders it using the port's UI library
