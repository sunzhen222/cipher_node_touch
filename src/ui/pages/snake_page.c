#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "ui_msg.h"
#include "navigation_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define SNAKE_GRID_SIZE          16
#define SNAKE_MAX_LENGTH         (SNAKE_GRID_SIZE * SNAKE_GRID_SIZE)
#define SNAKE_STEP_PERIOD_MS     300
#define SNAKE_INPUT_PERIOD_MS    5
#define SNAKE_BOARD_SIZE         288
#define SNAKE_CELL_SIZE          (SNAKE_BOARD_SIZE / SNAKE_GRID_SIZE)
#define SNAKE_SWIPE_THRESHOLD    24

typedef enum {
    SNAKE_DIR_UP,
    SNAKE_DIR_RIGHT,
    SNAKE_DIR_DOWN,
    SNAKE_DIR_LEFT,
} SnakeDirection_t;

typedef struct {
    int8_t x;
    int8_t y;
} SnakeCell_t;

typedef struct {
    lv_obj_t *scoreLabel;
    lv_obj_t *board;
    lv_obj_t *stateLabel;
    lv_timer_t *stepTimer;
    lv_timer_t *inputTimer;
    SnakeCell_t snake[SNAKE_MAX_LENGTH];
    SnakeCell_t food;
    lv_point_t pressStart;
    uint16_t length;
    uint16_t score;
    SnakeDirection_t direction;
    SnakeDirection_t nextDirection;
    bool trackingPress;
    bool gameOver;
    bool win;
} SnakePageValues_t;

static void SnakePageInit(void);
static void SnakePageDeinit(void);
static void SnakePageMsgHandler(uint32_t code, void *data, uint32_t dataLen);
static void SnakeBoardEventHandler(lv_event_t *e);
static void SnakeBoardDrawHandler(lv_event_t *e);
static void SnakeStepTimerHandler(lv_timer_t *timer);
static void SnakeInputTimerHandler(lv_timer_t *timer);
static void SnakeReset(SnakePageValues_t *values);
static void SnakeMove(SnakePageValues_t *values);
static void SnakeSetDirection(SnakePageValues_t *values, SnakeDirection_t direction);
static void SnakePlaceFood(SnakePageValues_t *values);
static bool SnakeCellOnBody(const SnakePageValues_t *values, SnakeCell_t cell, uint16_t length);
static bool SnakeIsReverse(SnakeDirection_t current, SnakeDirection_t next);
static void SnakeUpdateLabels(SnakePageValues_t *values);
static void SnakeHandleSwipe(SnakePageValues_t *values, int16_t dx, int16_t dy);

Page_t g_snakePage = {
    .init = SnakePageInit,
    .deinit = SnakePageDeinit,
    .msgHandler = SnakePageMsgHandler,
    .fullScreen = false,
};

