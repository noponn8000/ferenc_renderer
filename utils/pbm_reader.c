#include "pbm_reader.h"

#define BUFFER_SIZE 2048

bool parseNetPBMHeader(char* buffer, int n_bytes, int* x, int* y, int* bodyStart, NETPBM_FILE_TYPE type) {
    if (n_bytes <= 3) return false;

    if (buffer[0] != 'P') return false;
    switch(type) {
        case P4:
            if (buffer[1] != '4') return false;
            break;
        case P6:
            if (buffer[1] != '6') return false;
            break;
        default:
            return false;
    }
    if (buffer[2] != '\n') return false;

    // Read image dimensions
    char digitBuffer[8];
    memset(digitBuffer, 0, 8);
    int start = 3;
    int i = 0;
    for (; buffer[start + i] != ' ' && start + i < n_bytes; i++) {
        digitBuffer[i] = buffer[start + i];
    }
    *x = atoi(digitBuffer);

    // Clear digit buffer
    memset(digitBuffer, 0, 8);
    start += i + 1;
    i = 0;
    for (; buffer[start + i] != '\n' && start + i < n_bytes; i++) {
        digitBuffer[i] = buffer[start + i];
    }
    *y = atoi(digitBuffer);

    // 0 image dimension - invalid
    if (x == 0 || y == 0) return false;

    start += i + 1;
    memset(digitBuffer, 0, 8);
    i = 0;
    for (; buffer[start + i] != '\n' && start +i < n_bytes; i++) {
        digitBuffer[i] = buffer[start + i];
    }
    int maxValue = atoi(digitBuffer);

    printf("Max color value: %d\n", maxValue);
    start += i + 1;

    // No body - invalid
    if (start >= n_bytes) return false;

    *bodyStart = start;
    return true;
}

uint32_t* readPPM(FILE* file, int* width, int* height) {
    assert(file != NULL);

    char buffer[16384];
    int x = 0; int y = 0;

    size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, file);
    printf("%u\n", bytesRead);
    int i;

    assert(parseNetPBMHeader(buffer, bytesRead, &x, &y, &i, P6));
    
    uint32_t* tex = calloc(x * y, sizeof(uint32_t));
    uint32_t color;
    int texIndex = 0;
    for (; i + 2 < bytesRead; i+=3) {
        color = 0;
        uint8_t r = buffer[i];
        uint8_t g = buffer[i + 1];
        uint8_t b = buffer[i + 2];

        if (!(r == 0 && b == 0 && g == 0)) {
            color |= r;
            color |= ((uint32_t) g) << 8;
            color |= ((uint32_t) b) << 16;
            color |= 0xFF000000;
        }

        tex[texIndex] = color;
        texIndex++;
    }

    *width = x; *height = y;
    return tex;
}

bool* readPBM(FILE* file, int* width, int* height) {
    assert(file != NULL);

    char buffer[2048];
    int x = 0; int y = 0;
    size_t bytesRead;

    bool *tex;
    bytesRead = fread(buffer, 1, sizeof(buffer) - 1, file);
    if (bytesRead > 0) {
        assert(buffer[0] == 'P');
        assert(buffer[1] == '4');
        assert(buffer[2] == '\n');

        // Read image dimensions
        char digitBuffer[8];
        memset(digitBuffer, 0, 8);
        int start = 3;
        int i = 0;
        for (; buffer[start + i] != ' '; i++) {
            digitBuffer[i] = buffer[start + i];
        }
        x = atoi(digitBuffer);

        // Clear digit buffer
        memset(digitBuffer, 0, 8);
        start += i + 1;
        i = 0;
        for (; buffer[start + i] != '\n'; i++) {
            digitBuffer[i] = buffer[start + i];
        }
        start += i + 1;
        y = atoi(digitBuffer);

        assert((x >= 0) && (y >= 0));

        // Initialize array representing the texture
        tex = (bool*) calloc(x * y, 1);

        i = start;
        int j = 0;
        for (; i < bytesRead; i++) {
            int mask = 1 << 7;
            for (int k = 0; k < 8; k++) {
                tex[j] = !(buffer[i] & mask);
                mask >>= 1;
                j++;
            }
        }

        *width = x;
        *height = y;
    }

    return tex;
}

