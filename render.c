#include "render.h"
#include <math.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define PI 3.14159265

const int BAYER_4X4[16] = {
    0,  8,  2, 10,
    12, 4, 14, 6,
    3, 11, 1,  9,
    15, 7, 13, 5
};

uint32_t FR_RGBAToAGBR8888(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((uint32_t)a << 24) |
           ((uint32_t)b << 16) |
           ((uint32_t)g << 8)  |
           (uint32_t)r;
}

void FR_AGBR8888ToRGBA(uint32_t agbr, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a)
{
    *r =  agbr        & 0xFF;
    *g = (agbr >> 8)  & 0xFF;
    *b = (agbr >> 16) & 0xFF;
    *a = (agbr >> 24) & 0xFF;
}

float FR_RGBAToLuminance(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        float rf = (float) r / 255.0;
        float gf = (float) g / 255.0;
        float bf = (float) b / 255.0;
        return 0.299 * rf + 0.587 * gf + 0.114 * bf;
}

void FR_DrawPoint(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x, int y, uint32_t color) {
    if (x >= canvas_w || x < 0 || y >= canvas_h || y < 0) {
        // fprintf(stderr, "Wrong coordinate: (%u, %u)\n", x, y);
        return;
    }

    // Alpha blend
    uint8_t ra, ga, ba, aa;
    uint8_t rb, gb, bb, ab;
    FR_AGBR8888ToRGBA(color, &ra, &ga, &ba, &aa);
    FR_AGBR8888ToRGBA(pixels[y * canvas_w + x], &rb, &gb, &bb, &ab);

    float af = (float) aa / 255;
    uint8_t r = ra * af + rb * (1.0 - af);
    uint8_t g = ga * af + gb * (1.0 - af);
    uint8_t b = ba * af + bb * (1.0 - af);
    pixels[y * canvas_w + x] = FR_RGBAToAGBR8888(r, g, b, 255);
}

void FR_SetPoint(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x, int y, uint32_t color) {
    if (x >= canvas_w || x < 0 || y >= canvas_h || y < 0) {
        // fprintf(stderr, "Wrong coordinate: (%u, %u)\n", x, y);
        return;
    }

    pixels[y * canvas_w + x] = color;
}

uint32_t FR_LookPoint(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x, int y) {
    if (x >= canvas_w || x < 0 || y >= canvas_h || y < 0) {
        return 0;
    }

    return pixels[y * canvas_w + x];
}

void FR_DrawCircleFill(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x, int y, uint16_t radius, uint32_t color) {
    int h;
    for (int xi = -radius; xi <= radius; xi++) {
        h = sqrt(radius * radius - xi * xi);

        for (int yi = -h; yi <= h; yi++) {
            FR_DrawPoint(pixels, canvas_w, canvas_h, x + xi, y + yi, color);
        }
    }
}

void FR_DrawCircle(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x, int y, uint16_t radius, uint32_t color) {
    int yi;
    for (int xi = 0; xi <= radius; xi++) {
        yi = sqrt(radius * radius - xi * xi);

        // Quadrant 1
        FR_DrawPoint(pixels, canvas_w, canvas_h, x + xi, y + yi, color);
        // Quadrant 2
        FR_DrawPoint(pixels, canvas_w, canvas_h, x - xi, y + yi, color);
        // Quadrant 3
        FR_DrawPoint(pixels, canvas_w, canvas_h, x - xi, y - yi, color);
        // Quadrant 4
        FR_DrawPoint(pixels, canvas_w, canvas_h, x + xi, y - yi, color);
    }
}

void FR_DrawRect(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x_tl, int y_tl, int width, int height, uint32_t color) {
    // Horizontal edges
    for (int xi = 0; xi <= width; xi++) {
        FR_DrawPoint(pixels, canvas_w, canvas_h, x_tl + xi, y_tl, color);
        FR_DrawPoint(pixels, canvas_w, canvas_h, x_tl + xi, y_tl + height, color);
    }

    for (int yi = 0; yi <= height; yi++) {
        FR_DrawPoint(pixels, canvas_w, canvas_h, x_tl, y_tl + yi, color);
        FR_DrawPoint(pixels, canvas_w, canvas_h, x_tl + width, y_tl + yi, color);
    }
}

void DrawLineHigh(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    int xi = 1;
    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }
    int D = 2 * dx - dy;

    int x = x0;
    for (int y = y0; y <= y1; y++) {
        FR_DrawPoint(pixels, canvas_w, canvas_h, x, y, color);
        if (D > 0) {
            x += xi;
            D += 2 * (dx - dy);
        } else {
            D += 2 * dx;
        }
    }
}

