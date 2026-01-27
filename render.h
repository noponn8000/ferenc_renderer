#include <stdint.h>

// Color
uint32_t FR_RGBAToAGBR8888(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

// Utils
void FR_RotatePointI(int *x, int *y, int xc, int yc, float angle);

// - - - - - - - - - - - - Drawing
// - - - - - - Bresenham line drawing
// - - - Two helper functions. Internal use only.
void DrawLineLow(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x0, int y0, int x1, int y1, uint32_t color);
void DrawLineHigh(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x0, int y0, int x1, int y1, uint32_t color);
// - - - API function for external use.
void FR_DrawLine(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x0, int y0, int x1, int y1, uint32_t color);

void FR_DrawPoint(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x, int y, uint32_t color);
void FR_DrawCircle(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x, int y, uint16_t radius, uint32_t color);
void FR_DrawCircleFill(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x, int y, uint16_t radius, uint32_t color);
void FR_DrawCenteredRect(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x_tl, int y_tl, int width, int height, uint32_t color);
void FR_DrawCenteredRectRotated(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x_tl, int y_tl, int width, int height, float angle, uint32_t color);
void FR_DrawRegularPolyRotated(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int xc, int yc, int radius, uint8_t segments, float angle, uint32_t color);
void FR_DrawRect(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x_tl, int y_tl, int width, int height, uint32_t color);
void FR_DrawTriangle(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void FR_DrawTriangleFill(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
