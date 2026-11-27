#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool* fonttex;
    int tex_width;
    int tex_height;
    int glyph_width;
    int glyph_height;
    int n_glyphs;
    const char* glyphs;
    const int* lookup;
    const uint32_t* codepoints;
} Font;

// Maximum number of interpolated floating-point vertex attributes in a shader
#define FR_MAX_INTERPOLANTS 16

// Color
uint32_t FR_RGBAToAGBR8888(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

// - - - - - -Utils
// Clip a polygon (packed float array of vertices x,y,z,w) against a plane defined by n . (x - p) = 0 
int FR_ClipPolygon(const float *in, int nInput, float *out, int valuesPerVertex, float nx, float ny, float nz, float px, float py, float pz);

// Sampling
uint32_t FR_SampleNearest(uint32_t* tex, uint16_t tex_width, uint16_t tex_height, float u, float v);
float FR_SampleDepth(const float* depth_buffer, uint16_t canvas_w, uint16_t canvas_h, int x, int y);

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
void FR_DrawRectFill(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x_tl, int y_tl, int width, int height, uint32_t color);
void FR_DrawTriangle(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void FR_DrawTriangleFill(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void FR_DrawTriangle2DVC(uint32_t* pixels, uint16_t canvas_h, uint16_t canvas_w, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t c0, uint32_t c1, uint32_t c2);
void FR_DrawTriangle3DVC(uint32_t* pixels, float* depth_buffer, uint16_t canvas_w, uint16_t canvas_h,
                         float x0, float y0, float z0, float w0,
                         float x1, float y1, float z1, float w1,
                         float x2, float y2, float z2, float w2,
                         uint32_t c0, uint32_t c1, uint32_t c2
                         );
void FR_DrawTriangle3DUV(uint32_t* pixels, float* depth_buffer, uint16_t canvas_w, uint16_t canvas_h,
                         float x0, float y0, float z0, float w0,
                         float x1, float y1, float z1, float w1,
                         float x2, float y2, float z2, float w2,
                         float u0, float v0,
                         float u1, float v1,
                         float u2, float v2,
                         uint32_t* tex, uint16_t tex_width, uint16_t tex_height,
                         uint32_t (*sampler) (uint32_t*, uint16_t, uint16_t, float, float)
                         );
// Draw a triangle with a provided fragment shader
// `uniforms` is a void pointer so that fragment shaders can define their own struct for holding uniforms and then cast
// `in` is a float array with max size FR_MAX_INTERPOLANTS that holds vertex attributes
// those are passed for each vertex
void FR_DrawTriangle3DFS(uint32_t* pixels, float* depth_buffer, uint16_t canvas_w, uint16_t canvas_h,
                         float x0, float y0, float z0, float w0,
                         float x1, float y1, float z1, float w1,
                         float x2, float y2, float z2, float w2,
                         const float* var0, const float* var1, const float* var2,
                         int num_interpolants,
                         const void* uniforms,
                         uint32_t (*fs)(const void* uniforms, const float* in)
                         );
// Draw only the triangle's depth into a depth buffer
void FR_DrawTriangleDepth(float * depth_buffer, uint16_t canvas_w, uint16_t canvas_h,
                          float x0, float y0, float z0,
                          float x1, float y1, float z1,
                          float x2, float y2, float z2
                         );

// - - - - - - Font drawing
void FR_DrawLetter(uint32_t *pixels, uint16_t canvas_w, uint16_t canvas_h, int x, int y, char glyph, uint32_t color, Font font);
void FR_DrawText(uint32_t *pixels, uint16_t canvas_w, uint16_t canvas_h, int x_tl, int y_tl, int width, int height, int margin, int glyph_spacing, int line_spacing, uint32_t fg_color, char* text, Font font);

// - - - - - - Texture drawing
void FR_DrawBinaryTexture(uint32_t *pixels, uint16_t canvas_w, uint16_t canvas_h, int x_tl, int y_tl, bool* tex, int width, int height, uint32_t fg_color);
void FR_DrawBinarySpritesheet(uint32_t *pixels, uint16_t canvas_w, uint16_t canvas_h, int x_tl, int y_tl, bool* tex, int width, int height, int frames_x, int frames_y, int frame, uint32_t fg_color);
void FR_DrawAGBR8888Texture(uint32_t *pixels, uint16_t canvas_w, uint16_t canvas_h, int x_tl, int y_tl, uint32_t* tex, int width, int height);

// - - - - - - - - - - - - Post-processing
void FR_PostprocessDither(uint32_t* pixels, uint16_t canvas_w, uint16_t canvas_h, float dither_strength, int n_levels, bool jitter);
