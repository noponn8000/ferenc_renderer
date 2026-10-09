#include "font_bdf.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_CP        32
#define MAX_CP        0x10FFFF
#define MAX_ATLAS_W   256    // target atlas width in pixels; rows wrap past this

#define ATLAS_INDEX(f, cols, glyph, x, y)                                   \
    (((glyph) / (cols) * (f)->glyph_height + (y)) * (f)->tex_width +        \
     ((glyph) % (cols)) * (f)->glyph_width + (x))

typedef struct { uint32_t cp; bool *cell; } Pending;

static int hexval(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static int cmp_pending(const void *a, const void *b) {
    uint32_t x = ((const Pending *)a)->cp, y = ((const Pending *)b)->cp;
    return (x > y) - (x < y);
}

static void free_pending(Pending *list, size_t count) {
    for (size_t i = 0; i < count; i++) free(list[i].cell);
    free(list);
}

bool FU_FontReadBDF(Font *out, const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return false;

    Pending *list = NULL;
    size_t count = 0, cap = 0;

    int cw = 0, ch = 0, bbx_xoff = 0, bbx_yoff = 0;
    bool have_bbox = false;

    char line[512];
    long enc = -1;
    int gw = 0, gh = 0, gx = 0, gy = 0;
    bool in_bitmap = false;
    int row = 0;
    bool *cell = NULL;

    while (fgets(line, sizeof line, fp)) {
        if (in_bitmap) {
            if (strncmp(line, "ENDCHAR", 7) == 0) {
                in_bitmap = false; cell = NULL; enc = -1;
                continue;
            }
            if (!cell || row >= gh) continue;

            for (int col = 0; col < gw; col++) {
                int hi = hexval(line[2 * (col / 8)]);
                int lo = hi < 0 ? -1 : hexval(line[2 * (col / 8) + 1]);
                if (hi < 0 || lo < 0) break;
                if (!(((hi * 16 + lo) >> (7 - col % 8)) & 1)) continue;

                int cx = gx - bbx_xoff + col;
                int cy = (ch + bbx_yoff) - (gy + gh) + row;
                if (cx >= 0 && cx < cw && cy >= 0 && cy < ch)
                    cell[cy * cw + cx] = true;
            }
            row++;
            continue;
        }

        if (!have_bbox &&
            sscanf(line, "FONTBOUNDINGBOX %d %d %d %d",
                   &cw, &ch, &bbx_xoff, &bbx_yoff) == 4) {
            have_bbox = cw > 0 && ch > 0;
        } else if (strncmp(line, "STARTCHAR", 9) == 0) {
            enc = -1; cell = NULL;
        } else if (sscanf(line, "ENCODING %ld", &enc) == 1) {
            /* wait for BBX */
        } else if (sscanf(line, "BBX %d %d %d %d", &gw, &gh, &gx, &gy) == 4) {
            cell = NULL;
            if (have_bbox && enc >= MIN_CP && enc <= MAX_CP) {
                if (count == cap) {
                    cap = cap ? cap * 2 : 512;
                    Pending *p = realloc(list, cap * sizeof *p);
                    if (!p) { fclose(fp); free_pending(list, count); return false; }
                    list = p;
                }
                cell = calloc((size_t)cw * ch, sizeof(bool));
                if (!cell) { fclose(fp); free_pending(list, count); return false; }
                list[count].cp = (uint32_t)enc;
                list[count].cell = cell;
                count++;
            }
        } else if (strncmp(line, "BITMAP", 6) == 0) {
            in_bitmap = true; row = 0;
        }
    }
    fclose(fp);

    if (!have_bbox || count == 0) { free_pending(list, count); return false; }

    qsort(list, count, sizeof *list, cmp_pending);

    int n    = (int)count;
    int cols = MAX_ATLAS_W / cw;
    if (cols < 1) cols = 1;
    if (cols > n) cols = n;
    int rows = (n + cols - 1) / cols;

    bool     *tex   = calloc((size_t)cols * cw * rows * ch, sizeof(bool));
    char     *gstr  = malloc((size_t)n + 1);
    int      *lut   = malloc(128 * sizeof *lut);
    uint32_t *cps   = malloc((size_t)n * sizeof *cps);
    if (!tex || !gstr || !lut || !cps) {
        free(tex); free(gstr); free(lut); free(cps);
        free_pending(list, count);
        return false;
    }
    memset(lut, -1, 128 * sizeof *lut);   // -1 = no glyph

    out->tex_width    = cols * cw;
    out->tex_height   = rows * ch;
    out->glyph_width  = cw;
    out->glyph_height = ch;
    out->n_glyphs     = n;
    out->fonttex      = tex;
    out->glyphs       = gstr;
    out->lookup       = lut;
    out->codepoints   = cps;

    for (int i = 0; i < n; i++) {
        uint32_t cp = list[i].cp;
        cps[i]  = cp;
        gstr[i] = cp < 128 ? (char)cp : '?';
        if (cp < 128) lut[cp] = i;

        for (int y = 0; y < ch; y++)
            for (int x = 0; x < cw; x++)
                tex[ATLAS_INDEX(out, cols, i, x, y)] = list[i].cell[y * cw + x];
    }
    gstr[n] = '\0';

    free_pending(list, count);
    return true;
}

void FU_FontFreeBDF(Font *font) {
    free(font->fonttex);
    free((void *)font->glyphs);
    free((void *)font->lookup);
    free((void *)font->codepoints);
    memset(font, 0, sizeof *font);
}
