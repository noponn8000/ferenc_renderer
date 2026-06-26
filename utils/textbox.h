#pragma once

#include <stdlib.h>
#include <stdio.h>
#include "../engine.h"
#include "../physics.h"
#include "../pbm_reader.h"
#include "../render.h"
#include "../font.h"

typedef struct {
    Font font;
    int shown;
    int cps;
    float acc;
    Vector2i position;
    // In glyphs
    Vector2i size;
    Vector2i margin;
    Vector2i glyphSpacing;
    char* str;
} TextboxData;

void TextboxInit(void *self);
void TextboxDraw(void* self, RenderContext ctx);
void TextboxUpdate(void* self, Input input, float dt);
void TextboxRemove(void *self);
Entity TextboxConstruct(Vector2i position, Vector2i size, Vector2i margin, Vector2i glyphSpacing, char* str);
void TextboxSetText(TextboxData *data, char* str);
