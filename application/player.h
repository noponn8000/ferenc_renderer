#pragma once

#include <stdlib.h>
#include <stdio.h>
#include "../engine/engine.h"
#include "../physics/physics.h"
#include "../utils/pbm_reader.h"
#include "../renderer/render.h"
#include "../audio/audio.h"

typedef struct {    
    // Textures
    // bool* texture;
    uint32_t* texture;
    int texWidth;
    int texHeight;
    // Animation
    int framesX;
    int framesY;
    int frame;
    double targetFrameTime;
    double frameTime;
    // Audio
    size_t audioStream;
    double phase[3];
    float baseFreq;
    // State
    Vector2i direction;
    Vector2f position;
    int speed;
    // Input
    bool left;
    bool right;
    bool up;
    bool down;
} PlayerData;

void PlayerInit(void* self);
void PlayerUpdate(void* self, Input input, float dt);
void PlayerDraw(void* self, RenderContext ctx);
void PlayerAudio(void* self, AudioContext ctx);
void PlayerRemove(void* self);
Entity PlayerConstruct();
