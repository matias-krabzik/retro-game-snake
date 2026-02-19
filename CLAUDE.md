# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Retro Snake game written in C. The architecture strictly separates platform-independent core logic from platform-specific adapters (ports), enabling the same game to run on different targets (PC terminal, embedded, web via emscripten, etc.).

## Architecture

```
src/
  core/         -- Platform-independent game logic (state, mechanics, rules)
    types.h     -- Shared types: Point_t, Direction_t, GameConfig_t
    snake.h/c   -- Snake data structure and movement
    game.h/c    -- Game state machine, update loop, collision, food, scoring
  platform.h    -- Abstract interface that every port must implement
  ports/
    pc/         -- Terminal port (ANSI escape codes, termios)
    pc-gui/     -- GUI port using Raylib (windowed, 2D accelerated)
```

### Separation Rules

- **core/** contains pure game logic with zero platform dependencies. It must never `#include` platform or OS headers (`<termios.h>`, `<windows.h>`, SDL, etc.).
- **platform.h** defines the contract between core and ports: `platform_init()`, `platform_render()`, `platform_get_input()`, `platform_sleep_ms()`, `platform_shutdown()`. Ports implement these functions.
- **ports/<platform>/** contains `main()`, implements `platform.h`, and links against core objects. Each port has its own `Makefile`.
- Core calls port functions only through the `platform.h` interface. Ports call core functions to initialize, update, and query game state.

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

- **pc port:** None (POSIX only — termios, ANSI escape codes)
- **pc-gui port:** [Raylib](https://www.raylib.com/) 5.x (`brew install raylib`)

## Workflow Rules

- **All tests must pass before a task is considered complete.** Run `make -C test` and verify 0 failures before finishing any change.
- When modifying core or port code, add or update tests to cover the change.

## Adding a New Port

1. Create `src/ports/<platform>/` with `main.c` and `Makefile`
2. Implement all functions declared in `src/platform.h`
3. Link against compiled objects from `src/core/`
4. The `main.c` game loop must decouple render FPS from game tick rate (see pc-gui for reference)
