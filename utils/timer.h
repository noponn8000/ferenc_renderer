#include "../engine.h"

typedef void (*TimerCallback)(void);

typedef struct {
    TimerCallback callback;
    float acc;
    float duration;
    bool oneShot;
    bool finished;
} TimerData;

void TimerUpdate(void* self, Input input, float dt);
void TimerInit(void* self);
Entity TimerConstruct(TimerCallback callback, float duration, bool oneShot);


