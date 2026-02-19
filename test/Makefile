CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Werror

SRC     = ../src
CORE    = $(SRC)/core

TESTS   = test_snake test_game test_lcd test_sprites

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

test_lcd: ports/pc-gui/test_lcd.c
	$(CC) $(CFLAGS) -Iports/pc-gui/mock -I$(SRC) -o $@ $<

test_sprites: ports/pc-gui/test_sprites.c
	$(CC) $(CFLAGS) -Iports/pc-gui/mock -I$(SRC) -o $@ $<

clean:
	rm -f $(TESTS)
