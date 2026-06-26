#include "midiplayer.h"

void MIDIPlayerUpdate(void* self, Input input, float dt) {
    MIDIPlayerData* data = (MIDIPlayerData*) self;

    data->ticks += dt * data->ticksPerSecond;

    while (data->currentEventIndex < data->events.count) {
        MIDIEvent nextEvent = data->events.items[data->currentEventIndex];
        if (nextEvent.ticks > data->ticks) break;

        data->currentEventIndex++;
        // Invalid pitch
        if (nextEvent.pitch >= 128) continue;

        if (nextEvent.type == NOTE_ON) {
            data->wt->enabled[nextEvent.pitch] = true;
            data->wt->targetAmplitudes[nextEvent.pitch] = (float) nextEvent.velocity / 127;
        } else if (nextEvent.type == NOTE_OFF) {
            data->wt->enabled[nextEvent.pitch] = false;
            data->wt->targetAmplitudes[nextEvent.pitch] = 0;
            // We don't really care about note off velocity.
        }
    }
}

void MIDIPlayerAudio(void* self, AudioContext actx) {
    MIDIPlayerData* data = (MIDIPlayerData*) self;
    FA_PlayWavetable(actx.streams[data->audioStream].frames,
                     actx.streams[data->audioStream].n_samples,
                     actx.sample_rate, data->wt);

    FA_LowPass(actx.streams[data->audioStream].frames,
               actx.streams[data->audioStream].n_samples, 0.5);
}

void MIDIPlayerInit(void* self) {}

void MIDIPlayerFree(void* self) {
    MIDIPlayerData* data = (MIDIPlayerData*) self;

    free(data);
}

Entity MIDIPlayerConstruct(Wavetable* wt, MIDIEventArray events, uint16_t ticksPerSecond, uint8_t audioStream) {
    MIDIPlayerData* data = malloc(sizeof(MIDIPlayerData));
    data->audioStream = audioStream;
    data->ticksPerSecond = ticksPerSecond;
    data->ticks = 0;
    data->currentEventIndex = 0;
    data->events = events;
    data->wt = wt;

    Entity midiPlayer = {
        .c_init = MIDIPlayerInit,
        .c_update = MIDIPlayerUpdate,
        .c_audio = MIDIPlayerAudio,
        .c_remove = NULL,
        .data = data
    };

    return midiPlayer;
}