static void SnakePageInit(void)
{
    SnakePageValues_t *values = SRAM_MALLOC(sizeof(SnakePageValues_t));
    memset(values, 0, sizeof(SnakePageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);

    CreateGeneralNavigationBar();

    lv_obj_clear_flag(GetPageBackground(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(GetPageBackground(), LV_OBJ_FLAG_SCROLL_CHAIN);

    values->scoreLabel = lv_label_create(GetPageBackground());
    lv_obj_align(values->scoreLabel, LV_ALIGN_TOP_MID, 0, 40);

    values->board = lv_obj_create(GetPageBackground());
    lv_obj_remove_style_all(values->board);
    lv_obj_set_size(values->board, SNAKE_BOARD_SIZE, SNAKE_BOARD_SIZE);
    lv_obj_align(values->board, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_set_style_bg_color(values->board, lv_color_hex(0x101820), 0);
    lv_obj_set_style_bg_opa(values->board, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(values->board, lv_color_hex(0x5D7285), 0);
    lv_obj_set_style_border_width(values->board, 2, 0);
    lv_obj_add_flag(values->board, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(values->board, LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_clear_flag(values->board, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(values->board, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_add_event_cb(values->board, SnakeBoardEventHandler, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(values->board, SnakeBoardEventHandler, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(values->board, SnakeBoardEventHandler, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(values->board, SnakeBoardEventHandler, LV_EVENT_PRESS_LOST, NULL);
    lv_obj_add_event_cb(values->board, SnakeBoardEventHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(values->board, SnakeBoardEventHandler, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(values->board, SnakeBoardDrawHandler, LV_EVENT_DRAW_MAIN, NULL);

    values->stateLabel = lv_label_create(GetPageBackground());
    lv_obj_align(values->stateLabel, LV_ALIGN_TOP_MID, 0, 382);

    SnakeReset(values);
    values->stepTimer = lv_timer_create(SnakeStepTimerHandler, SNAKE_STEP_PERIOD_MS, values);
    values->inputTimer = lv_timer_create(SnakeInputTimerHandler, SNAKE_INPUT_PERIOD_MS, values);
}

static void SnakePageDeinit(void)
{
    SnakePageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    if (values != NULL) {
        if (values->stepTimer != NULL) {
            lv_timer_delete(values->stepTimer);
        }
        if (values->inputTimer != NULL) {
            lv_timer_delete(values->inputTimer);
        }
        SRAM_FREE(values);
    }
}

static void SnakePageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(data);
    UNUSED(dataLen);
    UNUSED(code);
}

static void SnakeBoardEventHandler(lv_event_t *e)
{
    SnakePageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t *indev = lv_indev_active();

    if (values == NULL) {
        return;
    }

    if (code == LV_EVENT_CLICKED) {
        if (values->gameOver) {
            SnakeReset(values);
        }
        return;
    }

    if (indev == NULL) {
        return;
    }

    if (code == LV_EVENT_GESTURE) {
        lv_dir_t gestureDir = lv_indev_get_gesture_dir(indev);
        if (gestureDir == LV_DIR_TOP) {
            SnakeSetDirection(values, SNAKE_DIR_UP);
        } else if (gestureDir == LV_DIR_RIGHT) {
            SnakeSetDirection(values, SNAKE_DIR_RIGHT);
        } else if (gestureDir == LV_DIR_BOTTOM) {
            SnakeSetDirection(values, SNAKE_DIR_DOWN);
        } else if (gestureDir == LV_DIR_LEFT) {
            SnakeSetDirection(values, SNAKE_DIR_LEFT);
        }
    } else if (code == LV_EVENT_PRESSED) {
        lv_indev_get_point(indev, &values->pressStart);
    } else if (code == LV_EVENT_PRESSING) {
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        SnakeHandleSwipe(values, (int16_t)point.x - values->pressStart.x, (int16_t)point.y - values->pressStart.y);
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        SnakeHandleSwipe(values, (int16_t)point.x - values->pressStart.x, (int16_t)point.y - values->pressStart.y);
    }
}

static void SnakeBoardDrawHandler(lv_event_t *e)
{
    lv_obj_t *board = lv_event_get_target(e);
    lv_layer_t *layer = lv_event_get_layer(e);
    SnakePageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    lv_area_t boardCoords;
    lv_obj_get_coords(board, &boardCoords);

    lv_draw_rect_dsc_t bgDsc;
    lv_draw_rect_dsc_init(&bgDsc);
    bgDsc.bg_opa = LV_OPA_COVER;
    bgDsc.bg_color = lv_color_hex(0x101820);
    bgDsc.border_width = 0;
    lv_draw_rect(layer, &bgDsc, &boardCoords);

    lv_draw_rect_dsc_t foodDsc;
    lv_draw_rect_dsc_init(&foodDsc);
    foodDsc.bg_opa = LV_OPA_COVER;
    foodDsc.bg_color = lv_color_hex(0xFF5A5F);
    foodDsc.radius = 4;
    foodDsc.border_width = 0;

    lv_area_t cellArea;
    cellArea.x1 = boardCoords.x1 + values->food.x * SNAKE_CELL_SIZE + 3;
    cellArea.y1 = boardCoords.y1 + values->food.y * SNAKE_CELL_SIZE + 3;
    cellArea.x2 = cellArea.x1 + SNAKE_CELL_SIZE - 7;
    cellArea.y2 = cellArea.y1 + SNAKE_CELL_SIZE - 7;
    lv_draw_rect(layer, &foodDsc, &cellArea);

    lv_draw_rect_dsc_t snakeDsc;
    lv_draw_rect_dsc_init(&snakeDsc);
    snakeDsc.bg_opa = LV_OPA_COVER;
    snakeDsc.bg_color = lv_color_hex(0x3DDC84);
    snakeDsc.radius = 3;
    snakeDsc.border_width = 0;

    for (uint16_t i = 0; i < values->length; i++) {
        snakeDsc.bg_color = (i == 0) ? lv_color_hex(0xC6F91F) : lv_color_hex(0x3DDC84);
        cellArea.x1 = boardCoords.x1 + values->snake[i].x * SNAKE_CELL_SIZE + 2;
        cellArea.y1 = boardCoords.y1 + values->snake[i].y * SNAKE_CELL_SIZE + 2;
        cellArea.x2 = cellArea.x1 + SNAKE_CELL_SIZE - 5;
        cellArea.y2 = cellArea.y1 + SNAKE_CELL_SIZE - 5;
        lv_draw_rect(layer, &snakeDsc, &cellArea);
    }
}

static void SnakeStepTimerHandler(lv_timer_t *timer)
{
    SnakePageValues_t *values = lv_timer_get_user_data(timer);
    if (values == NULL || values->gameOver) {
        return;
    }

    SnakeMove(values);
    SnakeUpdateLabels(values);
    lv_obj_invalidate(values->board);
}

static void SnakeInputTimerHandler(lv_timer_t *timer)
{
    SnakePageValues_t *values = lv_timer_get_user_data(timer);
    lv_indev_t *indev = NULL;

    if (values == NULL) {
        return;
    }

    while ((indev = lv_indev_get_next(indev)) != NULL) {
        if (lv_indev_get_type(indev) != LV_INDEV_TYPE_POINTER) {
            continue;
        }

        if (lv_indev_get_state(indev) == LV_INDEV_STATE_PRESSED) {
            lv_point_t point;
            lv_indev_get_point(indev, &point);

            if (!values->trackingPress) {
                values->pressStart = point;
                values->trackingPress = true;
            } else {
                int16_t dx = (int16_t)(point.x - values->pressStart.x);
                int16_t dy = (int16_t)(point.y - values->pressStart.y);
                SnakeDirection_t oldNextDirection = values->nextDirection;
                SnakeHandleSwipe(values, dx, dy);
                if (oldNextDirection != values->nextDirection) {
                    values->pressStart = point;
                }
            }
            return;
        }
    }

    values->trackingPress = false;
}

static void SnakeReset(SnakePageValues_t *values)
{
    values->length = 3;
    values->score = 0;
    values->direction = SNAKE_DIR_RIGHT;
    values->nextDirection = SNAKE_DIR_RIGHT;
    values->trackingPress = false;
    values->gameOver = false;
    values->win = false;

    values->snake[0] = (SnakeCell_t) {
        7, 8
    };
    values->snake[1] = (SnakeCell_t) {
        6, 8
    };
    values->snake[2] = (SnakeCell_t) {
        5, 8
    };
    values->food = (SnakeCell_t) {
        10, 8
    };
    SnakeUpdateLabels(values);
    lv_obj_invalidate(values->board);
}

static void SnakeMove(SnakePageValues_t *values)
{
    SnakeCell_t newHead = values->snake[0];
    bool grow;
    uint16_t collisionLength;

    values->direction = values->nextDirection;

    if (values->direction == SNAKE_DIR_UP) {
        newHead.y--;
    } else if (values->direction == SNAKE_DIR_RIGHT) {
        newHead.x++;
    } else if (values->direction == SNAKE_DIR_DOWN) {
        newHead.y++;
    } else {
        newHead.x--;
    }

    grow = (newHead.x == values->food.x && newHead.y == values->food.y);
    collisionLength = grow ? values->length : values->length - 1;

    if (newHead.x < 0 || newHead.x >= SNAKE_GRID_SIZE ||
            newHead.y < 0 || newHead.y >= SNAKE_GRID_SIZE ||
            SnakeCellOnBody(values, newHead, collisionLength)) {
        values->gameOver = true;
        return;
    }

    if (grow && values->length >= SNAKE_MAX_LENGTH) {
        values->gameOver = true;
        values->win = true;
        return;
    }

    uint16_t shiftStart = grow ? values->length : values->length - 1;
    for (uint16_t i = shiftStart; i > 0; i--) {
        values->snake[i] = values->snake[i - 1];
    }
    values->snake[0] = newHead;

    if (grow) {
        values->length++;
        values->score++;
        SnakePlaceFood(values);
    }
}

static void SnakeSetDirection(SnakePageValues_t *values, SnakeDirection_t direction)
{
    if (!SnakeIsReverse(values->direction, direction)) {
        values->nextDirection = direction;
    }
}

static void SnakePlaceFood(SnakePageValues_t *values)
{
    SnakeCell_t cell;

    do {
        cell.x = (int8_t)lv_rand(0, SNAKE_GRID_SIZE - 1);
        cell.y = (int8_t)lv_rand(0, SNAKE_GRID_SIZE - 1);
    } while (SnakeCellOnBody(values, cell, values->length));

    values->food = cell;
}

static bool SnakeCellOnBody(const SnakePageValues_t *values, SnakeCell_t cell, uint16_t length)
{
    for (uint16_t i = 0; i < length; i++) {
        if (values->snake[i].x == cell.x && values->snake[i].y == cell.y) {
            return true;
        }
    }
    return false;
}

static bool SnakeIsReverse(SnakeDirection_t current, SnakeDirection_t next)
{
    return (current == SNAKE_DIR_UP && next == SNAKE_DIR_DOWN) ||
           (current == SNAKE_DIR_DOWN && next == SNAKE_DIR_UP) ||
           (current == SNAKE_DIR_LEFT && next == SNAKE_DIR_RIGHT) ||
           (current == SNAKE_DIR_RIGHT && next == SNAKE_DIR_LEFT);
}

static void SnakeUpdateLabels(SnakePageValues_t *values)
{
    char scoreText[24];
    snprintf(scoreText, sizeof(scoreText), "Score: %u", values->score);
    lv_label_set_text(values->scoreLabel, scoreText);

    if (values->gameOver) {
        lv_label_set_text(values->stateLabel, values->win ? "You win. Tap board." : "Game over. Tap board.");
    } else {
        lv_label_set_text(values->stateLabel, "Swipe to steer");
    }
}

static void SnakeHandleSwipe(SnakePageValues_t *values, int16_t dx, int16_t dy)
{
    if (dx > SNAKE_SWIPE_THRESHOLD || dx < -SNAKE_SWIPE_THRESHOLD ||
            dy > SNAKE_SWIPE_THRESHOLD || dy < -SNAKE_SWIPE_THRESHOLD) {
        if ((dx < 0 ? -dx : dx) > (dy < 0 ? -dy : dy)) {
            SnakeSetDirection(values, dx > 0 ? SNAKE_DIR_RIGHT : SNAKE_DIR_LEFT);
        } else {
            SnakeSetDirection(values, dy > 0 ? SNAKE_DIR_DOWN : SNAKE_DIR_UP);
        }
    }
}
