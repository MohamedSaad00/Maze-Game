CC = gcc
CFLAGS = -Wall -Werror -Wextra -pedantic
SDL2_FLAGS = $(shell sdl2-config --cflags --libs)

SRC_DIR = src
INC_DIR = inc

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = maze_game

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(SDL2_FLAGS) -lm

%.o: %.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

re: clean all

.PHONY: all clean re 
