#include "audio.h"

void FA_ClearBuffer(float* buffer, int samples) {
    memset(buffer, 0, samples * sizeof(float));
}

void FA_Sine(float* buffer, int samples, int sample_rate, double* phase, int frequency, float amplitude) {
    double dPhi = (2 * M_PI * frequency)/sample_rate;
    for (int i = 0; i < samples; i++) {
        buffer[i] += amplitude * sin(*phase);
        *phase += dPhi;
    }

    *phase = fmod(*phase, 2 * M_PI);
}

void FA_SineAttenuated(float* buffer, int samples, int sample_rate, double* phase, int frequency, double* amplitude, double targetAmplitude, double attenuationRate) {
    double dAmp = attenuationRate / sample_rate;
    bool increasing =  (targetAmplitude >= *amplitude ? true : false);

    double dPhi = (2 * M_PI * frequency)/sample_rate;
    for (int i = 0; i < samples; i++) {
        if (increasing && *amplitude + dAmp <= targetAmplitude) {
            *amplitude += dAmp;
        }
        else if (!increasing && *amplitude - dAmp >= targetAmplitude) {
            *amplitude -= dAmp;
        } else {
            *amplitude = targetAmplitude;
        }

        buffer[i] += *amplitude * sin(*phase);
        *phase += dPhi;
    }

    *phase = fmod(*phase, 2 * M_PI);
}

void FA_Noise(float* buffer, int samples, int sample_rate, float amplitude) {
    for (int i = 0; i < samples; i++) {
        float random = 2 * (((float) rand() / RAND_MAX) - 0.5f);
        buffer[i] += amplitude * random;
    }
}

void FA_AttenuatedNoise(float* buffer, int samples, int sample_rate, double* phase, float duration, float amplitude) {
    // The whole signal has been rendered
    if (*phase > 1.0) return;

    double dPhi = 1.0 / (duration * sample_rate);
    for (int i = 0; i < samples; i++) {
        float random = 2 * (((float) rand() / RAND_MAX) - 0.5f);
        buffer[i] += amplitude * cos(M_PI * (*phase)) * random;
        *phase += dPhi;
    }
}


uint64_t readVRQ(uint8_t** buffer, uint8_t* bufferEnd)
{
    uint32_t value = 0;
    uint8_t c;

    do {
        c = *(*buffer)++;
        value = (value << 7) | (c & 0x7F);
    } while (c & 0x80 && *buffer < bufferEnd);

    return value;
}