void DrawLineLow(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    int yi = 1;
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }
    int D = 2 * dy - dx;

    int y = y0;
    for (int x = x0; x <= x1; x++) {
        FR_DrawPoint(pixels, canvas_w, canvas_h, x, y, color);
        if (D > 0) {
            y += yi;
            D += 2 * (dy - dx);
        } else {
            D += 2 * dy;
        }
    }
}

void FR_DrawLine(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x0, int y0, int x1, int y1, uint32_t color) {
    if (abs(y1 - y0) < abs(x1 - x0)) {
        if (x0 > x1) {
            DrawLineLow(pixels, canvas_w, canvas_h, x1, y1, x0, y0, color);
        } else {
            DrawLineLow(pixels, canvas_w, canvas_h, x0, y0, x1, y1, color);
        }
    } else {
        if (y0 > y1) {
            DrawLineHigh(pixels, canvas_w, canvas_h, x1, y1, x0, y0, color);
        } else {
            DrawLineHigh(pixels, canvas_w, canvas_h, x0, y0, x1, y1, color);
        }
    }
}

void FR_DrawCenteredRect(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int xc, int yc, int width, int height, uint32_t color) {
    // Horizontal edges
    for (int xi = -width/2; xi <= width/2; xi++) {
        FR_DrawPoint(pixels, canvas_w, canvas_h, xc + xi, yc - height/2, color);
        FR_DrawPoint(pixels, canvas_w, canvas_h, xc + xi, yc + height/2, color);
    }

    for (int yi = -height/2; yi <= height/2; yi++) {
        FR_DrawPoint(pixels, canvas_w, canvas_h, xc - width/2, yc + yi, color);
        FR_DrawPoint(pixels, canvas_w, canvas_h, xc + width/2, yc + yi, color);
    }
}

void FR_DrawRegularPoly(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x, int y, uint16_t radius, uint8_t sides, uint32_t color) {
     
}

void FR_DrawCenteredRectRotated(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int xc, int yc, int width, int height, float angle, uint32_t color) {
    // Horizontal edges
    int x, y;
    for (int xi = -width/2; xi <= width/2; xi++) {
        x = xc + xi;
        y = yc - height / 2;
        FR_RotatePointI(&x, &y, xc, yc, angle);
        FR_DrawPoint(pixels, canvas_w, canvas_h, x, y, color);
        x = xc + xi;
        y = yc + height / 2;
        FR_RotatePointI(&x, &y, xc, yc, angle);
        FR_DrawPoint(pixels, canvas_w, canvas_h, x, y, color);
    }

    for (int yi = -height/2; yi <= height/2; yi++) {
        x = xc + width / 2;
        y = yc + yi;
        FR_RotatePointI(&x, &y, xc, yc, angle);
        FR_DrawPoint(pixels, canvas_w, canvas_h, x, y, color);
        x = xc - width / 2;
        y = yc + yi;
        FR_RotatePointI(&x, &y, xc, yc, angle);
        FR_DrawPoint(pixels, canvas_w, canvas_h, x, y, color);
    }
}

void FR_RotatePointI(int *x, int *y, int xc, int yc, float angle) {
    // Distances from center to point
    int dx = *x - xc;
    int dy = *y - yc;

    float c = cos(angle); float s = sin(angle);
    int xprime = dx * c - dy * s;
    int yprime = dy * c + dx * s;

    *x = xc + xprime;
    *y = yc + yprime;
}

void FR_RotatePointF(float *x, float *y, float xc, float yc, float angle) {
    float dx = *x - xc;
    float dy = *y - yc;

    float c = cos(angle); float s = sin(angle);
    float xprime = dx * c - dy * s;
    float yprime = dy * c + dx * s;

    *x = xc + xprime;
    *y = yc + yprime;
}

void FR_DrawTriangle(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    FR_DrawLine(pixels, canvas_w, canvas_w, x0, y0, x1, y1, color);
    FR_DrawLine(pixels, canvas_w, canvas_w, x1, y1, x2, y2, color);
    FR_DrawLine(pixels, canvas_w, canvas_w, x2, y2, x0, y0, color);
}

