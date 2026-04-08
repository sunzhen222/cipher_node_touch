
#include "ui_msg.h"

#ifdef SIMULATOR
#include "stdlib.h"
#include "SDL2/SDL.h"
#else
#include "user_msg.h"
#include "user_assert.h"
#endif

#ifdef SIMULATOR
void SendUiMsg(uint32_t code, const void *data, uint32_t dataLen)
{
    SDL_Event event;
    void *sendData = NULL;
    if (data != NULL) {
        sendData = malloc(dataLen);
        memcpy(sendData, data, dataLen);
    }
    event.user.type = SDL_USEREVENT;
    event.user.code = code;
    event.user.data1 = sendData;
    event.user.data2 = (void *)(uintptr_t)dataLen;
    SDL_PushEvent(&event);
}
#else
void SendUiMsg(uint32_t code, const void *data, uint32_t dataLen)
{
    if (data == NULL) {
        PubValueMsg(UI_MSG_USER_EVENT, code);
    } else {
        ASSERT(dataLen > 0);
        PubValueBufferMsg(UI_MSG_USER_EVENT, code, data, dataLen);
    }
}

#endif
