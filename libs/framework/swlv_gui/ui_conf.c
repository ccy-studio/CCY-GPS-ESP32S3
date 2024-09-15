/*
 * @Description:
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2023-12-07 17:37:28
 * @LastEditTime: 2023-12-25 14:29:00
 */
#include "app_config.h"
#include "app_service.h"
#include "mergin_fsys.h"
#include "ui.h"

ui_data_t* ui_data_def[MAX_PAGE];

// 初始化定义Activity --->> Start
ui_data_t ui_def_start;
extern ui_data_t dark_dial;
extern ui_data_t page_setting;
// 初始化定义Activity <<--- End

void activity_init_start();

void register_ui_def() {
    init_mergin_bin_filesystem();
    activity_init_start();
    // 定义其他的Activity
    ui_data_def[0] = &ui_def_start;
    ui_data_def[1] = &dark_dial;
    ui_data_def[2] = &page_setting;
}

static lv_obj_t* ui_screen_start;

static void my_timer(lv_timer_t* timer) {
    // 跳转到主页面
    ui_intent_t intent;
    ui_fun_fast_create_intent(&ui_def_start, ACTIVITY_ID_DIAL_DARK, &intent);
    intent.anim = false;
    ui_fun_start_activity(&intent);
    ui_fun_finish(&ui_def_start, false);
}

static lv_obj_t* fun_get_view() {
    return ui_screen_start;
}

static void anim_stop(lv_anim_t* a) {
    lv_timer_t* timer = lv_timer_create(my_timer, 500, NULL);
    lv_timer_set_repeat_count(timer, 1);
}

static void anim_cb2(void* var, int32_t v) {
    lv_obj_clear_flag(var, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_x(var, v);
}
static void anim_cb(void* var, int32_t v) {
    lv_obj_set_style_opa(var, v, LV_PART_MAIN);
    if (v == 255) {
        // lv_obj_t* label = ((lv_obj_t*)var)->user_data;
        lv_obj_t* label = lv_obj_get_user_data(var);
        // 创建动画
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, label);
        lv_anim_set_values(&a, 0 - lv_obj_get_width(label),
                           lv_obj_get_x_aligned(label));
        lv_anim_set_time(&a, 800);
        lv_anim_set_exec_cb(&a, anim_cb2);
        lv_anim_set_path_cb(&a, lv_anim_path_bounce);
        lv_anim_set_deleted_cb(&a, anim_stop);
        lv_anim_start(&a);
    }
}

static void _send_open_bl(void* params) {
    notify_bl_event(100);
}

void activity_init_start() {
    ui_screen_start = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_screen_start, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_screen_start, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_screen_start, 255, 0);

    lv_obj_t* product = lv_label_create(ui_screen_start);
    lv_obj_set_width(product, LV_SIZE_CONTENT);
    lv_obj_set_height(product, LV_SIZE_CONTENT);
    lv_obj_center(product);
    lv_label_set_text(product, APP_NAME);
    lv_obj_set_style_text_color(product, lv_color_hex(0xfb8b05), LV_PART_MAIN);
    // lv_obj_set_style_text_font(product, &lv_font_montserrat_24,
    // LV_PART_MAIN);
    lv_obj_set_style_opa(product, 0, LV_PART_MAIN);

    lv_obj_t* label = lv_label_create(ui_screen_start);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 20);
    lv_label_set_text(label, "-SAISAIWA-");
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_obj_set_height(label, LV_SIZE_CONTENT);
    lv_obj_set_width(label, LV_SIZE_CONTENT);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xfb8b05), LV_PART_MAIN);
    // lv_obj_set_style_text_font(label, &font_douyin_12, LV_PART_MAIN);
    // product->user_data = label;
    // label->user_data = NULL;

    lv_obj_set_user_data(product, label);
    lv_obj_set_user_data(label, NULL);

    // 创建动画
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, product);
    lv_anim_set_values(&a, 0, 255);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_delay(&a, 200);
    lv_anim_set_exec_cb(&a, anim_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);

    ui_def_start.fun_get_view = fun_get_view;
    ui_def_start.launcher = true;
    ui_def_start.launcher_mode = SINGLE_TASK;
    ui_def_start.id = ACTIVITY_ID_START;
    lv_async_call(_send_open_bl, NULL);
};
