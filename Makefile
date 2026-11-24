CC      := gcc
CFLAGS  := -Wextra -O2 -g $(shell sdl2-config --cflags) -Isrc
LDFLAGS := $(shell sdl2-config --libs) -lm

TARGET  := build/program

SRC     := $(wildcard src/utils/*.c) \
           $(wildcard src/ui/*.c) \
           $(wildcard src/renderer/*.c) \
           $(wildcard src/audio/*.c) \
           $(wildcard src/engine/*.c) \
           $(wildcard src/math/*.c) \
           $(wildcard src/obj/*.c) \
           $(wildcard src/application/*.c)

OBJ     := $(patsubst src/%.c,build/%.o,$(SRC))

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build $(TARGET)

.PHONY: all clean
