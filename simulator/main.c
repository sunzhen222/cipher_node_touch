#include "SDL2/SDL.h"
#include "SDL2/SDL_thread.h"
#include <stdlib.h>
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "page.h"
#include "pages_declare.h"
#include "status_bar.h"
#include "lv_theme_pocket.h"
#include "lora_chat.h"
#include "mqtt_chat.h"

static SDL_Thread *g_tickThread = NULL;

static int TickThread(void *data)
{
    uint32_t last = SDL_GetTicks();
    (void)data;

    while (1) {
        uint32_t now = SDL_GetTicks();
        uint32_t delta = now - last;
        if (delta > 0) {
            lv_tick_inc(delta);
            last = now;
        }
        SDL_Delay(1);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    SDL_Event event;
    bool running = true;
    lv_display_t *disp;
    lv_theme_t *theme;

    (void)argc;
    (void)argv;

    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    disp = lv_display_get_default();
    theme = lv_theme_pocket_init(disp);
    lv_display_set_theme(disp, theme);

    LoraChatInit();
    MqttChatInit();
    TestLoraChat();
    TestMqttChat();

    g_tickThread = SDL_CreateThread(TickThread, "lv_tick", NULL);
    (void)g_tickThread;

    CreateStatusBar();
    EnterNewPage(&g_homePage);

    while (running) {
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEMOTION:
                SetTouchXY(event.motion.x, event.motion.y);
                break;
            case SDL_MOUSEBUTTONDOWN:
                SetTouchXY(event.button.x, event.button.y);
                SetTouchPressed(true);
                break;
            case SDL_MOUSEBUTTONUP:
                SetTouchXY(event.button.x, event.button.y);
                SetTouchPressed(false);
                break;
            case SDL_USEREVENT:
                HandleCurrentPageMsg((uint32_t)event.user.code,
                                     event.user.data1,
                                     (uint32_t)(uintptr_t)event.user.data2);
                HandleStatusBarMsg((uint32_t)event.user.code,
                                   event.user.data1,
                                   (uint32_t)(uintptr_t)event.user.data2);
                if (event.user.data1 != NULL) {
                    free(event.user.data1);
                }
                break;
            default:
                break;
            }
        }
        lv_timer_handler();
        SDL_Delay(5);
    }

    return 0;
}
