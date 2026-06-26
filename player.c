#include "player.h"

void PlayerInit(void* self) {
    PlayerData* data = (PlayerData*) self;
    data->direction.x = 0;
    data->direction.y = 0;
    data->position.x = 200;
    data->position.y = 200;
    data->left = false;
    data->right = false;
    data->up = false;
    data->down = false;
    data->speed = 30;

    FILE *texFile = fopen("res/player.pbm", "rb");

    data->texture = readPBM(texFile, &(data->texWidth), &(data->texHeight));
    assert(data->texture != NULL);
    data->framesX = 1;
    data->framesY = 2;
    data->frame = 0;
    data->frameTime = 0.0;
    data->targetFrameTime = 0.15;

    data->audioStream = 0;
    data->baseFreq = 200.0;
    data->phase[0] = 0.0;
    data->phase[1] = 0.0;
    data->phase[2] = 0.0;

    fclose(texFile);
}

void PlayerUpdate(void* self, Input input, float dt) {
    PlayerData* data = (PlayerData*) self;

    for (int i = 0; i < input.event_counter; i++) {
        if (input.events[i].key == 'a') {
            data->left = input.events[i].down;
        }
        else if (input.events[i].key == 'e') {
            data->right = input.events[i].down;
        }
        else if (input.events[i].key == ',') {
            data->up = input.events[i].down;
        }
        else if (input.events[i].key == 'o') {
            data->down = input.events[i].down;
        }
    }

    data->direction.x = 0; data->direction.y = 0;
    if (data->left) data->direction.x -= 1;
    if (data->right) data->direction.x += 1;
    if (data->up) data->direction.y -= 1;
    if (data->down) data->direction.y += 1;


    data->position.x += data->speed * data->direction.x * dt;
    data->position.y += data->speed * data->direction.y * dt;

    // Animate
    if (data->frameTime > data->targetFrameTime) {
        data->frame = (data->frame + 1) % 2;
        data->frameTime -= data->targetFrameTime;
    }
    data->frameTime += dt;

    data->baseFreq += 25.0 * dt;
    if (data->baseFreq > 800.0) data->baseFreq = 200.0;
}

void PlayerDraw(void* self, RenderContext ctx) {
    PlayerData* data = (PlayerData*) self;
    FR_DrawBinarySpritesheet(
		ctx.pixels, ctx.canvas_w, ctx.canvas_h,
		(int) data->position.x, (int) data->position.y,
        data->texture, data->texWidth, data->texHeight,
        data->framesX, data->framesY, data->frame,
        0xFFFFFFFF
    );

    //FR_PostprocessDither(ctx.pixels, ctx.canvas_w, ctx.canvas_h, 0.5, 4, false);
}

void PlayerAudio(void* self, AudioContext ctx) {
    PlayerData* data = (PlayerData*) self;
    int samples = ctx.streams[data->audioStream].n_samples;
    float* frames = ctx.streams[data->audioStream].frames;

    int freq = ((int) data->baseFreq / 20)*20;
    FA_ClearBuffer(frames, samples);
    FA_Sine(frames, samples, ctx.sample_rate, &data->phase[0], freq, 0.5);
    FA_Sine(frames, samples, ctx.sample_rate, &data->phase[1], 1.25*freq, 0.5);
    FA_Sine(frames, samples, ctx.sample_rate, &data->phase[2], 1.6*freq, 0.5);
}

void PlayerRemove(void* self) {
    PlayerData *data = malloc(sizeof(PlayerData));
    free(data->texture);

    free(data);
}

Entity PlayerConstruct() {
    PlayerData *data = malloc(sizeof(PlayerData));
    Entity player = {
        .c_init = PlayerInit,
        .c_update = PlayerUpdate,
        .c_draw = PlayerDraw,
        .c_audio = PlayerAudio,
        .c_remove = PlayerRemove,
        .data = data
    };

    return player;
}

// gr: go to references
// gd: go to definition
// space rn: rename
// K: hover
// space f: format
// space e: diagnostic
