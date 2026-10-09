#pragma once

#include <stdlib.h>
#include <stdio.h>
#include "../engine/engine.h"
#include "../physics/physics.h"
#include "../utils/pbm_reader.h"
#include "../renderer/render.h"
#include "../ui/font.h"

typedef void (*ButtonPressCallback)(void);

typedef struct {
    Font font;
    Vector2i position;
    Vector2i size;
    char* str;
    uint32_t normalColor;
    uint32_t hoverColor;
    uint32_t pressColor;
    ButtonPressCallback pressCallback;

    // Internal data
    bool hovered;
    bool pressed;

} ButtonData;

void ButtonInit(void *self);
void ButtonDraw(void* self, RenderContext ctx);
void ButtonUpdate(void* self, Input input, float dt);
void ButtonRemove(void *self);
Entity ButtonConstruct(Vector2i position, Vector2i size, char* str, uint32_t normalColor, uint32_t hoverColor, uint32_t press_color, ButtonPressCallback pressCallback);
