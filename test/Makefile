CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Werror

SRC     = ../src
CORE    = $(SRC)/core

TESTS   = test_snake test_game test_lcd test_sprites test_renderer test_ui

.PHONY: all clean

all: $(TESTS)
	@failed=0; \
	for t in $(TESTS); do \
		./$$t || failed=1; \
	done; \
	if [ $$failed -eq 1 ]; then exit 1; fi

test_snake: core/test_snake.c $(CORE)/snake.c
	$(CC) $(CFLAGS) -I$(SRC) -o $@ $^

test_game: core/test_game.c $(CORE)/game.c $(CORE)/snake.c
	$(CC) $(CFLAGS) -I$(SRC) -o $@ $^

test_lcd: core/test_lcd.c $(CORE)/lcd.c
	$(CC) $(CFLAGS) -I$(SRC) -o $@ $^

test_sprites: core/test_sprites.c $(CORE)/lcd.c
	$(CC) $(CFLAGS) -I$(SRC) -o $@ $^

test_renderer: core/test_renderer.c $(CORE)/renderer.c $(CORE)/lcd.c $(CORE)/game.c $(CORE)/snake.c $(CORE)/theme.c
	$(CC) $(CFLAGS) -I$(SRC) -o $@ $^

test_ui: core/test_ui.c $(CORE)/ui.c $(CORE)/renderer.c $(CORE)/lcd.c $(CORE)/game.c $(CORE)/snake.c $(CORE)/theme.c
	$(CC) $(CFLAGS) -I$(SRC) -o $@ $^

clean:
	rm -f $(TESTS)
