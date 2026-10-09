#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "../utils/textbox.h"
#include "../utils/midiplayer.h"
#include "../utils/array.h"
#include "../utils/font_bdf.h"
#include "../ui/button.h"

// SDL state
SDL_Window   *win;
SDL_Renderer *ren;
SDL_Texture  *scrtex;
uint32_t     *framebuffer;
SDL_AudioDeviceID dev;
bool running = true;

// Scale factor canvas size -> window size
#define WINDOW_SCALE_FACTOR 1
#define CANVAS_HEIGHT 800
#define CANVAS_WIDTH 800

typedef darray(int) arri;

float MyPreFrame(void* self) {
    memset(framebuffer, 0xFFFBFBFB, CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(uint32_t)); // Clear framebuffer
    
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

void OnButtonPressed() {
    printf("Pressed yahoo\n");
}

int main(void) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    win = SDL_CreateWindow("Engine Driver", SDL_WINDOWPOS_CENTERED,
                           SDL_WINDOWPOS_CENTERED, CANVAS_WIDTH * WINDOW_SCALE_FACTOR, CANVAS_HEIGHT * WINDOW_SCALE_FACTOR,
                           SDL_WINDOW_BORDERLESS);
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    scrtex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, CANVAS_WIDTH, CANVAS_HEIGHT);
    framebuffer = calloc(CANVAS_WIDTH * CANVAS_HEIGHT, sizeof(uint32_t));

    // Initialize audio
    SDL_AudioSpec desired = {0};
    desired.freq = 48000;
    desired.format = AUDIO_F32;
    desired.channels = 1;
    desired.samples = 896;
    desired.callback = NULL;

    dev =
        SDL_OpenAudioDevice(NULL, 0, &desired, NULL, 0);

    if (dev == 0) {
        printf("OpenAudioDevice failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }


    Engine engine = FE_InitEngine(MyPreFrame, MyPostFrame);
    RenderContext rctx = { framebuffer, CANVAS_WIDTH, CANVAS_HEIGHT };
    AudioContext actx = {
        .sample_rate = 48000
    }; 

    for (int i = 0; i < N_AUDIO_STREAMS; i++) {
        AudioStream s = {
            .frames = calloc(896, sizeof(float)),
            .volume = 0.1,
            .n_samples = 896
        };
        actx.streams[i] = s;
    }
    actx.output = calloc(896, sizeof(float));

    char* str = "You are a singing chalice. Revel in your new body, for you have been blessed. The nullity welcomes you into its lukewarm embrace.";
    Vector2i position = { 8, 8 };
    Vector2i size = { 46, 8 };
    Vector2i margin = { 8, 8 };
    Vector2i glyphSpacing = { 0, 4 };

    Font terminus12;
    FU_FontReadBDF(&terminus12, "res/ter-u12b.bdf");

    Entity textbox = TextboxConstruct(
        position, size, margin, glyphSpacing, 0xFF000000, 0xFF000000, false, str, terminus12
    );

    Vector2i buttonPosition = { 200, 200 };
    Vector2i buttonSize = { 100, 50 };
    Entity button = ButtonConstruct(buttonPosition, buttonSize, "Press me", 0xFF000000, 0xFF2020FF, 0xFF5050FF, OnButtonPressed);
    
    FE_AddEntity(&engine, textbox);
    FE_AddEntity(&engine, button);

    SDL_PauseAudioDevice(dev, 0);
    while (running) {
        FE_Loop(&engine, rctx, actx);
        actx.t += (float) actx.streams[0].n_samples / actx.sample_rate;
    }

    for (int i = 0; i < N_AUDIO_STREAMS; i++) {
        free(actx.streams[i].frames);
    }
    free(actx.output);

    FU_FontFreeBDF(&terminus12);

    // Cleanup
    FE_DestroyEngine(&engine);
    return 0;
}
