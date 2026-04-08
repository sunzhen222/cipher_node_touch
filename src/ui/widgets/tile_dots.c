#include "tile_dots.h"
#include "stdio.h"
#include "user_assert.h"
#include "user_memory.h"

#define DOT_SIZE                8
#define DOT_UNIT_WIDTH          16

typedef struct {
    uint32_t tileCount;
    lv_obj_t **dots;
} TileDotsValue_t;

lv_obj_t *CreateTileDots(lv_obj_t *parent, uint32_t tileCount)
{
    ASSERT(tileCount > 0 && tileCount < ((uint32_t)lv_display_get_horizontal_resolution(NULL) / DOT_UNIT_WIDTH) - 2);
    lv_obj_t *bg = lv_obj_create(parent);
    lv_obj_set_size(bg, lv_display_get_horizontal_resolution(NULL), DOT_SIZE);
    lv_obj_set_style_bg_color(bg, lv_color_black(), 0);

    TileDotsValue_t *tileDotsValue;
    tileDotsValue = SRAM_MALLOC(sizeof(TileDotsValue_t));
    tileDotsValue->tileCount = tileCount;
    tileDotsValue->dots = SRAM_MALLOC(sizeof(lv_obj_t *) * tileCount);
    uint32_t x = (lv_display_get_horizontal_resolution(NULL) - (tileCount - 1) * DOT_UNIT_WIDTH) / 2;
    printf("init x=%lu\n", x);
    for (uint32_t i = 0; i < tileCount; i++) {
        tileDotsValue->dots[i] = lv_obj_create(bg);
        lv_obj_set_size(tileDotsValue->dots[i], 8, 8);
        lv_obj_set_style_radius(tileDotsValue->dots[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(tileDotsValue->dots[i], lv_color_make(0xFF, 0xFF, 0xFF), 0);
        lv_obj_align(tileDotsValue->dots[i], LV_ALIGN_LEFT_MID, x, 0);
        lv_obj_set_style_bg_opa(tileDotsValue->dots[i], 0, 0);
        lv_obj_set_style_border_color(tileDotsValue->dots[i], lv_color_make(0xFF, 0xFF, 0xFF), 0);
        lv_obj_set_style_border_width(tileDotsValue->dots[i], 1, 0);
        x += DOT_UNIT_WIDTH;
    }
    lv_obj_set_style_bg_opa(tileDotsValue->dots[0], LV_OPA_COVER, 0);
    lv_obj_set_user_data(bg, tileDotsValue);
    return bg;
}

void SetTileDots(lv_obj_t *obj, uint32_t index)
{
    TileDotsValue_t *tileDotsValue = lv_obj_get_user_data(obj);
    ASSERT(index < tileDotsValue->tileCount);
    for (uint32_t i = 0; i < tileDotsValue->tileCount; i++) {
        if (i == index) {
            lv_obj_set_style_bg_opa(tileDotsValue->dots[i], LV_OPA_COVER, 0);
        } else {
            lv_obj_set_style_bg_opa(tileDotsValue->dots[i], 0, 0);
        }
    }
}

void DestroyTileDots(lv_obj_t *obj)
{
    TileDotsValue_t *tileDotsValue = lv_obj_get_user_data(obj);
    SRAM_FREE(tileDotsValue->dots);
    SRAM_FREE(tileDotsValue);
}

