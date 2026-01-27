#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PI 3.141592

#include "render.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 600

#define RENDER_WIDTH 400
#define RENDER_HEIGHT 300

#define X_CENTER RENDER_WIDTH / 2
#define Y_CENTER RENDER_HEIGHT / 2

SDL_Window   *win;
SDL_Renderer *ren;
SDL_Texture  *scrtex;

uint32_t *pixels;

int main(void)
{
    pixels = calloc(RENDER_WIDTH * RENDER_HEIGHT, sizeof(uint32_t));
    if (!pixels) {
        fprintf(stderr, "Failed to allocate framebuffer\n");
        return EXIT_FAILURE;
    }

    const uint32_t GRUVBOX_COLORS[8] = {
        FR_RGBAToAGBR8888(0xFB, 0x49, 0x34, 0xFF), // red
        FR_RGBAToAGBR8888(0xB8, 0xBB, 0x26, 0xFF), // green
        FR_RGBAToAGBR8888(0xFA, 0xBD, 0x2F, 0xFF), // yellow
        FR_RGBAToAGBR8888(0x83, 0xA5, 0x98, 0xFF), // blue
        FR_RGBAToAGBR8888(0xD3, 0x86, 0x9B, 0xFF), // purple
        FR_RGBAToAGBR8888(0x8E, 0xC0, 0x7C, 0xFF), // aqua
        FR_RGBAToAGBR8888(0xFE, 0x80, 0x19, 0xFF), // orange
        FR_RGBAToAGBR8888(0xA8, 0x99, 0x84, 0xFF)  // gray / fg
    };
    const uint32_t BG_COLOR = FR_RGBAToAGBR8888(0x12, 0x12, 0x12, 0xFF);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Failed to initialize video: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    win = SDL_CreateWindow(
        "Penguin",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!win) {
        fprintf(stderr, "Failed to open window: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    ren = SDL_CreateRenderer(
        win,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!ren) {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    /* Logical render size for pixel-perfect scaling */
    SDL_RenderSetLogicalSize(ren, RENDER_WIDTH, RENDER_HEIGHT);
    SDL_RenderSetIntegerScale(ren, SDL_TRUE);

    scrtex = SDL_CreateTexture(
        ren,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        RENDER_WIDTH,
        RENDER_HEIGHT
    );
    if (!scrtex) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    int running = 1;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
        }

        // Clear our framebuffer
        memset(pixels, BG_COLOR, RENDER_WIDTH * RENDER_HEIGHT * sizeof(uint32_t));


        for (int r = 100; r <= 150; r+=5) {
        FR_DrawRegularPolyRotated(
                pixels,
                RENDER_WIDTH,
                RENDER_HEIGHT,
                X_CENTER,
                Y_CENTER,
                r, 12,
                1.5,
                GRUVBOX_COLORS[3]
        );
        }

        FR_DrawCircleFill(
            pixels,
            RENDER_WIDTH,
            RENDER_HEIGHT,
            RENDER_WIDTH / 2,
            RENDER_HEIGHT / 2,
            40,
            GRUVBOX_COLORS[6]
        );

        FR_DrawCircleFill(
            pixels,
            RENDER_WIDTH,
            RENDER_HEIGHT,
            RENDER_WIDTH / 2,
            RENDER_HEIGHT / 2,
            20,
            GRUVBOX_COLORS[4]
        );

        const float dtheta = PI / 12;
        for (int i = 0; i < 24; i++) {
            int x1 = X_CENTER; int y1 = Y_CENTER + 100;
            FR_DrawCenteredRectRotated(
                pixels,
                RENDER_WIDTH,
                RENDER_HEIGHT,
                X_CENTER,
                Y_CENTER,
                X_CENTER,
                Y_CENTER,
                i * dtheta,
                GRUVBOX_COLORS[5]
            );

            FR_RotatePointI(&x1, &y1, X_CENTER, Y_CENTER, i * dtheta);
            FR_DrawLine(pixels, RENDER_WIDTH, RENDER_HEIGHT, X_CENTER, Y_CENTER, x1, y1, GRUVBOX_COLORS[0]);
        }

        void *tex_pixels;
        int pitch;

        SDL_LockTexture(scrtex, NULL, &tex_pixels, &pitch);

        uint8_t *dst = (uint8_t *)tex_pixels;
        uint8_t *src = (uint8_t *)pixels;

        for (int y = 0; y < RENDER_HEIGHT; y++) {
            memcpy(dst, src, RENDER_WIDTH * sizeof(uint32_t));
            dst += pitch;
            src += RENDER_WIDTH * sizeof(uint32_t);
        }

        SDL_UnlockTexture(scrtex);

        /* --- present --- */
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, scrtex, NULL, NULL);
        SDL_RenderPresent(ren);
    }

    /* --- cleanup --- */
    free(pixels);
    SDL_DestroyTexture(scrtex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}

