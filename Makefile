CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -g $(shell sdl2-config --cflags) 
LDFLAGS := $(shell sdl2-config --libs) -lm

TARGET  := game
SRC     := musicplayer/miniaudio.c utils/font_bdf.c cue/cuefile.c physics/physics.c ui/button.c ui/font.c renderer/render.c audio/audio.c engine/engine.c utils/pbm_reader.c utils/textbox.c utils/timer.c utils/array.c application/musicplayer.c 
OBJ     := $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean

