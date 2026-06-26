#pragma once

#include<math.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdio.h>
#include<byteswap.h>
#include<assert.h>
#include<stdbool.h>

#include "utils/array.h"

#define EQUAL_TEMPERAMENT_RATIO 1.05946

typedef enum { NOTE_ON, NOTE_OFF } MIDIEventType;

typedef struct {
    MIDIEventType type;
    uint8_t pitch;
    uint8_t velocity;
    uint64_t ticks;
} MIDIEvent;

typedef darray(MIDIEvent) MIDIEventArray;

typedef struct {
    bool enabled[128];
    double phases[128];
    double amplitudes[128];
    double targetAmplitudes[128];
    double attenuationRate;
    int baseFrequency;
} Wavetable;

void FA_Sine(float* buffer, int samples, int sample_rate, double* phase, int frequency, float amplitude);
void FA_Noise(float* buffer, int samples, int sample_rate, float amplitude);
void FA_AttenuatedNoise(float* buffer, int samples, int sample_rate, double* phase, float duration, float amplitude);
void FA_ClearBuffer(float* buffer, int samples);
void FA_PlayWavetable(float* buffer, int samples, int sample_rate, Wavetable *wt);

void FA_LowPass(float* buffer, int samples, float a);
void FA_FrequencyImpulseResponse(float* buffer, int samples, int n);

MIDIEventArray FA_readMIDI(FILE* file);
