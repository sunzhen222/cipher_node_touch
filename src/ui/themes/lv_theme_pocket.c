
#include "src/themes/lv_theme_private.h"
#include "lvgl.h"           /*To see all the widgets*/


#if LV_USE_THEME_POCKET

#include "lv_theme_pocket.h"
#include "src/core/lv_global.h"
#include "device_settings.h"

/*********************
 *      DEFINES
 *********************/

struct _my_theme_t;
typedef struct _my_theme_t my_theme_t;

//#define theme_def (*(my_theme_t **)(&LV_GLOBAL_DEFAULT()->theme_pocket))
static void *theme_def;
//#define theme_def (*(my_theme_t **)(&LV_GLOBAL_DEFAULT()->theme_pocket))

#define COLOR_SCR     lv_palette_lighten(LV_PALETTE_GREY, 4)
#define COLOR_WHITE   lv_color_white()
#define COLOR_LIGHT   lv_palette_lighten(LV_PALETTE_GREY, 2)
#define COLOR_DARK    lv_palette_main(LV_PALETTE_GREY)
#define COLOR_DIM     lv_palette_darken(LV_PALETTE_GREY, 2)
#define SCROLLBAR_WIDTH     6

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_style_t scrollbar;
    lv_style_t scr;
    lv_style_t transp;
    lv_style_t white;
    lv_style_t light;
    lv_style_t dark;
    lv_style_t black;
    lv_style_t bg_color_white;
    lv_style_t bg_color_black;
    lv_style_t main_color_100;
    lv_style_t main_color_70;
    lv_style_t main_color_30;
    lv_style_t grey_color;
    lv_style_t dark_grey_color;
    lv_style_t opa_30;
    lv_style_t dim;
    lv_style_t circle;
    lv_style_t knob;
    lv_style_t switch_knob;
    lv_style_t anim_fast;
#if LV_USE_ARC
    lv_style_t arc_line;
    lv_style_t arc_knob;
#endif
#if LV_USE_TEXTAREA
    lv_style_t textarea;
    lv_style_t ta_cursor;
#endif
#if LV_USE_KEYBOARD
    lv_style_t keyboard_pad_tiny;
    lv_style_t keyboard_outline_primary;
    lv_style_t keyboard_outline_secondary;
    lv_style_t keyboard_button_bg;
    lv_style_t keyboard_pressed;
    lv_style_t keyboard_disabled;
    lv_style_t keyboard_checked;
    lv_style_t keyboard_focus_key;
    lv_style_t keyboard_edited;
#endif
    lv_style_t label;
    lv_style_t btn;
#if LV_THEME_DEFAULT_GROW
    lv_style_t grow;
#endif
    lv_style_t slider;
    lv_style_t dropdown;
    lv_style_t dropdown_list;
    lv_style_t line;
} my_theme_styles_t;

struct _my_theme_t {
    lv_theme_t base;
    my_theme_styles_t styles;
    bool inited;
};

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void style_init_reset(lv_style_t * style);
static void theme_apply(lv_theme_t * th, lv_obj_t * obj);

