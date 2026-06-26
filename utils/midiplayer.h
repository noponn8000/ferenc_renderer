#include "../engine.h"
#include "../audio.h"

typedef struct {
    Wavetable* wt;
    MIDIEventArray events;
    uint8_t audioStream;
    uint16_t ticksPerSecond;
    uint64_t ticks;
    size_t currentEventIndex;
} MIDIPlayerData;

void MIDIPlayerUpdate(void* self, Input input, float dt);
void MIDIPlayerAudio(void* self, AudioContext actx);
void MIDIPlayerInit(void* self);
Entity MIDIPlayerConstruct(Wavetable* wt, MIDIEventArray events, uint16_t ticksPerSecond, uint8_t audioStream);
