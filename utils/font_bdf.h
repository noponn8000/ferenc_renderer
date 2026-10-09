#ifndef FONT_BDF_H
#define FONT_BDF_H
#include <stdbool.h>
#include "../renderer/render.h"

bool FU_FontReadBDF(Font *out, const char *path);  
void FU_FontFreeBDF(Font *font);                  
#endif
