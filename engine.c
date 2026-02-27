#include <stdlib.h>
#include <stdbool.h>

#include "engine.h"

int main(void) {
    return EXIT_SUCCESS;
}

void FE_InitEngine(PreFrameCallback pre_frame, PostFrameCallback post_frame) {
     
}

EntityRef FE_RegisterEntity(Engine* e, Entity* entity) {
    EntityRef ref = {0};
    for (int i = 0; i < e->entity_count; i++) {
        if (e->entities[i].full) {
            ref.epoch = e->entities[i].epoch;
            ref.ref = e->entities[i].index;
            e->entities[i].full = true;
            e->entities[i].entity = entity;

            return ref;
        }
    }

    // Resize the array and put the entity in the first available place
    size_t index = FE_GrowEntityStore(e);
    e->entities[index].full = true; 
    e->entities[index].entity = entity;
    ref.epoch = e->entities[index].epoch;
    ref.ref = e->entities[index].index;

    return ref;
}

size_t FE_GrowEntityStore(Engine* e) {
    e->entities = realloc(e->entities, sizeof(EntityField) * e->entity_count * 2);
    size_t available_index = e->entity_count;

    for (int i = e->entity_count; i < e->entity_count * 2; i++) {
        e->entities[i].index = i;
    }

    e->entity_count *= 2;
    return available_index;
}

bool FE_IsEntityRefValid(Engine* e, EntityRef ref) {
    if (ref.ref < 0 || ref.ref >= e->entity_count) {
        return false;
    }

    if (!e->entities[ref.ref].full || e->entities[ref.ref].epoch != ref.epoch) {
        return false;
    }

    return true;
}

void FE_RemoveObject(Engine* e, EntityRef ref) {
    if (!FE_IsEntityRefValid(e, ref)) {
        return;
    }

    e->entities[ref.ref].full = false;
    e->entities[ref.ref].epoch++;
}

