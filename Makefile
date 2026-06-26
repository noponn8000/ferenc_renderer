CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -g $(shell sdl2-config --cflags) -fsanitize=address
LDFLAGS := $(shell sdl2-config --libs) -lm -fsanitize=address

TARGET  := game
SRC     := font.c render.c audio.c engine.c pbm_reader.c player.c utils/textbox.c utils/timer.c utils/array.c main.c utils/midiplayer.c
OBJ     := $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean

