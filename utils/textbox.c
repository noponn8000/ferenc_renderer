#include "textbox.h"

void TextboxInit(void *self) {
    TextboxData* data =  (TextboxData*) self;
    FILE *texFile = fopen("res/font_halfsize.pbm", "rb");

    if (texFile == NULL) {
        fprintf(stderr, "WARNING: Missing font texture!");
    }

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
    data->shown = 0; data->cps = 15;
    data->str = "You are a singing chalice. Revel in your new body, for you have been blessed. The nullity welcomes you into its lukewarm embrace.";
}

void TextboxDraw(void* self, RenderContext ctx) {
    TextboxData* data =  (TextboxData*) self;

    char current = data->str[0];
    int x_0 = data->position.x + data->margin.x;
    int y_0 = data->position.y + data->margin.y;
    int x = x_0; int y = y_0;
    int i = 0;
    while (current != '\0' && i < data->size.x * data->size.y && i < data->shown) {
        if (current == ' ') {
            int remaining = data->size.x - (i % data->size.x);
            int wordLen = 0;
            // Check if we need to wrap
            for (int k = i + 1; ; k++) {
                char peek = data->str[k];
                if (peek == '\0' || peek == ' ' || peek == '\n') {
                    break;
                }

                wordLen++;
            }

            // No space found further on, so we break on this one.
            if (wordLen >= remaining)
                current = '\n';
        }

        if (current == '\n') {
            x = x_0;
            y += data->font.glyph_height + data->glyphSpacing.y;
            current = data->str[++i];
            continue;
        }

        FR_DrawLetter(ctx.pixels, ctx.canvas_w, ctx.canvas_h, x, y, current, 0xFFFFFFFF, data->font);
        x += data->font.glyph_width + data->glyphSpacing.x;
        current = data->str[++i];
    }

    FR_DrawRect(ctx.pixels, ctx.canvas_w, ctx.canvas_h, 
                data->position.x, data->position.y,
                2 * data->margin.x + data->size.x * data->font.glyph_width,
                2 * data->margin.y + data->size.y * data->font.glyph_height,
                0xFFFFFFFF);
}

void TextboxUpdate(void* self, Input input, float dt) {
    TextboxData* data =  (TextboxData*) self;

    data->acc += dt;
    if (data->acc > (1.0 / data->cps)) {
        data->shown++;
        data->acc -= (1.0 / data->cps);
    }
}

Entity TextboxConstruct(Vector2i position, Vector2i size, Vector2i margin, Vector2i glyphSpacing, char* str) {
    TextboxData* data = malloc(sizeof(TextboxData)); 
    *data = (TextboxData) {
        .position = position,
        .size = size,
        .margin = margin,
        .glyphSpacing = glyphSpacing,
        .str = str
    };

    Entity textbox = {
        .c_init = TextboxInit,
        .c_update = TextboxUpdate,
        .c_draw = TextboxDraw,
        .c_audio = NULL,
        .data = data
    };

    return textbox;
}
