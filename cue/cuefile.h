#include <stdint.h>

#include "cuetrack.h"
#include "../utils/array.h"

typedef darray(CueTrack) CueTrackArray;

typedef struct {
    char* genre;
    uint16_t date;
    char* discid;
    char* comment;
    char* performer;
    char* songwriter;
    char* title;
    CueTrackArray tracks;
} CueFile;

bool FCUE_ParseCUE(char* path, CueFile* cue);
uint8_t FCUE_ParseTimestamp(CueTimestamp* timestamp, char* stamp);
