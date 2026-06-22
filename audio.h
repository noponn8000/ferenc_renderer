#pragma once

void FR_Sine(float* buffer, int samples, int sample_rate, double* phase, int frequency, float amplitude);
void FR_Noise(float* buffer, int samples, int sample_rate, float amplitude);
void FR_AttenuatedNoise(float* buffer, int samples, int sample_rate, double* phase, float duration, float amplitude);
void FR_ClearBuffer(float* buffer, int samples);
