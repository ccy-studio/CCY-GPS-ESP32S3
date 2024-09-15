#include <math.h>
#include "app_service.h"
#include "custom_font.h"
#include "ui.h"

static ui_data_t* _this;

static lv_obj_t* page_view = NULL;
static lv_obj_t* icon_gps = NULL;            // GPS图标
static lv_obj_t* icon_control_state = NULL;  // 当前运行图标
static lv_obj_t* text_current_time = NULL;   // 顶部当前时间
static lv_obj_t* text_battery = NULL;  // 顶部当前电量百分比文字
static lv_obj_t* bar_battery = NULL;  // 顶部当前电量百分比Bar动态组件
static lv_obj_t* icon_battery_charge = NULL;  // 顶部当前电量显示正在充电
static lv_obj_t* text_current_speed = NULL;  // 显示当前速度值文字
static lv_obj_t* text_run_timeinfo = NULL;  // 显示当前运行的时间信息文字
static lv_obj_t* text_speed_max = NULL;  // 显示当前运行的最大行驶速度
static lv_obj_t* text_speed_avg = NULL;  // 显示当前运行的平均行驶速度
static lv_obj_t* text_speed_sum = NULL;        // 显示当前运行的总里程
static lv_obj_t* text_sensor_temp = NULL;      // 显示当前传感器的温度
static lv_obj_t* text_sensor_humidity = NULL;  // 显示当前传感器的湿度

/*EventBus*/
static void* bus_gps_change;   /*GPS数据*/
static void* bus_data_change;  /*行驶记录数据*/
static void* bus_battery;      /*电池电量数据*/
static void* bus_env_change;   /*环境信息数据*/
static void* bus_record_start; /*开始骑行事件*/
static void* bus_record_stop;  /*停止骑行事件*/
static void* bus_button;       /*按键事件*/

static lv_obj_t* fun_get_view() {
    return page_view;
}

/**
 * @brief 未充电时创建动态电量百分比Bar组件
 */
