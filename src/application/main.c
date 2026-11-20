#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <engine/engine.h>
#include <renderer/render.h>
#include <renderer/colors.h>
#include <math/fmath.h>

// SDL state
SDL_Window   *win;
SDL_Renderer *ren;
SDL_Texture  *scrtex;
uint32_t     *framebuffer;
SDL_AudioDeviceID dev;
bool running = true;

// Data
float theta;

// Scale factor canvas size -> window size
#define WINDOW_SCALE_FACTOR 1
#define CANVAS_HEIGHT 800
#define CANVAS_WIDTH 800

float MyPreFrame(void* self) {
    memset(framebuffer, 0xFFFBFBFB, CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(uint32_t)); // Clear framebuffer

	// Spinning cube in orthographic projection
	if (true) {
    	// viewport matrix
    	// canonical view volume -> screen space
    	float nx = CANVAS_WIDTH; float ny = CANVAS_HEIGHT;
    	mat4 m_vp = {
    		nx / 2,    0, 0, (nx - 1) / 2,
    		     0, ny/2, 0, (ny - 1) / 2,
    		     0,    0, 1, 			0,
    		     0,    0, 0, 			1
    	};

    	// Orthographic view volume
    	// Assuming the view volume looks at the negative z-axis, with the up-direction being the positive y-axis
        float l = -2.0f;
        float r =  2.0f;
        float b = -2.0f;
        float t =  2.0f;
        float n =  0.1f;
        float f = 10.0f;

    	vec3 cube_edges[] = {
        // Bottom face
        {-1, -1, -1}, { 1, -1, -1},
        { 1, -1, -1}, { 1, -1,  1},
        { 1, -1,  1}, {-1, -1,  1},
        {-1, -1,  1}, {-1, -1, -1},

        // Top face
        {-1,  1, -1}, { 1,  1, -1},
        { 1,  1, -1}, { 1,  1,  1},
        { 1,  1,  1}, {-1,  1,  1},
        {-1,  1,  1}, {-1,  1, -1},

        // Connecting edges
        {-1, -1, -1}, {-1,  1, -1},
        { 1, -1, -1}, { 1,  1, -1},
        { 1, -1,  1}, { 1,  1,  1},
        {-1, -1,  1}, {-1,  1,  1}
        };

		// spin faster in the x axis than y
        mat4 rot_x = {
            cos(theta * 2), 0, sin(theta * 2), 0,
            0, 1, 0, 0,
            -sin(theta * 2), 0, cos(theta * 2), 0,
            0, 0, 0, 1
        };

        mat4 rot_y = {
            1, 0, 0, 0,
            0, cos(theta), -sin(theta), 0,
            0, sin(theta), cos(theta), 0,
            0, 0, 0, 1
        };

		// Scale it with the sin function so that it shrinks and grows periodically
        mat4 scale = {
            1 + 0.5*sin(theta), 0, 0, 0,
            0, 1 + 0.5*sin(theta), 0, 0,
            0, 0, 1 + 0.5*sin(theta), 0,
            0, 0, 0, 1
        };

        mat4 m_model = mat4mat4mul(rot_x, rot_y);
        m_model = mat4mat4mul(scale, m_model);

    	mat4 m_orth = {
        	2/(r - l),       0,       0, -(r+l)/(r-l),
        	        0, 2/(t-b),       0, -(t+b)/(t-b),
        	        0,       0, 2/(n-f), -(n+f)/(n-f),
        	        0,       0,       0, 			1
    	};

    	mat4 m = mat4mat4mul(m_vp, m_orth);

    	for (size_t i = 0; i < (size_t) sizeof(cube_edges) / sizeof(vec3); i += 2) {
    		vec4 p = { vec3dc(cube_edges[i]), 1};
    		vec4 q = { vec3dc(cube_edges[i + 1]), 1};

    		p = mat4vec4mul(m_model, p);
    		q = mat4vec4mul(m_model, q);

    		p = mat4vec4mul(m, p);
    		q = mat4vec4mul(m, q);
    		FR_DrawLine(framebuffer, CANVAS_WIDTH, CANVAS_HEIGHT, p.x, p.y, q.x, q.y, MAGENTA);
    	}
	}

	// 2D homogenous example, barycentric coords
	if (false) {
    	int tx = 400 + 100 * sin(theta); int ty = 400 + 100 * sin(theta);
    	vec3 v1 = { -100, -100, 1 };
    	vec3 v2 = { 100, -100, 1 };
    	vec3 v3 = { 0, 100, 1 };

    	mat3 rotate_translate = {
        	cos(theta), -sin(theta), tx,
        	sin(theta), cos(theta),  ty,
    		0,			0,			 1
    	};

    	v1 = mat3vec3mul(rotate_translate, v1);
    	v2 = mat3vec3mul(rotate_translate, v2);
    	v3 = mat3vec3mul(rotate_translate, v3);
    	
        FR_DrawCircleFill(framebuffer, CANVAS_WIDTH, CANVAS_HEIGHT, 300, 300, 50.0f, MAGENTA);
        FR_DrawTriangle2DVC(framebuffer, CANVAS_WIDTH, CANVAS_HEIGHT, vec2dc(v1), vec2dc(v2), vec2dc(v3), RED, GREEN, BLUE);
	}
    
    SDL_Event e;
    Engine* eng = (Engine*) self;
    // Clear input buffer
    eng->input.event_counter = 0;

    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEMOTION:
                eng->input.mouseX = (int) e.motion.x / WINDOW_SCALE_FACTOR;
                eng->input.mouseY = (int) e.motion.y / WINDOW_SCALE_FACTOR;
                break;
            case SDL_MOUSEBUTTONDOWN:
                eng->input.mouseButton[e.button.button - 1] = true;
                break;
            case SDL_MOUSEBUTTONUP:
                eng->input.mouseButton[e.button.button - 1] = false;
                break;
            case SDL_KEYDOWN:
                if (eng->input.event_counter < INPUT_BUFFER_SIZE) {
                    InputEvent ev = { e.key.keysym.sym, true };
                    eng->input.events[eng->input.event_counter++] = ev;
                }
                break;
            case SDL_KEYUP:
                if (eng->input.event_counter < INPUT_BUFFER_SIZE) {
                    InputEvent ev = { e.key.keysym.sym, false };
                    eng->input.events[eng->input.event_counter++] = ev;
                }
                break;
        }
    }

    static struct timespec t_i;
	struct timespec t_f;
	clock_gettime(CLOCK_MONOTONIC_RAW, &t_f);

	double start = (double)t_i.tv_sec + (double)t_i.tv_nsec / 1e9;
	double end = (double)t_f.tv_sec + (double)t_f.tv_nsec / 1e9;
	float dt = (float)(end - start);

	{
    	theta += dt;
	};

	if (t_i.tv_sec == 0 || dt > 0.1f) dt = 0.016f;
	t_i = t_f;
    
    return dt;
}

