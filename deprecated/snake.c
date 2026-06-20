#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "engine.h"
#include "render.h"
#include "font.h"
#include "physics.h"

// SDL state
SDL_Window   *win;
SDL_Renderer *ren;
SDL_Texture  *scrtex;
uint32_t     *framebuffer;

int mouseX;
int mouseY;

#define N_SEGMENTS 8
typedef struct {
   Vector2f direction;
   int speed;
   float phase;
   Font font;
   Vector2f segments[N_SEGMENTS];
   int segmentRadii[N_SEGMENTS];
} SnakeData;


void SnakeInit(void* self) {
    SnakeData* data = (SnakeData*)self;
    for (int i = 0; i < N_SEGMENTS; i++) {
    	data->segmentRadii[i] = 5 + 2 * (N_SEGMENTS - i);
	data->segments[i].x = 40 - (4 * i);
	data->segments[i].y = 150;
    }
    data->direction.x = 1.0;
    data->direction.y = 0.0;
    data->speed = 30;
}

void SnakeUpdate(void* self, float dt) {
    SnakeData* data = (SnakeData*)self;
    data->phase += dt * 0.5;
    Vector2f headPos = data->segments[0];
    data->segments[0].x = headPos.x + data->direction.x * data->speed * dt;
    data->segments[0].y = headPos.y + data->direction.y * data->speed * dt;

    Vector2f toMouse = {
    	mouseX - data->segments[0].x,
    	mouseY - data->segments[0].y,
    };
    float length = sqrtf(toMouse.x * toMouse.x + toMouse.y * toMouse.y);
    data->direction.x = toMouse.x / length;
    data->direction.y = toMouse.y / length;
    
    for (int i = 1; i < N_SEGMENTS; i++) {
    Vector2f prev = data->segments[i-1];
    Vector2f curr = data->segments[i];

    Vector2f diff = { curr.x - prev.x, curr.y - prev.y };
    float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);

    if (dist > 0) {
        float targetDist = data->segmentRadii[i];
        data->segments[i].x = prev.x + (diff.x / dist) * targetDist;
        data->segments[i].y = prev.y + (diff.y / dist) * targetDist;
    }
}
}

void SnakeDraw(void* self, RenderContext ctx) {
    SnakeData* data = (SnakeData*)self;
    for (int i = 0; i < N_SEGMENTS; i++) {
    	const uint32_t fg = FR_RGBAToAGBR8888(0xFB - 5*i, 0x49 - 5*i, 0x34 - 5*i, 0xFF);
    	FR_DrawCircleFill(
		ctx.pixels, ctx.canvas_w, ctx.canvas_h,
		(int) data->segments[i].x, (int) data->segments[i].y,
		data->segmentRadii[i], fg
	);
    }
    FR_PostprocessDither(ctx.pixels, ctx.canvas_w, ctx.canvas_h, 0.5, 4, false);
}

float MyPreFrame(void* self) {
    memset(framebuffer, 0x12, 400 * 300 * sizeof(uint32_t)); // Dark BG
    
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) exit(0); 
    }
    SDL_GetMouseState(&mouseX, &mouseY);
    mouseX /= 3; mouseY /= 3;

    	static struct timespec t_i;
	struct timespec t_f;
	clock_gettime(CLOCK_MONOTONIC_RAW, &t_f);

	double start = (double)t_i.tv_sec + (double)t_i.tv_nsec / 1e9;
	double end = (double)t_f.tv_sec + (double)t_f.tv_nsec / 1e9;
	float dt = (float)(end - start);

	if (t_i.tv_sec == 0 || dt > 0.1f) dt = 0.016f; // Initial frame or spike
	t_i = t_f;
    
    return dt;
}

void MyPostFrame(void* self) {
    // Upload framebuffer to SDL Texture
    void *tex_pixels;
    int pitch;
    SDL_LockTexture(scrtex, NULL, &tex_pixels, &pitch);
    
    uint8_t *dst = (uint8_t *)tex_pixels;
    uint8_t *src = (uint8_t *)framebuffer;
    for (int y = 0; y < 300; y++) {
        memcpy(dst, src, 400 * sizeof(uint32_t));
        dst += pitch;
        src += 400 * sizeof(uint32_t);
    }
    SDL_UnlockTexture(scrtex);

    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, scrtex, NULL, NULL);
    SDL_RenderPresent(ren);
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    win = SDL_CreateWindow("Engine Driver", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 900, 0);
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    scrtex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 400, 300);
    framebuffer = calloc(400 * 300, sizeof(uint32_t));

    Engine engine = FE_InitEngine(MyPreFrame, MyPostFrame);
    RenderContext ctx = { framebuffer, 400, 300 };

    SnakeData mySnakeData = { .font = { fonttex, FONT_TEX_WIDTH, FONT_TEX_HEIGHT, FONT_GLYPH_WIDTH, FONT_GLYPH_HEIGHT, FONT_N_GLYPHS, FONT_GLYPH_SET, lookup } };
    Entity snake = {
        .c_init = SnakeInit,
        .c_update = SnakeUpdate,
        .c_draw = SnakeDraw,
        .data = &mySnakeData
    };
    
    FE_AddEntity(&engine, snake);

    while (true) {
        FE_Loop(&engine, ctx);
    }

    // Cleanup
    FE_DestroyEngine(&engine);
    return 0;
}
