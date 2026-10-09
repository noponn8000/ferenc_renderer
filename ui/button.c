#include "button.h"
#include "../physics/physics.h"

void ButtonInit(void *self) {
    ButtonData* data =  (ButtonData*) self;
    FILE *texFile = fopen("res/font_halfsize.pbm", "rb");

    assert(texFile != NULL);

    int x; int y;
    bool* fonttex = readPBM(texFile, &x, &y);
    Font font = {
        fonttex,
        x, y,
        FONT_GLYPH_WIDTH_SM, FONT_GLYPH_HEIGHT_SM,
        FONT_N_GLYPHS, FONT_GLYPH_SET,
        FONT_LOOKUP_TABLE
    };

    data->font = font;

    fclose(texFile);
}

void ButtonDraw(void* self, RenderContext ctx) {
    ButtonData* data =  (ButtonData*) self;

    uint32_t color;
    if (data->pressed) {
        color = data->pressColor;
    } else if (data->hovered) {
        color = data->hoverColor;
    } else {
        color = data->normalColor;
    }

    FR_DrawRect(ctx.pixels, ctx.canvas_w, ctx.canvas_h, 
                data->position.x, data->position.y,
                data->size.x,
                data->size.y,
                color);
}

void ButtonUpdate(void* self, Input input, float dt) {
    ButtonData* data =  (ButtonData*) self;
    
    Vector2i mousePos = { input.mouseX, input.mouseY };
    data->hovered = pointInRect(data->position, data->size, mousePos);

    if (data->hovered) {
        if (input.mouseButton[0]) {
            data->pressed = true;
        } else {
            if (data->pressed) {
                data->pressed = false;
                data->pressCallback();
            }
        }
    }
}

void ButtonRemove(void *self) {
    ButtonData* data = (ButtonData*) self;

    free(data);
}

Entity ButtonConstruct(Vector2i position, Vector2i size, char* str, uint32_t normalColor, uint32_t hoverColor, uint32_t pressColor, ButtonPressCallback pressCallback) {
    ButtonData* data = malloc(sizeof(ButtonData)); 
    *data = (ButtonData) {
        .position = position,
        .size = size,
        .str = str,
        .hoverColor = hoverColor,
        .pressColor = pressColor,
        .normalColor = normalColor,
        .pressCallback = pressCallback,
        .hovered = false,
    };

    Entity button = {
        .c_init = ButtonInit,
        .c_update = ButtonUpdate,
        .c_draw = ButtonDraw,
        .c_remove = ButtonRemove,
        .data = data
    };

    return button;
}
