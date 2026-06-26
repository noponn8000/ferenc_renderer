#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "utils/textbox.h"
#include "utils/midiplayer.h"
#include "player.h"
#include "utils/array.h"

// SDL state
SDL_Window   *win;
SDL_Renderer *ren;
SDL_Texture  *scrtex;
uint32_t     *framebuffer;
SDL_AudioDeviceID dev;

// Scale factor canvas size -> window size
#define WINDOW_SCALE_FACTOR 3

typedef darray(int) arri;

float MyPreFrame(void* self) {
    memset(framebuffer, 0x12, 400 * 300 * sizeof(uint32_t)); // Clear framebuffer
    
    SDL_Event e;
    Engine* eng = (Engine*) self;
    // Clear input buffer
    eng->input.event_counter = 0;

    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                exit(0);
                break;
            case SDL_MOUSEMOTION:
                eng->input.mouseX = (int) e.motion.x / WINDOW_SCALE_FACTOR;
                eng->input.mouseY = (int) e.motion.y / WINDOW_SCALE_FACTOR;
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
    for (int y = 0; y < 300; y++) {
        memcpy(dst, src, 400 * sizeof(uint32_t));
        dst += pitch;
        src += 400 * sizeof(uint32_t);
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
    FILE* midiFile = fopen("res/test3.mid", "rb");
    MIDIEventArray events = FA_readMIDI(midiFile);
    Wavetable wt = {
        .baseFrequency = 28,
        .attenuationRate = 10.0
    };

    Entity midi = MIDIPlayerConstruct(&wt, events, 960, 1);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    win = SDL_CreateWindow("Engine Driver", SDL_WINDOWPOS_CENTERED,
                           SDL_WINDOWPOS_CENTERED, 1200, 900,
                           SDL_WINDOW_BORDERLESS);
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    scrtex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 400, 300);
    framebuffer = calloc(400 * 300, sizeof(uint32_t));

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
    RenderContext rctx = { framebuffer, 400, 300 };
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

    Entity player = PlayerConstruct();
    Entity textbox = TextboxConstruct(
        position, size, margin, glyphSpacing, str         
    );
    
    //FE_AddEntity(&engine, player);
    FE_AddEntity(&engine, textbox);
    FE_AddEntity(&engine, midi);

    SDL_PauseAudioDevice(dev, 0);
    while (true) {
        FE_Loop(&engine, rctx, actx);
        actx.t += (float) actx.streams[0].n_samples / actx.sample_rate;
    }

    // Cleanup
    FE_DestroyEngine(&engine);
    return 0;
}
