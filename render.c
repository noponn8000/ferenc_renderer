#include "render.h"
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265

uint32_t FR_RGBAToAGBR8888(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((uint32_t)a << 24) |
           ((uint32_t)b << 16) |
           ((uint32_t)g << 8)  |
           (uint32_t)r;
}

void FR_DrawPoint(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x, int y, uint32_t color) {
    if (x >= canvas_w || x < 0 || y >= canvas_h || y < 0) {
        // fprintf(stderr, "Wrong coordinate: (%u, %u)\n", x, y);
        return;
    }

    pixels[y * canvas_w + x] = color;
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
