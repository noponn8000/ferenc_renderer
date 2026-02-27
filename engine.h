#include <stdint.h>

typedef void (*PreFrameCallback)(void *self);
typedef float (*PostFrameCallback)(void *self);
typedef void (*UpdateFn)(void *self, float dt);
typedef void (*InitFn)(void *self);
typedef void (*DrawFn)(void *self, uint32_t *pixels, uint16_t canvas_w, uint16_t canvas_h);
typedef void (*RemoveFn)(void *self);

typedef struct {
        InitFn  c_init;
        UpdateFn c_update;
        DrawFn c_draw;
        RemoveFn c_remove;
        bool queueFree;
        void* data;
} Entity;

typedef struct {
    uint32_t ref;
    uint32_t epoch;
} EntityRef;

typedef struct {
    uint32_t index;
    uint32_t epoch;
    Entity* entity;
    bool full;
} EntityField;

typedef struct {
    PreFrameCallback preframe;
    PostFrameCallback postframe;
    EntityField* entities;
    int entity_count;
} Engine;

size_t FE_GrowEntityStore(Engine* e);
void FE_RemoveObject(Engine* e, EntityRef ref);
