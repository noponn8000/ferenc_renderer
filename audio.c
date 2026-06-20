#include<math.h>
#include<string.h>

void FR_ClearBuffer(float* buffer, int samples) {
    memset(buffer, 0, samples * sizeof(float));
}

void FR_Sine(float* buffer, int samples, int sample_rate, double t, int frequency, float amplitude) {
    double delta = 1.0 / sample_rate;
    for (int i = 0; i < samples; i++) {
        buffer[i] += amplitude * sinf(2.0f * M_PI * frequency * t);
        t += delta;
    }
}
