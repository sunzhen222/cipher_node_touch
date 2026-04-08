#include "widgets_line.h"

static lv_point_precise_t g_points[2] = {0};


lv_obj_t *CreateLine(lv_obj_t *parent, uint16_t length)
{
    lv_obj_t *line;
    g_points[1].x = length - 1;

    line = lv_line_create(parent);
    lv_line_set_points(line, g_points, 2);

    return line;
}
