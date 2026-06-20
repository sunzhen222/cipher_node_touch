
#include "loading_spinner.h"
#include "user_memory.h"

#define SPINNER_ARC_WIDTH           8
#define SPINNER_ANIM_INTERVAL_MS    16
#define SPINNER_STEP_DEGREE         5
#define SPINNER_INCREASE_STEP       2

typedef struct {
    lv_obj_t *root;
    lv_obj_t *progressArc;
    lv_timer_t *animTimer;
    uint16_t angle;
    uint16_t length;
    bool lengthIncreasing;
} LoadingSpinnerValue_t;

static void SpinnerAnimTimerCb(lv_timer_t *timer);
static void SpinnerTimeoutCb(lv_timer_t *timer);
static void SpinnerDeleteEventCb(lv_event_t *e);


lv_obj_t *CreateLoadingSpinner(lv_obj_t *parent, uint32_t timeout)
{
    lv_obj_t *bg;
    lv_obj_t *progressArc;
    LoadingSpinnerValue_t *values;
    int32_t spinnerSize;

    values = SRAM_MALLOC(sizeof(LoadingSpinnerValue_t));
    values->angle = 0;
    values->length = 70;
    values->lengthIncreasing = true;

    bg = lv_obj_create(parent);
    lv_obj_set_size(bg, lv_obj_get_width(parent), lv_obj_get_height(parent));
    lv_obj_set_style_bg_color(bg, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(bg, LV_OPA_70, 0);
    lv_obj_set_style_radius(bg, 0, 0);
    lv_obj_set_style_border_width(bg, 0, 0);
    lv_obj_set_style_pad_all(bg, 0, 0);
    lv_obj_remove_flag(bg, LV_OBJ_FLAG_SCROLLABLE);

    spinnerSize = 48;

    progressArc = lv_arc_create(bg);
    lv_obj_set_size(progressArc, spinnerSize, spinnerSize);
    lv_obj_center(progressArc);
    lv_obj_remove_style(progressArc, NULL, LV_PART_KNOB);
    lv_obj_remove_flag(progressArc, LV_OBJ_FLAG_CLICKABLE);
    lv_arc_set_rotation(progressArc, values->angle);
    lv_arc_set_bg_angles(progressArc, 0, 360);
    lv_arc_set_start_angle(progressArc, 0);
    lv_arc_set_end_angle(progressArc, values->length);
    lv_obj_set_style_arc_width(progressArc, SPINNER_ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_color(progressArc, lv_color_hex(0x606060), LV_PART_MAIN);
    lv_obj_set_style_arc_color(progressArc, lv_color_hex(0x009EFF), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(progressArc, SPINNER_ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(progressArc, LV_OPA_COVER, LV_PART_INDICATOR);

    values->root = bg;
    values->progressArc = progressArc;

    values->animTimer = lv_timer_create(SpinnerAnimTimerCb, SPINNER_ANIM_INTERVAL_MS, values);
    lv_obj_set_user_data(bg, values);
    lv_obj_add_event_cb(bg, SpinnerDeleteEventCb, LV_EVENT_DELETE, NULL);

    if (timeout > 0) {
        lv_timer_t *timeoutTimer = lv_timer_create(SpinnerTimeoutCb, timeout, bg);
        lv_timer_set_repeat_count(timeoutTimer, 1);
    }

    return bg;
}

void DeleteLoadingSpinner(lv_obj_t *spinner)
{
    if (lv_obj_is_valid(spinner)) {
        LoadingSpinnerValue_t *values = lv_obj_get_user_data(spinner);
        if (values != NULL) {
            if (values->animTimer != NULL) {
                lv_timer_delete(values->animTimer);
                values->animTimer = NULL;
            }
            SRAM_FREE(values);
            lv_obj_set_user_data(spinner, NULL);
        }
        lv_obj_delete(spinner);
    }
}

static void SpinnerAnimTimerCb(lv_timer_t *timer)
{
    LoadingSpinnerValue_t *values = lv_timer_get_user_data(timer);

    values->angle += SPINNER_STEP_DEGREE;
    values->angle = values->angle % 360;

    if (values->lengthIncreasing) {
        values->length += SPINNER_INCREASE_STEP;
        if (values->length >= 290) {
            values->length = 290;
            values->lengthIncreasing = false;
        }
    } else {
        values->length -= SPINNER_INCREASE_STEP;
        if (values->length <= 70) {
            values->length = 70;
            values->lengthIncreasing = true;
        }
    }
    lv_arc_set_rotation(values->progressArc, values->angle);
    lv_arc_set_end_angle(values->progressArc, values->length);
}

static void SpinnerTimeoutCb(lv_timer_t *timer)
{
    lv_obj_t *obj = lv_timer_get_user_data(timer);

    if (lv_obj_is_valid(obj)) {
        lv_obj_delete(obj);
    }
}

static void SpinnerDeleteEventCb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    LoadingSpinnerValue_t *values = lv_obj_get_user_data(obj);

    if (values == NULL) {
        return;
    }

    if (values->animTimer != NULL) {
        lv_timer_delete(values->animTimer);
        values->animTimer = NULL;
    }
    SRAM_FREE(values);
    lv_obj_set_user_data(obj, NULL);
}