static void create_battery_bar() {
    bar_battery = lv_bar_create(page_view);
    lv_bar_set_value(bar_battery, 100, LV_ANIM_OFF);
    lv_bar_set_range(bar_battery, 0, 100);
    lv_bar_set_start_value(bar_battery, 0, LV_ANIM_OFF);
    lv_obj_set_width(bar_battery, 20);
    lv_obj_set_height(bar_battery, 10);
    lv_obj_set_x(bar_battery, 208);
    lv_obj_set_y(bar_battery, 10);
    lv_obj_set_style_radius(bar_battery, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(bar_battery, lv_color_hex(0xFFFFFF),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bar_battery, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(bar_battery, 0,
                            LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(bar_battery, lv_color_hex(0x30BA58),
                              LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bar_battery, 255,
                            LV_PART_INDICATOR | LV_STATE_DEFAULT);
}

static void on_event_key_cb(lv_event_t* e) {
    uint32_t key = lv_event_get_key(e);
    //默认给了聚焦所以只接受LV_KEY_ENTER等控制性按键类型，这里默认只有LV_KEY_ENTER
    ui_intent_t intent;
    ui_fun_fast_create_intent(_this, ACTIVITY_ID_SETTING, &intent);
    ui_fun_start_activity(&intent);
}

static void on_create_fun(ui_data_t* ui_dat, void* params) {
    _this = ui_dat;
    page_view = lv_obj_create(NULL);
    lv_group_add_obj(ui_dat->group, page_view);
    lv_obj_clear_flag(page_view, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_img_src(page_view, "S:/img/dark_style_bg.bin",
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    // 添加按键的事件
    lv_obj_add_event_cb(page_view, on_event_key_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_remove_state(page_view, LV_STATE_DISABLED);
    lv_group_focus_obj(page_view);

    icon_gps = lv_img_create(page_view);
    lv_img_set_src(icon_gps, "S:/img/gps.bin");
    lv_obj_set_width(icon_gps, LV_SIZE_CONTENT);
    lv_obj_set_height(icon_gps, LV_SIZE_CONTENT);
    lv_obj_set_x(icon_gps, 42);
    lv_obj_set_y(icon_gps, 7);
    lv_obj_add_flag(icon_gps, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(icon_gps, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(icon_gps, LV_OBJ_FLAG_HIDDEN);

    text_current_time = lv_label_create(page_view);
    lv_obj_set_width(text_current_time, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(text_current_time, 0, LV_PART_MAIN);
    lv_obj_set_height(text_current_time, LV_SIZE_CONTENT);
    lv_obj_set_x(text_current_time, 104);
    lv_obj_set_y(text_current_time, 5);
    lv_label_set_text(text_current_time, "00:00");
    lv_obj_set_style_text_color(text_current_time, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(text_current_time, 255,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(text_current_time, &font_douyin_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    text_battery = lv_label_create(page_view);
    lv_obj_set_width(text_battery, LV_SIZE_CONTENT);
    lv_obj_set_height(text_battery, LV_SIZE_CONTENT);
    lv_obj_set_x(text_battery, 167);
    lv_obj_set_y(text_battery, 9);
    lv_label_set_text(text_battery, "0%");
    lv_obj_set_style_text_color(text_battery, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(text_battery, 255,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(text_battery, &font_douyin_12,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    text_current_speed = lv_label_create(page_view);
    lv_obj_set_width(text_current_speed, 130);
    lv_obj_set_height(text_current_speed, LV_SIZE_CONTENT);
    lv_label_set_long_mode(text_current_speed, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_text_align(text_current_speed, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(text_current_speed, 10);
    lv_obj_set_y(text_current_speed, 40);
    lv_label_set_text(text_current_speed, "00");
    lv_obj_set_style_text_color(text_current_speed, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(text_current_speed, 255,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(text_current_speed, &font_dignumber_64,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* text_kms = lv_label_create(page_view);
    lv_obj_set_width(text_kms, LV_SIZE_CONTENT);
    lv_obj_set_height(text_kms, LV_SIZE_CONTENT);
    lv_obj_set_x(text_kms, 143);
    lv_obj_set_y(text_kms, 104);
    lv_label_set_text(text_kms, "KM/h");
    lv_obj_set_style_text_color(text_kms, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(text_kms, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(text_kms, &font_douyin_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    text_run_timeinfo = lv_label_create(page_view);
    lv_obj_set_width(text_run_timeinfo, LV_SIZE_CONTENT);
    lv_obj_set_height(text_run_timeinfo, LV_SIZE_CONTENT);
    lv_obj_set_x(text_run_timeinfo, 13);
    lv_obj_set_y(text_run_timeinfo, 137);
    lv_label_set_text(text_run_timeinfo, "");
    lv_obj_set_style_text_color(text_run_timeinfo, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(text_run_timeinfo, 255,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(text_run_timeinfo, &font_douyin_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    text_speed_max = lv_label_create(page_view);
    lv_obj_set_width(text_speed_max, LV_SIZE_CONTENT);
    lv_obj_set_height(text_speed_max, LV_SIZE_CONTENT);
    lv_obj_set_x(text_speed_max, 14);
    lv_obj_set_y(text_speed_max, 189);
    lv_label_set_text(text_speed_max, "最大(Max): 0km/s");
    lv_obj_set_style_text_color(text_speed_max, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(text_speed_max, 255,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(text_speed_max, &font_douyin_12,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    text_speed_avg = lv_label_create(page_view);
    lv_obj_set_width(text_speed_avg, LV_SIZE_CONTENT);
    lv_obj_set_height(text_speed_avg, LV_SIZE_CONTENT);
    lv_obj_set_x(text_speed_avg, 14);
    lv_obj_set_y(text_speed_avg, 212);
    lv_label_set_text(text_speed_avg, "平均(Avg): 0km/s");
    lv_obj_set_style_text_color(text_speed_avg, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(text_speed_avg, 255,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(text_speed_avg, &font_douyin_12,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    text_speed_sum = lv_label_create(page_view);
    lv_obj_set_width(text_speed_sum, LV_SIZE_CONTENT);
    lv_obj_set_height(text_speed_sum, LV_SIZE_CONTENT);
    lv_obj_set_x(text_speed_sum, 14);
    lv_obj_set_y(text_speed_sum, 235);
    lv_label_set_text(text_speed_sum, "里程(Sum): 0km");
    lv_obj_set_style_text_color(text_speed_sum, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(text_speed_sum, 255,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(text_speed_sum, &font_douyin_12,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    text_sensor_temp = lv_label_create(page_view);
    lv_obj_set_width(text_sensor_temp, LV_SIZE_CONTENT);
    lv_obj_set_height(text_sensor_temp, LV_SIZE_CONTENT);
    lv_obj_set_x(text_sensor_temp, 188);
    lv_obj_set_y(text_sensor_temp, 212);
    lv_label_set_text(text_sensor_temp, "-");
    lv_obj_set_style_text_color(text_sensor_temp, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(text_sensor_temp, 255,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(text_sensor_temp, &font_douyin_12,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    text_sensor_humidity = lv_label_create(page_view);
    lv_obj_set_width(text_sensor_humidity, LV_SIZE_CONTENT);
    lv_obj_set_height(text_sensor_humidity, LV_SIZE_CONTENT);
    lv_obj_set_x(text_sensor_humidity, 190);
    lv_obj_set_y(text_sensor_humidity, 240);
    lv_label_set_text(text_sensor_humidity, "-");
    lv_obj_set_style_text_color(text_sensor_humidity, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(text_sensor_humidity, 255,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(text_sensor_humidity, &font_douyin_12,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    icon_control_state = lv_img_create(page_view);
    lv_img_set_src(icon_control_state, "S:/img/record_start.bin");
    lv_obj_set_width(icon_control_state, LV_SIZE_CONTENT);
    lv_obj_set_height(icon_control_state, LV_SIZE_CONTENT);
    lv_obj_set_x(icon_control_state, 202);
    lv_obj_set_y(icon_control_state, 39);
    lv_obj_add_flag(icon_control_state, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(icon_control_state, LV_OBJ_FLAG_SCROLLABLE);
}

static void on_event_data_change(bus_msg_t msg) {
    app_real_record_t* dat = (app_real_record_t*)msg.payload;

    // 当开启了骑行
    if (dat->is_start) {
        // 设置骑行时间
        lv_label_set_text(text_run_timeinfo,
                          format_time(dat->curr_log_dat.run_second));
        uint16_t max_speed = roundf(dat->curr_log_dat.max_speed * 3.6);
        uint16_t avg_speed = roundf((dat->sum_speed / dat->tick_cnt) * 3.6);
        lv_label_set_text_fmt(text_speed_max, "最大(Max): %dkm/h", max_speed);
        lv_label_set_text_fmt(text_speed_avg, "平均(Avg): %dkm/h", avg_speed);

        int distance_km = (int)(dat->curr_log_dat.mileage / 1000);
        lv_label_set_text_fmt(text_speed_sum, "里程(Sum): %dkm", distance_km);
    }
}
static void on_event_battery(bus_msg_t msg) {
    app_battery_t* battery = msg.payload;
    // 电量
    lv_label_set_text_fmt(text_battery, "%.1f%%", battery->level);
    if (battery->is_charge) {
        // 正在充电
        lv_obj_set_style_text_color(text_battery, lv_color_hex(0x59EE71), 0);
        if (bar_battery != NULL) {
            lv_obj_del(bar_battery);
            bar_battery = NULL;
        }
        // 切换充电图标
        if (icon_battery_charge == NULL) {
            icon_battery_charge = lv_img_create(page_view);
            lv_img_set_src(icon_battery_charge, "S:/img/battery.bin");
            lv_obj_set_width(icon_battery_charge, LV_SIZE_CONTENT);
            lv_obj_set_height(icon_battery_charge, LV_SIZE_CONTENT);
            lv_obj_align_to(icon_battery_charge, text_battery,
                            LV_ALIGN_OUT_RIGHT_MID, 10, 0);
            lv_obj_clear_flag(icon_battery_charge, LV_OBJ_FLAG_SCROLLABLE);
        }
    } else {
        // 未充电
        if (icon_battery_charge != NULL) {
            lv_obj_del(icon_battery_charge);
            icon_battery_charge = NULL;
        }
        if (bar_battery == NULL) {
            create_battery_bar();
        }
        // 改变进度条的值
        lv_bar_set_value(bar_battery, battery->level, LV_ANIM_OFF);
        if (battery->level < 20.0) {
            lv_obj_set_style_text_color(text_battery, lv_color_hex(0xde1c31),
                                        0);
            lv_obj_set_style_bg_color(bar_battery, lv_color_hex(0xde1c31),
                                      LV_PART_INDICATOR | LV_STATE_DEFAULT);
        } else {
            lv_obj_set_style_text_color(text_battery, lv_color_white(), 0);
            lv_obj_set_style_bg_color(bar_battery, lv_color_hex(0x3170a7),
                                      LV_PART_INDICATOR | LV_STATE_DEFAULT);
        }
    }
}
static void on_event_env_change(bus_msg_t msg) {
    app_environment_t* env = msg.payload;
    lv_label_set_text_fmt(text_sensor_temp, "%.1f度", env->temp);
    lv_label_set_text_fmt(text_sensor_humidity, "%.1f%%", env->humidity);
}
static void on_event_record_start(bus_msg_t msg) {
    lv_label_set_text(text_run_timeinfo, "0秒");
    lv_img_set_src(icon_control_state, "S:/img/record_stop.bin");
}
static void on_event_record_stop(bus_msg_t msg) {
    lv_img_set_src(icon_control_state, "S:/img/record_start.bin");

    lv_label_set_text(text_speed_max, "最大(Max): 0km/s");
    lv_label_set_text(text_speed_avg, "平均(Avg): 0km/s");
    lv_label_set_text(text_speed_sum, "里程(Sum): 0km");
    lv_label_set_text(text_run_timeinfo, "");
}

/**
 * GPS的刷新事件
 */
static void on_event_gps_change(bus_msg_t msg) {
    app_gps_t* dat = (app_gps_t*)msg.payload;
    // 时间
    lv_label_set_text_fmt(text_current_time, "%02d:%02d", dat->datetime.hour,
                          dat->datetime.minute);

    // GPS图标
    if (dat->sats_in_use == 0) {
        lv_obj_add_flag(icon_gps, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(icon_gps, LV_OBJ_FLAG_HIDDEN);
    }
    // GPS信息 从m/s换算成km/h
    if (dat->speed == 0) {
        return;
    }
    int speed_kmh = roundf((dat->speed * 3.6));
    lv_label_set_text_fmt(text_current_speed, "%02d", speed_kmh);
}

static void on_destoy_fun(void* params) {}

static void on_stop_fun(void* params) {
    bus_unregister_subscribe(bus_data_change);
    bus_unregister_subscribe(bus_battery);
    bus_unregister_subscribe(bus_env_change);
    bus_unregister_subscribe(bus_record_start);
    bus_unregister_subscribe(bus_record_stop);
    bus_unregister_subscribe(bus_gps_change);
}

static void on_start_fun(void* params) {
    bus_data_change = bus_register_subscribe(DATA_BUS_RECORD_CHANGE,
                                             on_event_data_change, NULL);
    bus_gps_change =
        bus_register_subscribe(DATA_BUS_GPS_REFRESH, on_event_gps_change, NULL);
    bus_battery =
        bus_register_subscribe(DATA_BUS_BATTERY_EVENT, on_event_battery, NULL);
    bus_env_change =
        bus_register_subscribe(DATA_BUS_ENV_REFRESH, on_event_env_change, NULL);
    bus_record_start = bus_register_subscribe(DATA_BUS_RECORD_START,
                                              on_event_record_start, NULL);
    bus_record_stop = bus_register_subscribe(DATA_BUS_RECORD_STOP,
                                             on_event_record_stop, NULL);
}

static void on_index_read_cb(lv_indev_t* indev, lv_indev_data_t* data) {
    app_btn_pck* pck = lv_indev_get_driver_data(indev);
    if (pck->btn_code == APP_BUTTON_UP) {
        data->key = LV_KEY_UP;
    } else if (pck->btn_code == APP_BUTTON_DOWN) {
        data->key = LV_KEY_DOWN;
    } else if (pck->btn_code == APP_BUTTON_ENTER) {
        data->key = LV_KEY_ENTER;
        if (pck->btn_state == APP_BUTTON_LONG_PRESS) {
            if (global_real_record.is_start) {
                app_stop_record();
            } else {
                app_start_record();
            }
            data->key = LV_KEY_ESC;
        }
    }
}

ui_data_t dark_dial = {.id = ACTIVITY_ID_DIAL_DARK,
                       .launcher_mode = SINGLE_TASK,  // 栈内复用模式
                       .fun_get_view = fun_get_view,
                       .fun_on_create = on_create_fun,
                       .fun_on_destoy = on_destoy_fun,
                       .fun_on_stop = on_stop_fun,
                       .fun_read_cb = on_index_read_cb,
                       .fun_on_start = on_start_fun};
