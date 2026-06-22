CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -g $(shell sdl2-config --cflags)
LDFLAGS := $(shell sdl2-config --libs) -lm

TARGET  := game
SRC     := font.c render.c audio.c engine.c pbm_reader.c player.c utils/textbox.c utils/timer.c main.c 
OBJ     := $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean

