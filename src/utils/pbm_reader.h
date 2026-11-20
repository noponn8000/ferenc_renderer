#pragma once

#include <stdbool.h>
#include<assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { P4, P6 } NETPBM_FILE_TYPE;

bool* readPBM(FILE* file, int* width, int* height);
uint32_t* readPPM(FILE* file, int* width, int* height);
