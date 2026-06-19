CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -O2 -Iinclude
LDFLAGS = -lSDL2

SRC     = src/main.c src/game.c src/snake.c src/food.c
OBJ     = $(SRC:.c=.o)
TARGET  = snake


all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(TARGET)
