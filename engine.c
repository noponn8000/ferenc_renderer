#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "engine.h"

Engine FE_InitEngine(PreFrameCallback pre_frame, PostFrameCallback post_frame) {
    Entity* entities = malloc(32 * sizeof(Entity));
    Input input = { 0 };
    Engine engine = {
    	pre_frame,
	post_frame,
    input,
	entities,
	32,
	0,
	0,
    0.016
    };

    return engine;
}

void FE_DestroyEngine(Engine* engine) {
	free(engine->entities);
}

int FE_AddEntity(Engine* engine, Entity e) {
	if (engine->entity_count >= engine->capacity) {
		// Resize entity store
		engine->capacity *= 2;
		engine->entities = realloc(engine->entities, engine->capacity * sizeof(Entity));
	}
	e.id = engine->id_counter++;
	engine->entities[engine->entity_count] = e;
	engine->entities[engine->entity_count].c_init(e.data);
	engine->entity_count++;
	return e.id;
}

void FE_RemoveEntity(Engine* engine, int id) {
	for (int i = 0; i < engine->entity_count; i++) {
		if (engine->entities[i].id == id) {
			engine->entities[i].queueFree = true;
			return;
		}
	}
}

void FE_Loop(Engine* engine, RenderContext rctx, AudioContext actx) {
	float delta = engine->preframe(engine);
    engine->dt = delta;
	for (int i = 0; i < engine->entity_count; i++) {
		Entity* e = &engine->entities[i];
        if (e->c_update != NULL) {
            e->c_update(e->data, engine->input, delta);
        }
        if (e->c_update != NULL) {
		    e->c_draw(e->data, rctx);
        }
        if (e->c_audio != NULL) {
            e->c_audio(e->data, actx);
        }
	}

	// Free entities that are queued to be deleted
	FE_Free(engine);

    // Mix audio streams
    FE_MixAudio(actx);

	// Call post-frame callback
	engine->postframe(engine, rctx, actx);
}

void FE_Free(Engine* engine) {
	for (int i = 0; i < engine->entity_count; ) {
		if (engine->entities[i].queueFree) {
			if (engine->entities[i].c_remove)
				engine->entities[i].c_remove(engine->entities[i].data);

			// Swap the last entity into the place of the one being removed
			engine->entities[i] = engine->entities[engine->entity_count - 1];		
			// Decrement entity count
			engine->entity_count--;
		} else {
			i++;
		}
	}
}

void FE_MixAudio(AudioContext actx) {
    memset(actx.output, 0, actx.streams[0].n_samples * sizeof(float));
    for (int i = 0; i < N_AUDIO_STREAMS; i++) {
        for (int j = 0; j < actx.streams[i].n_samples; j++) {
            actx.output[j] += actx.streams[i].volume * actx.streams[i].frames[j];
        }
    }
}

