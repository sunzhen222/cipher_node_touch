
#ifndef _TILE_DOTS_H
#define _TILE_DOTS_H

#include "stdint.h"
#include "stdbool.h"
#include "lvgl.h"

lv_obj_t *CreateTileDots(lv_obj_t *parent, uint32_t tileCount);
void SetTileDots(lv_obj_t *obj, uint32_t index);
void DestroyTileDots(lv_obj_t *obj);

#endif
