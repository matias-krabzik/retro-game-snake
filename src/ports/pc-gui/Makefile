CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Werror
TARGET  = snake-gui

CORE_DIR    = ../../core
SRC_DIR     = .
RAYLIB_DIR  = /opt/homebrew

CFLAGS  += -I$(RAYLIB_DIR)/include
LDFLAGS  = -L$(RAYLIB_DIR)/lib -lraylib
LDFLAGS += -framework IOKit -framework Cocoa -framework OpenGL

CORE_SRC = $(CORE_DIR)/game.c $(CORE_DIR)/snake.c $(CORE_DIR)/lcd.c $(CORE_DIR)/theme.c $(CORE_DIR)/renderer.c $(CORE_DIR)/ui.c
PORT_SRC = $(SRC_DIR)/main.c $(SRC_DIR)/platform_raylib.c $(SRC_DIR)/audio.c

SRCS = $(CORE_SRC) $(PORT_SRC)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -I$(CORE_DIR)/../.. -o $@ $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: clean
