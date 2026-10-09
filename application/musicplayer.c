#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "../utils/textbox.h"
#include "../utils/font_bdf.h"
#include "../cue/cuefile.h"
#include "../ui/button.h"

#include "../musicplayer/miniaudio.h"

// SDL state
SDL_Window   *win;
SDL_Renderer *ren;
SDL_Texture  *scrtex;
uint32_t     *framebuffer;
bool running = true;

// Scale factor canvas size -> window size
#define WINDOW_SCALE_FACTOR 1
#define CANVAS_HEIGHT 800
#define CANVAS_WIDTH 800

typedef struct {
    char dir[PATH_MAX];
    struct dirent **entries;
    int count;
} CueList;

static int IsCue(const struct dirent *e)
{
    const char *dot = strrchr(e->d_name, '.');
    return dot && strcasecmp(dot, ".cue") == 0;
}

int CueListScan(CueList *list, const char *dir)
{
    if (dir) {
        if (!realpath(dir, list->dir)) return -1;
    } else {
        if (!getcwd(list->dir, sizeof list->dir)) return -1;
    }
    list->count = scandir(list->dir, &list->entries, IsCue, alphasort);
    return list->count < 0 ? -1 : 0;
}

void CueListFree(CueList *list)
{
    for (int i = 0; i < list->count; i++) free(list->entries[i]);
    free(list->entries);
    list->entries = NULL;
    list->count = 0;
}

typedef struct {
    bool playing;
    char* rootDirectory;
    CueList fileList;
    CueFile currentFile;
    size_t currentFileIndex;
    size_t currentTrackIndex;
} MusicPlayerData;

/* Global */
MusicPlayerData* mp;

ma_result result;
ma_engine audioEngine;
ma_decoder decoder;
ma_sound sound;

void MusicPlayerInit(void *self) {
    TextboxData* data =  (TextboxData*) self;
}

void MusicPlayerDraw(void* self, RenderContext ctx) {
    MusicPlayerData* data =  (MusicPlayerData*) self;
}

void MusicPlayerUpdate(void* self, Input input, float dt) {
    MusicPlayerData* data =  (MusicPlayerData*) self;
}

void MusicPlayerRemove(void *self) {
    MusicPlayerData* data =  (MusicPlayerData*) self;
    free(data);
}

Entity MusicPlayerConstruct(char* directory) {
    mp = calloc(1, sizeof(MusicPlayerData));

    mp->rootDirectory = directory;
    mp->playing = false;
    mp->currentFileIndex = 0;
    mp->currentTrackIndex = 0;

    CueListScan(&(mp->fileList), directory);

    Entity musicPlayer = {
        .c_init = MusicPlayerInit,
        .c_update = MusicPlayerUpdate,
        .c_draw = MusicPlayerDraw,
        .c_remove = MusicPlayerRemove,
        .data = mp
    };

    return musicPlayer;
}

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
}

ma_uint64 CueToPCM(CueTimestamp t, ma_uint32 sampleRate) {
    ma_uint64 cdFrames = ((ma_uint64)t.mm * 60 + t.ss) * 75 + t.ff;
    return cdFrames * sampleRate / 75;
}