void MyPostFrame(void* self, RenderContext rctx, AudioContext actx) {
    Engine* engine = (Engine*) self;
    // Upload framebuffer to SDL Texture
    void *tex_pixels;
    int pitch;
    SDL_LockTexture(scrtex, NULL, &tex_pixels, &pitch);
    
    uint8_t *dst = (uint8_t *)tex_pixels;
    uint8_t *src = (uint8_t *)framebuffer;
    for (int y = 0; y < CANVAS_HEIGHT; y++) {
        memcpy(dst, src, CANVAS_WIDTH * sizeof(uint32_t));
        dst += pitch;
        src += CANVAS_WIDTH * sizeof(uint32_t);
    }
    SDL_UnlockTexture(scrtex);

    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, scrtex, NULL, NULL);
    SDL_RenderPresent(ren);

    SDL_QueueAudio(
        dev,
        actx.output,
        actx.streams[0].n_samples * sizeof(float)
);
}

int main(void) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    win = SDL_CreateWindow("Engine Driver", SDL_WINDOWPOS_CENTERED,
                           SDL_WINDOWPOS_CENTERED, CANVAS_WIDTH * WINDOW_SCALE_FACTOR, CANVAS_HEIGHT * WINDOW_SCALE_FACTOR,
                           SDL_WINDOW_BORDERLESS);
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    scrtex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, CANVAS_WIDTH, CANVAS_HEIGHT);
    framebuffer = calloc(CANVAS_WIDTH * CANVAS_HEIGHT, sizeof(uint32_t));

    Engine engine = FE_InitEngine(MyPreFrame, MyPostFrame);
    RenderContext rctx = { framebuffer, CANVAS_WIDTH, CANVAS_HEIGHT };

    while (running) {
        FE_Loop(&engine, rctx, (AudioContext) {0});
    }

    // Cleanup
    FE_DestroyEngine(&engine);
    return 0;
}
