#include "pbm_reader.h"

#define BUFFER_SIZE 2048

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

