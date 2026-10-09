#pragma once

#include <stdlib.h>
#include <stdio.h>
#include "../engine/engine.h"
#include "../physics/physics.h"
#include "../utils/pbm_reader.h"
#include "../renderer/render.h"
#include "../ui/font.h"

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
    uint32_t fg_color;
    uint32_t box_color;
    bool fill;
    char* str;
} TextboxData;

void TextboxInit(void *self);
void TextboxDraw(void* self, RenderContext ctx);
void TextboxUpdate(void* self, Input input, float dt);
void TextboxRemove(void *self);
Entity TextboxConstruct(Vector2i position, Vector2i size, Vector2i margin, Vector2i glyphSpacing, uint32_t fg_color, uint32_t box_color, bool fill, int cps, char* str, Font font);
void TextboxSetText(TextboxData *data, char* str);
