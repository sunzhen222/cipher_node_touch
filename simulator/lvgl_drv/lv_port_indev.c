#include "lv_port_indev.h"

static volatile bool g_touchPressed = false;
static volatile int32_t g_touchX = 0;
static volatile int32_t g_touchY = 0;

static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    static int32_t last_x = 0;
    static int32_t last_y = 0;
    (void)indev;

    if (g_touchPressed) {
        last_x = g_touchX;
        last_y = g_touchY;
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    data->point.x = last_x;
    data->point.y = last_y;
}

void lv_port_indev_init(void)
{
    lv_indev_t *indev_touchpad = lv_indev_create();
    lv_indev_set_type(indev_touchpad, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_touchpad, touchpad_read);
}

void SetTouchPressed(bool pressed)
{
    g_touchPressed = pressed;
}

void SetTouchXY(int32_t x, int32_t y)
{
    g_touchX = x;
    g_touchY = y;
}
