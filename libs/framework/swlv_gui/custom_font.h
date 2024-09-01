/*
 * @Description:
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2023-12-25 14:36:08
 * @LastEditTime: 2023-12-25 14:46:53
 */
#ifndef __CUSTOM_FONT_H
#define __CUSTOM_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

    LV_FONT_DECLARE(font_dignumber_16);
    LV_FONT_DECLARE(font_dignumber_64);
    LV_FONT_DECLARE(font_douyin_12);
    LV_FONT_DECLARE(font_douyin_16);
    LV_FONT_DECLARE(font_douyin_24);

    /**
     * @brief 创建一个Label底部内边距5px
     * @param parent 
     * @param text 
     * @return 
    */
    lv_obj_t* util_label_create_pad_b5(lv_obj_t* parent, const char* text);

#ifdef __cplusplus
}
#endif
#endif
