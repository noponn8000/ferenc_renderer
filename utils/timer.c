#include <math.h>
#include <stdlib.h>
#include "timer.h"

void TimerUpdate(void* self, Input input, float dt) {
    TimerData* data = (TimerData*) self;

    if (data->finished) return;

    data->acc += dt;
    
    if (data->acc >= data->duration) {
        data->callback();

        if (data->oneShot) {
            data->finished = true;
        } else {
            data->acc = fmod(data->acc, data->duration);
        }
    }
}

void TimerInit(void* self) {
    TimerData* data = (TimerData*) self;
    
    data->acc = 0.0;
    data->finished = false;
}

Entity TimerConstruct(TimerCallback callback, float duration, bool oneShot) {
    TimerData data = {
        .callback = callback,
        .duration = duration,
        .oneShot = oneShot
    };

    Entity timer = {
        .c_init = TimerInit,
        .c_update = TimerUpdate,
        .c_draw = NULL,
        .c_audio = NULL,
        .data = &data
    };

    return timer;
}