void FR_DrawTriangleFill(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // Order the vertices by y-value
    // thus P_0 = (x_0, y_0) is lowest y, P_2 = (x_2, y_2) is highest y.
    if (y1 < y0) {
        int xtemp = x0;
        int ytemp = y0;
        x0 = x1; y0 = y1;
        x1 = xtemp; y1 = ytemp;
    }
    if (y2 < y0) {
        int xtemp = x0;
        int ytemp = y0;
        x0 = x2; y0 = y2;
        x2 = xtemp; y2 = ytemp;
    }
    if (y2 < y1) {
        int xtemp = x1;
        int ytemp = y1;
        x1 = x2; y1 = y2;
        x2 = xtemp; y2 = ytemp;
    }

    float m01 = (y1 != y0) ? ((float) x1 - x0)/((float) y1 - y0) : 0;
    float m12 = (y2 != y1) ? ((float) x2 - x1)/((float) y2 - y1) : 0;
    float m02 = (y2 != y0) ? ((float) x2 - x0)/((float) y2 - y0) : 0;
    int xa;
    int xb;
    for (int yi = y0; yi <= y2; yi++) {
        if (yi <= y1 && y0 != y1) {
            xa = m01 * (yi - y0) + x0;
        } else {
            xa = m12 * (yi - y1) + x1;
        }
        xb = m02 * (yi - y0) + x0;

        if (xa > xb) {
            int xtemp = xa;
            xa = xb;
            xb = xtemp;
        }

        for (int xi = xa; xi <= xb; xi++) {
            FR_DrawPoint(pixels, canvas_w, canvas_h, xi, yi, color);
        }
    } 
}

void FR_DrawRegularPolyRotated(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int xc, int yc, int radius, uint8_t segments, float angle, uint32_t color) {
    float dtheta = 2 * PI / segments;
    for (int i = 0; i < segments; i++) {
        int xa = xc + radius * cos(dtheta * i + angle);
        int xb = xc + radius * cos(dtheta * (i + 1) + angle);
        int ya = yc + radius * sin(dtheta * i + angle);
        int yb = yc + radius * sin(dtheta * (i + 1) + angle);

        FR_DrawLine(pixels, canvas_w, canvas_h, xa, ya, xb, yb, color);
    }
}

void FR_PostprocessDither(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, float dither_strength, int n_levels, bool jitter) {
    int x_offset, y_offset = 0;

    for (int x = 0; x < canvas_w; x++) {
        for (int y = 0; y < canvas_h; y++) {
            if (jitter) { 
                x_offset = rand() & 3;
                x_offset = rand() & 3;
            }
            // Calculate Bayer matrix threshold
            float threshold = (BAYER_4X4[4 * ((y + y_offset) % 4) + ((x + x_offset) % 4)] + 0.5) / 16.0;

            // Convert AGBR8888 format to RGBA
            uint8_t r, g, b, a;
            FR_AGBR8888ToRGBA(FR_LookPoint(pixels, canvas_w, canvas_h, x, y), &r, &g, &b, &a);

            // Calculate luminance
            float luminance = FR_RGBAToLuminance(r, g, b, a);
            // Dither, clamp and quantise
            float luminance_dithered = luminance + (threshold - 0.5) * dither_strength;
            if (luminance_dithered < 0.0) luminance_dithered = 0.0; if (luminance_dithered > 1.0) luminance_dithered = 1.0;
            luminance_dithered = floor(luminance_dithered * n_levels) / n_levels;

            // Apply to original colors
            float scale = (luminance > 0.0001f) ? (luminance_dithered / luminance) : 0.0f;
            float rf = r / 255.0f;
            float gf = g / 255.0f;
            float bf = b / 255.0f;

            rf *= scale; gf *= scale; bf *= scale;
            r = (rf < 0) ? 0 : ((rf > 1.0) ? 255 : rf * 255);
            g = (gf < 0) ? 0 : ((gf > 1.0) ? 255 : gf * 255);
            b = (bf < 0) ? 0 : ((bf > 1.0) ? 255 : bf * 255);

            uint32_t agbr = FR_RGBAToAGBR8888(r, g, b, a);
            FR_SetPoint(pixels, canvas_w, canvas_h, x, y, agbr);
        }
    }
}

void FR_DrawLetter(uint32_t *pixels, uint8_t canvas_w, uint8_t canvas_h, int x, int y, char glyph, Font font) {
    int glyph_index = -1;
    for (int i = 0; i < font.n_glyphs; i++) {
        if (font.glyphs[i] == glyph) {
            glyph_index = i;
        }
    }

    if (glyph_index == -1) return;

    int glyphs_x = font.tex_width / font.glyph_width;
    int x_0 = glyph_index % glyphs_x
    int y_0 = glyph_index / glyphs_x;
}

uint32_t* LoadFontTexture(int *tex_width, int *tex_height, char const *font_filename) {
    int x; int y;
    unsigned char* pixels = stbi_load(font_filename, &x, &y, NULL, 4);
    uint32_t* texture = malloc(sizeof(uint32_t) * x * y);

    for (int i = 0; i < x * y; i++) {
        int r = pixels[4 * i];
        int g = pixels[4 * i + 1];
        int b = pixels[4 * i + 2];
        int a = pixels[4 * i + 3];
        uint32_t color = FR_RGBAToAGBR8888(r, g, b, a);
        texture[i] = color;
    }

    stbi_image_free(pixels);

    *tex_width = x;
    *tex_height = y;

    return texture;
}

