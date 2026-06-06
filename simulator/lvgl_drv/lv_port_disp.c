#include "lv_port_disp.h"
#include <stdbool.h>
#include <stdio.h>
#include "SDL2/SDL.h"

#ifndef MY_DISP_HOR_RES
#define MY_DISP_HOR_RES 320
#endif

#ifndef MY_DISP_VER_RES
#define MY_DISP_VER_RES 480
#endif

static SDL_Window *g_window = NULL;
static SDL_Renderer *g_renderer = NULL;
static SDL_Texture *g_texture = NULL;
static volatile bool g_dispFlushEnabled = true;

static void disp_init(void);
static void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

void lv_port_disp_init(void)
{
    static lv_color_t buf_1[MY_DISP_HOR_RES * MY_DISP_VER_RES];

    disp_init();

    lv_display_t *disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush);
    lv_display_set_buffers(disp, buf_1, NULL, sizeof(buf_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
}

void disp_enable_update(void)
{
    g_dispFlushEnabled = true;
}

void disp_disable_update(void)
{
    g_dispFlushEnabled = false;
}

static void disp_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return;
    }

    g_window = SDL_CreateWindow("cipher_node_touch simulator",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                MY_DISP_HOR_RES,
                                MY_DISP_VER_RES,
                                0);
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE);
    SDL_PixelFormatEnum px_format;
#if LV_COLOR_DEPTH == 32
    px_format = SDL_PIXELFORMAT_ARGB8888;
#elif LV_COLOR_DEPTH == 24
    px_format = SDL_PIXELFORMAT_BGR24;
#elif LV_COLOR_DEPTH == 16
    px_format = SDL_PIXELFORMAT_RGB565;
#else
#error "Unsupported simulator LV_COLOR_DEPTH"
#endif

    g_texture = SDL_CreateTexture(g_renderer,
                                  px_format,
                                  SDL_TEXTUREACCESS_STREAMING,
                                  MY_DISP_HOR_RES,
                                  MY_DISP_VER_RES);
}

static void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    SDL_Rect r;

    if (g_dispFlushEnabled && g_texture != NULL) {
        r.x = area->x1;
        r.y = area->y1;
        r.w = area->x2 - area->x1 + 1;
        r.h = area->y2 - area->y1 + 1;
        SDL_UpdateTexture(g_texture, &r, px_map, r.w * ((LV_COLOR_DEPTH + 7) / 8));
        SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
        SDL_RenderPresent(g_renderer);
    }

    lv_display_flush_ready(disp);
}
