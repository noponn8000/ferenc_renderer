CC      := gcc
CFLAGS  := -Wall -Wextra -O2 $(shell sdl2-config --cflags)
LDFLAGS := $(shell sdl2-config --libs) -lm

TARGET  := game
SRC     := main.c render.c
OBJ     := $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean

