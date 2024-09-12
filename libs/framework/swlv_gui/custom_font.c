/*
 * @Description:
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2023-12-25 14:40:14
 * @LastEditTime: 2023-12-25 14:56:47
 */
#include "custom_font.h"

lv_obj_t* util_label_create_pad_b5(lv_obj_t* parent, const char* text) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(label, 5, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &font_douyin_16, LV_PART_MAIN);
    return label;
}