void OnPlayButtonPressed() {
    char path[512];
    snprintf(path, sizeof path, "%s/%s", mp->rootDirectory,
             mp->fileList.entries[mp->currentFileIndex]->d_name);

    // Stop any previous track first
    if (mp->playing) {
        ma_sound_uninit(&sound);
        mp->playing = false;
    }

    if (!FCUE_ParseCUE(path, &mp->currentFile)) {
        printf("failed to read cue\n");
        return;
    }

    CueTrackArray *tr = &mp->currentFile.tracks;
    CueTrack current = tr->items[mp->currentTrackIndex];

    printf("Track %d: %s, %u:%u\n", current.trackNumber, current.title, current.index01.mm, current.index01.ss);

    snprintf(path, sizeof path, "%s/%s", mp->rootDirectory, current.file);

    // Probe the file's native sample rate and length
    ma_decoder probe;
    if (ma_decoder_init_file(path, NULL, &probe) != MA_SUCCESS) {
        printf("failed to open %s\n", path);
        return;
    }
    ma_uint32 fileRate = probe.outputSampleRate;
    ma_uint64 fileLen = 0;
    ma_decoder_get_length_in_pcm_frames(&probe, &fileLen);
    ma_decoder_uninit(&probe);

    ma_uint64 start = CueToPCM(current.index01, audioEngine.sampleRate);
    ma_uint64 end;

    bool hasNext = mp->currentTrackIndex + 1 < (size_t)tr->count &&
                   strcmp(tr->items[mp->currentTrackIndex + 1].file, current.file) == 0;
    if (hasNext) end = CueToPCM(tr->items[mp->currentTrackIndex + 1].index01, audioEngine.sampleRate);
    else         end = fileLen;

    result = ma_sound_init_from_file(&audioEngine, path, MA_SOUND_FLAG_STREAM,
                                     NULL, NULL, &sound);
    if (result != MA_SUCCESS) {
        printf("failed to init sound, code %d\n", result);
        return;
    }

    ma_sound_seek_to_pcm_frame(&sound, start);

    ma_sound_start(&sound);
    mp->playing = true;
}

void OnNextTrackButtonPressed() {
    mp->currentTrackIndex++;
    OnPlayButtonPressed();
}

int main(void) {
    SDL_Init(SDL_INIT_VIDEO);
    win = SDL_CreateWindow("Engine Driver", SDL_WINDOWPOS_CENTERED,
                           SDL_WINDOWPOS_CENTERED, CANVAS_WIDTH * WINDOW_SCALE_FACTOR, CANVAS_HEIGHT * WINDOW_SCALE_FACTOR,
                           SDL_WINDOW_BORDERLESS);
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    scrtex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, CANVAS_WIDTH, CANVAS_HEIGHT);
    framebuffer = calloc(CANVAS_WIDTH * CANVAS_HEIGHT, sizeof(uint32_t));

    Engine engine = FE_InitEngine(MyPreFrame, MyPostFrame);
    RenderContext rctx = { framebuffer, CANVAS_WIDTH, CANVAS_HEIGHT };

    AudioContext actx = { 0 }; 

    char* str = "FERENC MUSIC PLAYER";
    Vector2i textboxPosition = { 140 , 20 };
    Vector2i textboxSize = { 80, 4 };
    Vector2i margin = { 8, 8 };
    Vector2i glyphSpacing = { 0, 4 };

    Font terminus12;
    FU_FontReadBDF(&terminus12, "res/ter-u12b.bdf");

    Entity textbox = TextboxConstruct(
        textboxPosition, textboxSize, margin, glyphSpacing, 0xFF000000, 0xFF000000, false, -1, str, terminus12
    );

    Vector2i buttonPosition = { 20, 20 };
    Vector2i buttonSize = { 100, 50 };
    Entity playButton = ButtonConstruct(buttonPosition, buttonSize, "Play", 0xFF000000, 0xFF2020FF, 0xFF5050FF, OnPlayButtonPressed);

    Vector2i nextTrackButtonPosition = { 20, 80 };
    Entity nextTrackButton = ButtonConstruct(nextTrackButtonPosition, buttonSize, "Next track", 0xFF000000, 0xFF2020FF, 0xFF5050FF, OnNextTrackButtonPressed);

    Entity musicPlayer = MusicPlayerConstruct("/home/ferenc/Music/Richard Strauss - Orchestral Works 9CD/CD7");
    
    FE_AddEntity(&engine, textbox);
    FE_AddEntity(&engine, playButton);
    FE_AddEntity(&engine, nextTrackButton);

    if (ma_engine_init(NULL, &audioEngine) != MA_SUCCESS) { printf("engine init failed\n"); return 1; }

    while (running) {
        FE_Loop(&engine, rctx, actx);
    }

    FU_FontFreeBDF(&terminus12);

    // Cleanup
    FE_DestroyEngine(&engine);
    return 0;
}
