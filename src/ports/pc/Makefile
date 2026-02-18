CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Werror
TARGET  = snake

CORE_DIR = ../../core
SRC_DIR  = .

CORE_SRC = $(CORE_DIR)/game.c $(CORE_DIR)/snake.c
PORT_SRC = $(SRC_DIR)/main.c $(SRC_DIR)/platform_pc.c $(SRC_DIR)/render.c $(SRC_DIR)/input.c

SRCS = $(CORE_SRC) $(PORT_SRC)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -I$(CORE_DIR)/../.. -o $@ $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: clean
