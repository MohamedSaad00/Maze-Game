CC = gcc
CFLAGS = -Wall -Werror -Wextra -pedantic -O2
# sdl2-config is on the PATH on Linux and macOS. On Windows (MinGW) pass the
# paths yourself, e.g.  make SDL2_CFLAGS="-IC:/SDL2/include -Dmain=SDL_main" \
#                            SDL2_LIBS="-LC:/SDL2/lib -lmingw32 -lSDL2main -lSDL2"
SDL2_CFLAGS ?= $(shell sdl2-config --cflags)
SDL2_LIBS ?= $(shell sdl2-config --libs)

SRC_DIR = src
INC_DIR = inc

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = maze_game

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(SDL2_LIBS) -lm

%.o: %.c $(INC_DIR)/maze.h
	$(CC) $(CFLAGS) $(SDL2_CFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe

re: clean all

.PHONY: all clean re