MIDIEventArray* FA_readMIDI(FILE* file, int* numberOfTracks) {
    assert(file != NULL);

    uint8_t buffer[65536];
    size_t bytesRead;

    bytesRead = fread(buffer, 1, sizeof(buffer), file);
    assert(bytesRead >= 0);

    // Parse header
    
    uint8_t* b = buffer;
    uint8_t* buffer_end = buffer + bytesRead;

    uint32_t format = 0;
    uint32_t ntrks = 0;
    uint32_t division = 0;
    bool deltaTimeFlag = 0;
    uint16_t ticksPerQuarter = 0;

    // 0x4d546864 == MThd
    uint32_t midiHeader = 0;
    for (int i = 0; i < 4 && b < buffer_end; i++) {
        midiHeader |= ((uint32_t) *b) << (8 * i);
        b++;
    }
    midiHeader = bswap_32(midiHeader);
    assert(midiHeader == 0x4d546864);
    
    for (int i = 0; i < 2 && b < buffer_end; i++) {
        format |= ((uint16_t) *b) << (8 * i);
        b++;
    }
    format = bswap_16(format);

    b += 4;

    for (int i = 0; i < 2 && b < buffer_end; i++) {
        ntrks |= ((uint16_t) *b) << (8 * i);
        b++;
    }
    ntrks = bswap_16(ntrks);

    for (int i = 0; i < 2 && b < buffer_end; i++) {
        division |= ((uint16_t) *b) << (8 * i);
        b++;
    }
    division = bswap_16(division);

    deltaTimeFlag = (1 << 15) & division;
    if (deltaTimeFlag) {
        printf("File uses SMPTE - aborting!\n");
    } else {
        printf("File uses tick/quarter note. All good.\n");
    }

    assert(!deltaTimeFlag);
    ticksPerQuarter = division;

    printf("Format: %u, Number of tracks: %u, Ticks per frame: %u\n", format, ntrks, ticksPerQuarter);

    MIDIEventArray* tracks = malloc(sizeof(MIDIEventArray));
    for (int i = 0; i < ntrks; i++) {
        // Parse track
        uint32_t chunkLength = 0;
        uint32_t trackHeader = 0;
        for (int i = 0; i < 4 && b < buffer_end; i++) {
            trackHeader |= ((uint32_t) *b) << (8 * i);
            b++;
        }
        trackHeader = bswap_32(trackHeader);
        // 0x4d54726b == MTrk
        assert(trackHeader == 0x4d54726b);

        for (int i = 0; i < 4 && b < buffer_end; i++) {
            chunkLength |= ((uint32_t) *b) << (8 * i);
            b++;
        }
        chunkLength = bswap_32(chunkLength);
        printf("Track length: %u\n", chunkLength);
        
        // Event loop

        // Time between the last event and the current event
        uint64_t delta; 
        // Ticks elapsed from the start of the track
        uint64_t ticks = 0;

        uint8_t status;
        uint8_t running_status = 0;

        uint8_t channel;
        uint8_t pitch;
        uint8_t velocity;

        // Array to store all MIDI events we find
        MIDIEventArray events = {0};

        // Pointer to end of chunk
        uint8_t* chunkEnd = b + chunkLength;

        while (b < chunkEnd && b < buffer_end) {
            delta = readVRQ(&b, buffer_end);
            ticks += delta;

            // This is a status byte
            if (*b & 0x80) {
                status = *(b++);

                if (status < 0xF0) {
                    running_status = status;
                }
            } else {
                status = running_status;
            }

            // Meta event
            if (status == 0xFF) {
                running_status = 0;
                uint8_t meta_type = *(b++);

                // Track end
                if (meta_type == 0x2F) {
                    b++;
                    tracks[i] = events;
                    break;
                }
                uint64_t length = readVRQ(&b, buffer_end);

                //printf("Meta event found with code %u and length %lu\n", meta_type, length);
                b += length;
            }
            else if (status == 0xF0 || status == 0xF7) {
                running_status = 0;
                uint64_t length = readVRQ(&b, buffer_end);

                //printf("Sysex event found with length %lu\n", length);
                b += length;
            } else {
                // Normal event?
                uint8_t event_type = status & 0xF0;
                uint8_t channel = status & 0x0F;

                switch (event_type) {

                    case 0x80: // note off
                    {
                        pitch = *(b++);
                        velocity  = *(b++);

                        MIDIEvent ev = {
                            .type = NOTE_OFF,
                            .pitch = pitch,
                            .velocity = 0,
                            ticks
                        };
                        dappend(events, ev);

                        break;
                    }

                    case 0x90: // note on
                    {
                        pitch = *(b++);
                        velocity  = *(b++);

                        MIDIEvent ev = {
                            .type = velocity == 0 ? NOTE_OFF : NOTE_ON,
                            .pitch = pitch,
                            .velocity = velocity,
                            ticks
                        };
                        dappend(events, ev);

                        break;
                    }

                    case 0xA0: // poly aftertouch
                    {
                        pitch = *(b++);
                        uint8_t pressure = *(b++);
                        break;
                    }

                    case 0xB0: // control change
                    {
                        uint8_t controller = *(b++);
                        uint8_t value = *(b++);
                        break;
                    }

                    case 0xC0: // program change, 1 data byte
                    {
                        uint8_t program = *(b++);
                        break;
                    }

                    case 0xD0: // channel pressure, 1 data byte
                    {
                        uint8_t pressure = *(b++);
                        break;
                    }

                    case 0xE0: // pitch bend, 2 bytes, LSB first
                    {
                        uint8_t lsb = *(b++);
                        uint8_t msb = *(b++);
                        int value = (msb << 7) | lsb;
                        break;
                    }

                    default:
                        printf("Unknown MIDI event: 0x%X\n", status);
                        return tracks;
                }
            }
        }
        tracks[i] = events;
    }
    
    (*numberOfTracks) = ntrks;
    return tracks;
}

void FA_PlayWavetable(float* buffer, int samples, int sample_rate, Wavetable *wt) {
    FA_ClearBuffer(buffer, samples);
    double freq = wt->baseFrequency;
    // Only iterate over keys in the standard range of piano
    for (int i = 21; i <= 108; i++) {
      FA_SineAttenuated(buffer, samples, sample_rate, &wt->phases[i],
                        (int)freq, &wt->amplitudes[i],
                        wt->targetAmplitudes[i], wt->attenuationRate);

        freq *= EQUAL_TEMPERAMENT_RATIO;
    }
}

void FA_LowPass(float* buffer, int samples, float a) {
    float lastSample = buffer[0];
    float currentSample;
    for (int i = 1; i < samples; i++) {
        currentSample = buffer[i];
        buffer[i] = a * currentSample + (1 - a) * lastSample;

        lastSample = currentSample;
    }
}

void FA_FrequencyImpulseResponse(float* buffer, int samples, int n) {
    for (int i = n; i < samples; i++) {
        float sum = 0.0;
        for (int j = i - n; j <= i; j++) sum += buffer[j];
        buffer[i] = sum / n;
    }
}

// piodła