static inline lv_color_t ColorDarkenPct(lv_color_t c, uint8_t percent);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void style_init(my_theme_t * theme)
{
    style_init_reset(&theme->styles.scrollbar);
    lv_style_set_bg_opa(&theme->styles.scrollbar, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.scrollbar, COLOR_DARK);
    lv_style_set_width(&theme->styles.scrollbar, SCROLLBAR_WIDTH);

    style_init_reset(&theme->styles.scr);
    lv_style_set_bg_opa(&theme->styles.scr, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.scr, COLOR_SCR);
    lv_style_set_text_color(&theme->styles.scr, COLOR_DIM);

    style_init_reset(&theme->styles.transp);
    lv_style_set_bg_opa(&theme->styles.transp, LV_OPA_TRANSP);

    style_init_reset(&theme->styles.white);
    lv_style_set_bg_opa(&theme->styles.white, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.white, COLOR_WHITE);
    lv_style_set_line_width(&theme->styles.white, 1);
    lv_style_set_line_color(&theme->styles.white, COLOR_WHITE);
    lv_style_set_arc_width(&theme->styles.white, 2);
    lv_style_set_arc_color(&theme->styles.white, COLOR_WHITE);

    style_init_reset(&theme->styles.light);
    lv_style_set_bg_opa(&theme->styles.light, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.light, COLOR_LIGHT);
    lv_style_set_line_width(&theme->styles.light, 1);
    lv_style_set_line_color(&theme->styles.light, COLOR_LIGHT);
    lv_style_set_arc_width(&theme->styles.light, 2);
    lv_style_set_arc_color(&theme->styles.light, COLOR_LIGHT);

    style_init_reset(&theme->styles.dark);
    lv_style_set_bg_opa(&theme->styles.dark, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.dark, COLOR_DARK);
    lv_style_set_line_width(&theme->styles.dark, 1);
    lv_style_set_line_color(&theme->styles.dark, COLOR_DARK);
    lv_style_set_arc_width(&theme->styles.dark, 2);
    lv_style_set_arc_color(&theme->styles.dark, COLOR_DARK);

    style_init_reset(&theme->styles.black);
    lv_style_set_bg_opa(&theme->styles.black, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.black, lv_color_black());
    lv_style_set_line_width(&theme->styles.black, 1);
    lv_style_set_line_color(&theme->styles.black, lv_color_black());
    lv_style_set_arc_width(&theme->styles.black, 2);
    lv_style_set_arc_color(&theme->styles.black, lv_color_black());

    style_init_reset(&theme->styles.bg_color_white);
    lv_style_set_bg_color(&theme->styles.bg_color_white, lv_color_white());
    lv_style_set_bg_opa(&theme->styles.bg_color_white, LV_OPA_COVER);
    lv_style_set_text_color(&theme->styles.bg_color_white, lv_color_black());

    style_init_reset(&theme->styles.bg_color_black);
    lv_style_set_bg_color(&theme->styles.bg_color_black, lv_color_black());
    lv_style_set_bg_opa(&theme->styles.bg_color_black, LV_OPA_COVER);
    lv_style_set_text_color(&theme->styles.bg_color_black, lv_color_white());

    style_init_reset(&theme->styles.main_color_100);
    lv_style_set_bg_opa(&theme->styles.main_color_100, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.main_color_100, lv_palette_main(DeviceSettingsGetWidgetColor()));

    style_init_reset(&theme->styles.main_color_70);
    lv_style_set_bg_opa(&theme->styles.main_color_70, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.main_color_70, ColorDarkenPct(lv_palette_main(DeviceSettingsGetWidgetColor()), 70));

    style_init_reset(&theme->styles.main_color_30);
    lv_style_set_bg_opa(&theme->styles.main_color_30, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.main_color_30, ColorDarkenPct(lv_palette_main(DeviceSettingsGetWidgetColor()), 30));
    style_init_reset(&theme->styles.grey_color);
    lv_style_set_bg_opa(&theme->styles.grey_color, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.grey_color, lv_palette_main(LV_PALETTE_GREY));

    style_init_reset(&theme->styles.dark_grey_color);
    lv_style_set_bg_opa(&theme->styles.dark_grey_color, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.dark_grey_color, ColorDarkenPct(lv_palette_main(LV_PALETTE_GREY), 30));

    style_init_reset(&theme->styles.opa_30);
    lv_style_set_bg_opa(&theme->styles.opa_30, LV_OPA_30);

    style_init_reset(&theme->styles.dim);
    lv_style_set_bg_opa(&theme->styles.dim, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.dim, COLOR_DIM);
    lv_style_set_line_width(&theme->styles.dim, 1);
    lv_style_set_line_color(&theme->styles.dim, COLOR_DIM);
    lv_style_set_arc_width(&theme->styles.dim, 2);
    lv_style_set_arc_color(&theme->styles.dim, COLOR_DIM);

    style_init_reset(&theme->styles.circle);
    lv_style_set_radius(&theme->styles.circle, LV_RADIUS_CIRCLE);

    style_init_reset(&theme->styles.knob);
    lv_style_set_bg_color(&theme->styles.knob, lv_color_white());
    lv_style_set_bg_opa(&theme->styles.knob, LV_OPA_COVER);
    lv_style_set_pad_all(&theme->styles.knob, 2);
    lv_style_set_radius(&theme->styles.knob, LV_RADIUS_CIRCLE);

    style_init_reset(&theme->styles.switch_knob);
    lv_style_set_bg_color(&theme->styles.switch_knob, lv_color_white());
    lv_style_set_bg_opa(&theme->styles.switch_knob, LV_OPA_COVER);
    lv_style_set_pad_all(&theme->styles.switch_knob, - 2);
    lv_style_set_radius(&theme->styles.switch_knob, LV_RADIUS_CIRCLE);

    style_init_reset(&theme->styles.anim_fast);
    lv_style_set_anim_duration(&theme->styles.anim_fast, 120);

#if LV_USE_ARC
    style_init_reset(&theme->styles.arc_line);
    lv_style_set_arc_width(&theme->styles.arc_line, 6);
    style_init_reset(&theme->styles.arc_knob);
    lv_style_set_pad_all(&theme->styles.arc_knob, 5);
#endif

#if LV_USE_TEXTAREA
    style_init_reset(&theme->styles.textarea);
    lv_style_set_radius(&theme->styles.textarea, 5);
    lv_style_set_text_color(&theme->styles.textarea, lv_color_black());

    style_init_reset(&theme->styles.ta_cursor);
    lv_style_set_border_side(&theme->styles.ta_cursor, LV_BORDER_SIDE_LEFT);
    lv_style_set_border_color(&theme->styles.ta_cursor, COLOR_DIM);
    lv_style_set_border_width(&theme->styles.ta_cursor, 2);
    lv_style_set_bg_opa(&theme->styles.ta_cursor, LV_OPA_TRANSP);
    lv_style_set_anim_duration(&theme->styles.ta_cursor, 500);
#endif

#if LV_USE_KEYBOARD
    style_init_reset(&theme->styles.keyboard_pad_tiny);
    lv_style_set_pad_all(&theme->styles.keyboard_pad_tiny, 2);
    lv_style_set_pad_row(&theme->styles.keyboard_pad_tiny, 2);
    lv_style_set_pad_column(&theme->styles.keyboard_pad_tiny, 2);
    lv_style_set_bg_color(&theme->styles.keyboard_pad_tiny, lv_color_make(220, 220, 220));

    style_init_reset(&theme->styles.keyboard_outline_primary);
    lv_style_set_outline_color(&theme->styles.keyboard_outline_primary, lv_palette_main(DeviceSettingsGetWidgetColor()));
    lv_style_set_outline_width(&theme->styles.keyboard_outline_primary, 2);
    lv_style_set_outline_pad(&theme->styles.keyboard_outline_primary, 2);
    lv_style_set_outline_opa(&theme->styles.keyboard_outline_primary, LV_OPA_50);

    style_init_reset(&theme->styles.keyboard_outline_secondary);
    lv_style_set_outline_color(&theme->styles.keyboard_outline_secondary, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_outline_width(&theme->styles.keyboard_outline_secondary, 2);
    lv_style_set_outline_opa(&theme->styles.keyboard_outline_secondary, LV_OPA_50);

    style_init_reset(&theme->styles.keyboard_button_bg);
    lv_style_set_shadow_width(&theme->styles.keyboard_button_bg, 0);
    lv_style_set_radius(&theme->styles.keyboard_button_bg, 4);
    lv_style_set_bg_color(&theme->styles.keyboard_button_bg, lv_color_white());

    style_init_reset(&theme->styles.keyboard_pressed);
    lv_style_set_recolor(&theme->styles.keyboard_pressed, lv_color_black());
    lv_style_set_recolor_opa(&theme->styles.keyboard_pressed, 35);

    style_init_reset(&theme->styles.keyboard_disabled);
    lv_style_set_recolor(&theme->styles.keyboard_disabled, lv_palette_darken(LV_PALETTE_GREY, 2));
    lv_style_set_recolor_opa(&theme->styles.keyboard_disabled, LV_OPA_50);

    style_init_reset(&theme->styles.keyboard_checked);
    lv_style_set_bg_opa(&theme->styles.keyboard_checked, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.keyboard_checked, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_text_color(&theme->styles.keyboard_checked, lv_color_white());

    style_init_reset(&theme->styles.keyboard_focus_key);
    lv_style_set_bg_opa(&theme->styles.keyboard_focus_key, LV_OPA_20);
    lv_style_set_bg_color(&theme->styles.keyboard_focus_key, lv_palette_main(DeviceSettingsGetWidgetColor()));
    lv_style_set_text_color(&theme->styles.keyboard_focus_key, lv_palette_main(DeviceSettingsGetWidgetColor()));

    style_init_reset(&theme->styles.keyboard_edited);
    lv_style_set_bg_opa(&theme->styles.keyboard_edited, LV_OPA_20);
    lv_style_set_bg_color(&theme->styles.keyboard_edited, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_text_color(&theme->styles.keyboard_edited, lv_palette_main(LV_PALETTE_BLUE));
#endif

    style_init_reset(&theme->styles.label);
    lv_style_set_text_color(&theme->styles.label, lv_color_white());

    style_init_reset(&theme->styles.btn);
    lv_style_set_radius(&theme->styles.btn, 8);
    lv_style_set_bg_opa(&theme->styles.btn, LV_OPA_COVER);
    lv_style_set_text_color(&theme->styles.btn, lv_color_white());
    lv_style_set_layout(&theme->styles.btn, LV_LAYOUT_FLEX);
    lv_style_set_flex_cross_place(&theme->styles.btn, LV_FLEX_ALIGN_CENTER);
    lv_style_set_flex_main_place(&theme->styles.btn, LV_FLEX_ALIGN_CENTER);
    lv_style_set_flex_track_place(&theme->styles.btn, LV_FLEX_ALIGN_CENTER);
    lv_style_set_pad_hor(&theme->styles.btn, 4);
    lv_style_set_pad_ver(&theme->styles.btn, 4);
    lv_style_set_pad_column(&theme->styles.btn, 4);
    lv_style_set_pad_row(&theme->styles.btn, 4);

#if LV_THEME_DEFAULT_GROW
    style_init_reset(&theme->styles.grow);
    lv_style_set_transform_width(&theme->styles.grow, 2);
    lv_style_set_transform_height(&theme->styles.grow, 2);
#endif

    style_init_reset(&theme->styles.slider);

    style_init_reset(&theme->styles.dropdown);
    lv_style_set_radius(&theme->styles.dropdown, 8);
    lv_style_set_bg_opa(&theme->styles.dropdown, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.dropdown, lv_color_white());
    lv_style_set_border_color(&theme->styles.dropdown, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_border_width(&theme->styles.dropdown, 2);
    lv_style_set_border_post(&theme->styles.dropdown, true);
    lv_style_set_text_color(&theme->styles.dropdown, lv_color_black());
    lv_style_set_pad_all(&theme->styles.dropdown, 8);
    lv_style_set_pad_row(&theme->styles.dropdown, 4);
    lv_style_set_pad_column(&theme->styles.dropdown, 4);
    lv_style_set_line_color(&theme->styles.dropdown, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_line_width(&theme->styles.dropdown, 1);

    style_init_reset(&theme->styles.dropdown_list);
    lv_style_set_radius(&theme->styles.dropdown_list, 8);
    lv_style_set_bg_opa(&theme->styles.dropdown_list, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.dropdown_list, lv_color_white());
    lv_style_set_border_color(&theme->styles.dropdown_list, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_border_width(&theme->styles.dropdown_list, 2);
    lv_style_set_border_post(&theme->styles.dropdown_list, true);
    lv_style_set_text_color(&theme->styles.dropdown_list, lv_color_black());
    lv_style_set_pad_all(&theme->styles.dropdown_list, 8);
    lv_style_set_pad_row(&theme->styles.dropdown_list, 8);
    lv_style_set_pad_column(&theme->styles.dropdown_list, 8);
    lv_style_set_line_color(&theme->styles.dropdown_list, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_line_width(&theme->styles.dropdown_list, 1);
    lv_style_set_clip_corner(&theme->styles.dropdown_list, true);
    lv_style_set_border_post(&theme->styles.dropdown_list, true);

    style_init_reset(&theme->styles.line);
    lv_style_set_line_width(&theme->styles.line, 2);
    lv_style_set_line_color(&theme->styles.line, lv_color_hex(0x8F8F8F));
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_theme_t * lv_theme_pocket_init(lv_display_t * disp)
{
    /*This trick is required only to avoid the garbage collection of
     *styles' data if LVGL is used in a binding (e.g. MicroPython)
     *In a general case styles could be in a pocket `static lv_style_t my_style...` variables*/
    if (!lv_theme_pocket_is_inited()) {
        theme_def = lv_malloc_zeroed(sizeof(my_theme_t));
        LV_ASSERT_MALLOC(theme_def);
    }

    my_theme_t * theme = theme_def;

    theme->base.disp = disp;
    theme->base.font_small = LV_FONT_DEFAULT;
    theme->base.font_normal = LV_FONT_DEFAULT;
    theme->base.font_large = LV_FONT_DEFAULT;
    theme->base.apply_cb = theme_apply;

    style_init(theme);

    if (disp == NULL || lv_display_get_theme(disp) == (lv_theme_t *)theme) {
        lv_obj_report_style_change(NULL);
    }

    theme->inited = true;

    return (lv_theme_t *)theme_def;
}

bool lv_theme_pocket_is_inited(void)
{
    my_theme_t * theme = theme_def;
    if (theme == NULL) return false;
    return theme->inited;
}

lv_theme_t * lv_theme_pocket_get(void)
{
    if (!lv_theme_pocket_is_inited()) {
        return NULL;
    }

    return (lv_theme_t *)theme_def;
}

void lv_theme_pocket_deinit(void)
{
    my_theme_t * theme = theme_def;
    if (theme) {
        if (theme->inited) {
            lv_style_t * theme_styles = (lv_style_t *)(&(theme->styles));
            uint32_t i;
            for (i = 0; i < sizeof(my_theme_styles_t) / sizeof(lv_style_t); i++) {
                lv_style_reset(theme_styles + i);
            }
        }
        lv_free(theme_def);
        theme_def = NULL;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void theme_apply(lv_theme_t * th, lv_obj_t * obj)
{
    LV_UNUSED(th);

    my_theme_t * theme = theme_def;
    lv_obj_t * parent = lv_obj_get_parent(obj);

    if (parent == NULL) {
        lv_obj_add_style(obj, &theme->styles.scr, 0);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
        return;
    }

    if (lv_obj_check_type(obj, &lv_obj_class)) {
#if LV_USE_TABVIEW
        /*Tabview content area*/
        if (lv_obj_check_type(parent, &lv_tabview_class)) {
            lv_obj_add_style(obj, &theme->styles.scr, 0);
            return;
        }
        /*Tabview pages*/
        else if (lv_obj_check_type(lv_obj_get_parent(parent), &lv_tabview_class)) {
            lv_obj_add_style(obj, &theme->styles.scr, 0);
            lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
            return;
        }
#endif

#if LV_USE_WIN
        /*Header*/
        if (lv_obj_check_type(parent, &lv_win_class) && lv_obj_get_child(parent, 0) == obj) {
            lv_obj_add_style(obj, &theme->styles.light, 0);
            return;
        }
        /*Content*/
        else if (lv_obj_check_type(parent, &lv_win_class) && lv_obj_get_child(parent, 1) == obj) {
            lv_obj_add_style(obj, &theme->styles.light, 0);
            lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
            return;
        }
#endif
        lv_obj_add_style(obj, &theme->styles.black, 0);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
    } else if (lv_obj_check_type(obj, &lv_button_class)) {
        lv_obj_add_style(obj, &theme->styles.btn, 0);
        lv_obj_add_style(obj, &theme->styles.main_color_100, 0);
        lv_obj_add_style(obj, &theme->styles.main_color_70, LV_STATE_PRESSED);
        lv_obj_add_style(obj, &theme->styles.dark_grey_color, LV_STATE_DISABLED);
#if LV_THEME_DEFAULT_GROW
        lv_obj_add_style(obj, &theme->styles.grow, LV_STATE_PRESSED);
#endif
    }

#if LV_USE_BUTTONMATRIX
    else if (lv_obj_check_type(obj, &lv_buttonmatrix_class)) {
#if LV_USE_MSGBOX
        if (lv_obj_check_type(parent, &lv_msgbox_class)) {
            lv_obj_add_style(obj, &theme->styles.light, LV_PART_ITEMS);
            return;
        }
#endif
#if LV_USE_TABVIEW
        if (lv_obj_check_type(parent, &lv_tabview_class)) {
            lv_obj_add_style(obj, &theme->styles.light, LV_PART_ITEMS);
            return;
        }
#endif
        lv_obj_add_style(obj, &theme->styles.white, 0);
        lv_obj_add_style(obj, &theme->styles.light, LV_PART_ITEMS);
    }
#endif

#if LV_USE_BAR
    else if (lv_obj_check_type(obj, &lv_bar_class)) {
        lv_obj_add_style(obj, &theme->styles.dark_grey_color, 0);
        lv_obj_add_style(obj, &theme->styles.circle, 0);
        lv_obj_add_style(obj, &theme->styles.main_color_100, LV_PART_INDICATOR);
    }
#endif

#if LV_USE_TABLE
    else if (lv_obj_check_type(obj, &lv_table_class)) {
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
        lv_obj_add_style(obj, &theme->styles.light, LV_PART_ITEMS);
    }
#endif

#if LV_USE_CHECKBOX
    else if (lv_obj_check_type(obj, &lv_checkbox_class)) {
        lv_obj_add_style(obj, &theme->styles.light, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &theme->styles.dark, LV_PART_INDICATOR | LV_STATE_CHECKED);
    }
#endif

#if LV_USE_SWITCH
    else if (lv_obj_check_type(obj, &lv_switch_class)) {
        lv_obj_add_style(obj, &theme->styles.grey_color, 0);
        lv_obj_add_style(obj, &theme->styles.circle, 0);
        lv_obj_add_style(obj, &theme->styles.anim_fast, 0);
        lv_obj_add_style(obj, &theme->styles.main_color_100, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &theme->styles.circle, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &theme->styles.switch_knob, LV_PART_KNOB);
    }
#endif

#if LV_USE_CHART
    else if (lv_obj_check_type(obj, &lv_chart_class)) {
        lv_obj_add_style(obj, &theme->styles.white, 0);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
        lv_obj_add_style(obj, &theme->styles.light, LV_PART_ITEMS);
        lv_obj_add_style(obj, &theme->styles.dark, LV_PART_CURSOR);
    }
#endif

#if LV_USE_ROLLER
    else if (lv_obj_check_type(obj, &lv_roller_class)) {
        lv_obj_add_style(obj, &theme->styles.light, 0);
        lv_obj_add_style(obj, &theme->styles.dark, LV_PART_SELECTED);
    }
#endif

#if LV_USE_ARC
    else if (lv_obj_check_type(obj, &lv_arc_class)) {
        lv_obj_add_style(obj, &theme->styles.light, 0);
        lv_obj_add_style(obj, &theme->styles.transp, 0);
        lv_obj_add_style(obj, &theme->styles.arc_line, 0);
        lv_obj_add_style(obj, &theme->styles.dark, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &theme->styles.arc_line, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &theme->styles.dim, LV_PART_KNOB);
        lv_obj_add_style(obj, &theme->styles.arc_knob, LV_PART_KNOB);
    }
#endif

#if LV_USE_SPINNER
    else if (lv_obj_check_type(obj, &lv_spinner_class)) {
        lv_obj_add_style(obj, &theme->styles.light, 0);
        lv_obj_add_style(obj, &theme->styles.transp, 0);
        lv_obj_add_style(obj, &theme->styles.arc_line, 0);
        lv_obj_add_style(obj, &theme->styles.dark, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &theme->styles.arc_line, LV_PART_INDICATOR);
    }
#endif

#if LV_USE_TEXTAREA
    else if (lv_obj_check_type(obj, &lv_textarea_class)) {
        lv_obj_add_style(obj, &theme->styles.white, 0);
        lv_obj_add_style(obj, &theme->styles.textarea, 0);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
        lv_obj_add_style(obj, &theme->styles.ta_cursor, LV_PART_CURSOR | LV_STATE_FOCUSED);
    }
#endif

#if LV_USE_CALENDAR
    else if (lv_obj_check_type(obj, &lv_calendar_class)) {
        lv_obj_add_style(obj, &theme->styles.light, 0);
    }
#endif

#if LV_USE_KEYBOARD
    else if (lv_obj_check_type(obj, &lv_keyboard_class)) {
        lv_obj_add_style(obj, &theme->styles.scr, 0);
        lv_obj_add_style(obj, &theme->styles.keyboard_pad_tiny, 0);
        lv_obj_add_style(obj, &theme->styles.keyboard_outline_primary, LV_STATE_FOCUS_KEY);
        lv_obj_add_style(obj, &theme->styles.keyboard_outline_secondary, LV_STATE_EDITED);
        lv_obj_add_style(obj, &theme->styles.btn, LV_PART_ITEMS);
        lv_obj_add_style(obj, &theme->styles.keyboard_disabled, LV_PART_ITEMS | LV_STATE_DISABLED);
        lv_obj_add_style(obj, &theme->styles.bg_color_white, LV_PART_ITEMS);
        lv_obj_add_style(obj, &theme->styles.keyboard_button_bg, LV_PART_ITEMS);
        lv_obj_add_style(obj, &theme->styles.keyboard_pressed, LV_PART_ITEMS | LV_STATE_PRESSED);
        lv_obj_add_style(obj, &theme->styles.keyboard_checked, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &theme->styles.keyboard_focus_key, LV_PART_ITEMS | LV_STATE_FOCUS_KEY);
        lv_obj_add_style(obj, &theme->styles.keyboard_edited, LV_PART_ITEMS | LV_STATE_EDITED);
    }
#endif

#if LV_USE_LIST
    else if (lv_obj_check_type(obj, &lv_list_class)) {
        lv_obj_add_style(obj, &theme->styles.light, 0);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
        return;
    } else if (lv_obj_check_type(obj, &lv_list_text_class)) {

    } else if (lv_obj_check_type(obj, &lv_list_button_class)) {
        lv_obj_add_style(obj, &theme->styles.dark, 0);
    }
#endif
#if LV_USE_MSGBOX
    else if (lv_obj_check_type(obj, &lv_msgbox_class)) {
        lv_obj_add_style(obj, &theme->styles.light, 0);
        return;
    }
#endif

#if LV_USE_SPINBOX
    else if (lv_obj_check_type(obj, &lv_spinbox_class)) {
        lv_obj_add_style(obj, &theme->styles.light, 0);
        lv_obj_add_style(obj, &theme->styles.dark, LV_PART_CURSOR);
    }
#endif
#if LV_USE_TILEVIEW
    else if (lv_obj_check_type(obj, &lv_tileview_class)) {
        lv_obj_add_style(obj, &theme->styles.bg_color_black, 0);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
    } else if (lv_obj_check_type(obj, &lv_tileview_tile_class)) {
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
    }
#endif

#if LV_USE_LED
    else if (lv_obj_check_type(obj, &lv_led_class)) {
        lv_obj_add_style(obj, &theme->styles.light, 0);
        lv_obj_add_style(obj, &theme->styles.circle, 0);
    }
#endif

    else if (lv_obj_check_type(obj, &lv_label_class)) {
        if (!lv_obj_check_type(parent, &lv_button_class) && !lv_obj_check_type(parent, &lv_dropdownlist_class)) {
            lv_obj_add_style(obj, &theme->styles.label, 0);
        }
    }

    else if (lv_obj_check_type(obj, &lv_slider_class)) {
        lv_obj_add_style(obj, &theme->styles.slider, 0);
        lv_obj_add_style(obj, &theme->styles.dark_grey_color, 0);
        lv_obj_add_style(obj, &theme->styles.circle, 0);
        lv_obj_add_style(obj, &theme->styles.knob, LV_PART_KNOB);
        lv_obj_add_style(obj, &theme->styles.slider, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &theme->styles.circle, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &theme->styles.main_color_100, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &theme->styles.dark_grey_color, LV_STATE_DISABLED);
        lv_obj_add_style(obj, &theme->styles.grey_color, LV_STATE_DISABLED | LV_PART_KNOB);
        lv_obj_add_style(obj, &theme->styles.main_color_30, LV_STATE_DISABLED | LV_PART_INDICATOR);
    }

#if LV_USE_DROPDOWN
    else if (lv_obj_check_type(obj, &lv_dropdown_class)) {
        lv_obj_add_style(obj, &theme->styles.dropdown, 0);
    } else if (lv_obj_check_type(obj, &lv_dropdownlist_class)) {
        lv_obj_add_style(obj, &theme->styles.dropdown_list, 0);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
        lv_obj_add_style(obj, &theme->styles.bg_color_white, LV_PART_SELECTED);
        lv_obj_add_style(obj, &theme->styles.main_color_100, LV_PART_SELECTED | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &theme->styles.grey_color, LV_PART_SELECTED | LV_STATE_PRESSED);
    }
#endif

    else if (lv_obj_check_type(obj, &lv_line_class)) {
        lv_obj_add_style(obj, &theme->styles.line, 0);
    }
}

static void style_init_reset(lv_style_t * style)
{
    if (lv_theme_pocket_is_inited()) {
        lv_style_reset(style);
    } else {
        lv_style_init(style);
    }
}

static inline lv_color_t ColorDarkenPct(lv_color_t c, uint8_t percent)
{
    // percent: 0~100
    lv_color_t res;

    res.red   = (c.red   * percent) / 100;
    res.green = (c.green * percent) / 100;
    res.blue  = (c.blue  * percent) / 100;

    return res;
}

#endif
