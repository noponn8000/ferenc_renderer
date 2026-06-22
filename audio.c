#include<math.h>
#include<string.h>
#include<stdlib.h>

void FR_ClearBuffer(float* buffer, int samples) {
    memset(buffer, 0, samples * sizeof(float));
}

void FR_Sine(float* buffer, int samples, int sample_rate, double* phase, int frequency, float amplitude) {
    double dPhi = (2 * M_PI * frequency)/sample_rate;
    for (int i = 0; i < samples; i++) {
        buffer[i] += amplitude * sin(*phase);
        *phase += dPhi;
    }

    *phase = fmod(*phase, 2 * M_PI);
}

void FR_Noise(float* buffer, int samples, int sample_rate, float amplitude) {
    for (int i = 0; i < samples; i++) {
        float random = 2 * (((float) rand() / RAND_MAX) - 0.5f);
        buffer[i] += amplitude * random;
    }
}

void FR_AttenuatedNoise(float* buffer, int samples, int sample_rate, double* phase, float duration, float amplitude) {
    // The whole signal has been rendered
    if (*phase > 1.0) return;

    double dPhi = 1.0 / (duration * sample_rate);
    for (int i = 0; i < samples; i++) {
        float random = 2 * (((float) rand() / RAND_MAX) - 0.5f);
        buffer[i] += amplitude * cos(M_PI * (*phase)) * random;
        *phase += dPhi;
    }
}

// piodła
