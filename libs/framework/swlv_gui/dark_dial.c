#include <math.h>
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
static void* bus_data_change;  /*仪表数据*/
static void* bus_battery;      /*电池电量数据*/
static void* bus_env_change;   /*环境信息数据*/
static void* bus_record_start; /*开始骑行事件*/
static void* bus_record_stop;  /*停止骑行事件*/
static void* bus_button;       /*按键事件*/

static void on_event_data_change(void* sub, bus_msg_t* msg);
static void on_event_battery(void* sub, bus_msg_t* msg);
static void on_event_env_change(void* sub, bus_msg_t* msg);
static void on_event_record_start(void* sub, bus_msg_t* msg);
static void on_event_record_stop(void* sub, bus_msg_t* msg);
static void on_event_button(void* sub, bus_msg_t* msg);

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

static void on_create_fun(ui_data_t* ui_dat, void* params) {
    _this = ui_dat;
    page_view = lv_obj_create(NULL);
    lv_obj_clear_flag(page_view, LV_OBJ_FLAG_SCROLLABLE);  
    lv_obj_set_style_bg_img_src(page_view, "S:/img/dark_style_bg.bin",
                                LV_PART_MAIN | LV_STATE_DEFAULT);

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
    lv_obj_set_x(text_battery, 180);
    lv_obj_set_y(text_battery, 9);
    lv_label_set_text(text_battery, "100%");
    lv_obj_set_style_text_color(text_battery, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(text_battery, 255,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(text_battery, &font_douyin_12, LV_PART_MAIN |
    LV_STATE_DEFAULT);

    text_current_speed = lv_label_create(page_view);
    lv_obj_set_width(text_current_speed, 128);
    lv_obj_set_height(text_current_speed, LV_SIZE_CONTENT);
    lv_label_set_long_mode(text_current_speed, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_text_align(text_current_speed, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(text_current_speed, 10);
    lv_obj_set_y(text_current_speed, 40);
    lv_label_set_text(text_current_speed, "50");
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
    lv_obj_set_style_text_font(text_kms, &font_douyin_16, LV_PART_MAIN |
    LV_STATE_DEFAULT);

    text_run_timeinfo = lv_label_create(page_view);
    lv_obj_set_width(text_run_timeinfo, LV_SIZE_CONTENT);   
    lv_obj_set_height(text_run_timeinfo, LV_SIZE_CONTENT);  
    lv_obj_set_x(text_run_timeinfo, 13);
    lv_obj_set_y(text_run_timeinfo, 137);
    lv_label_set_text(text_run_timeinfo, "时长：12小时05分06秒");
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
    lv_obj_set_style_text_font(text_speed_max, &font_douyin_12, LV_PART_MAIN
    | LV_STATE_DEFAULT);

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
    lv_obj_set_style_text_font(text_speed_avg, &font_douyin_12, LV_PART_MAIN
    | LV_STATE_DEFAULT);

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
    lv_obj_set_style_text_font(text_speed_sum, &font_douyin_12, LV_PART_MAIN
    | LV_STATE_DEFAULT);

    text_sensor_temp = lv_label_create(page_view);
    lv_obj_set_width(text_sensor_temp, LV_SIZE_CONTENT);   
    lv_obj_set_height(text_sensor_temp, LV_SIZE_CONTENT);  
    lv_obj_set_x(text_sensor_temp, 188);
    lv_obj_set_y(text_sensor_temp, 212);
    lv_label_set_text(text_sensor_temp, "23.3度");
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
    lv_label_set_text(text_sensor_humidity, "46.4%");
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

static void on_event_data_change(void* sub, bus_msg_t* msg) {
    app_real_record_t* dat = msg->payload;
    // 时间
    lv_label_set_text_fmt(text_current_time, "%02d:%02d",
                          dat->curr_gps.datetime.hour,
                          dat->curr_gps.datetime.minute);
    // GPS图标
    if (dat->curr_gps.sats_in_use == 0) {
        lv_obj_add_flag(icon_gps, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(icon_gps, LV_OBJ_FLAG_HIDDEN);
    }
    // GPS信息
    int speed_kmh = roundf((dat->curr_gps.speed * 3.6));
    lv_label_set_text_fmt(text_current_speed, "%02d", speed_kmh);

    // 当开启了骑行
    if (dat->is_start) {
        // 设置骑行时间
        lv_label_set_text(text_run_timeinfo,
                          format_time(dat->curr_log_dat.run_second));
        if (dat->curr_log_dat.max_speed < speed_kmh) {
            dat->curr_log_dat.max_speed = speed_kmh;
        }
        lv_label_set_text_fmt(text_speed_max, "最大(Max): %dkm/h",
                              dat->curr_log_dat.max_speed);
        // 平均速度计算
        if (dat->curr_gps.speed > 1) {
            dat->tick_gps_cnt += 1;
            dat->speed_sum += dat->curr_gps.speed;
            if (dat->tick_gps_cnt % 10 == 0) {
                uint16_t avg_speed =
                    roundf((dat->speed_sum / dat->tick_gps_cnt) * 3.6);
                lv_label_set_text_fmt(text_speed_avg, "平均(Avg): %dkm/h",
                                      avg_speed);
            }
        }
        // 计算小阶段的里程，里程=速度x时间;
        double distance = dat->curr_gps.speed * APP_GPS_FRAME;
        dat->distance += distance;
        int distance_km = (int)(dat->distance / 1000);
        lv_label_set_text_fmt(text_speed_sum, "里程(Sum): %dkm", distance_km);
    }
}
static void on_event_battery(void* sub, bus_msg_t* msg) {
    app_battery_t* battery = msg->payload;
    // 电量
    lv_label_set_text_fmt(text_battery, "%f%%", battery->level);
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
            lv_obj_set_x(icon_battery_charge, 208);
            lv_obj_set_y(icon_battery_charge, 8);
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
        if (battery->level < 20) {
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
static void on_event_env_change(void* sub, bus_msg_t* msg) {
    app_environment_t* env = msg->payload;
    if (current_real_record.is_start) {
        current_real_record.tick_environment_cnt += 1;
        current_real_record.curr_log_dat.avg_temp += (uint8_t)env->temp;
    }
    lv_label_set_text_fmt(text_sensor_temp, "%d度", (uint8_t)env->temp);
    lv_label_set_text_fmt(text_sensor_humidity, "%d%%", (uint8_t)env->humidity);
}
static void on_event_record_start(void* sub, bus_msg_t* msg) {
    lv_label_set_text(text_run_timeinfo, "0秒");
    lv_img_set_src(icon_control_state, "S:/img/record_stop.bin");
}
static void on_event_record_stop(void* sub, bus_msg_t* msg) {
    lv_img_set_src(icon_control_state, "S:/img/record_start.bin");
}
static void on_event_button(void* sub, bus_msg_t* msg) {
    app_btn_pck* key = msg->payload;
    if (key->btn_code == APP_BUTTON_UP &&
        key->btn_state == APP_BUTTON_LONG_PRESS) {
#ifndef APP_TEST
        if (current_real_record.is_start) {
            return;
        }
#endif  // APP_TEST

        // 打开设置页面
        ui_intent_t intent;
        ui_fun_fast_create_intent(_this, ACTIVITY_ID_SETTING, &intent);
        ui_fun_start_activity(&intent);
    }
}

static void on_destoy_fun(void* params) {}

static void on_stop_fun(void* params) {
    bus_unregister_subscribe(bus_data_change);
    bus_unregister_subscribe(bus_battery);
    bus_unregister_subscribe(bus_env_change);
    bus_unregister_subscribe(bus_record_start);
    bus_unregister_subscribe(bus_record_stop);
    bus_unregister_subscribe(bus_button);
}

static void on_start_fun(void* params) {
    bus_data_change = bus_register_subscribe(DATA_BUS_RECORD_CHANGE,
                                             on_event_data_change, NULL);
    bus_battery =
        bus_register_subscribe(DATA_BUS_BATTERY_EVENT, on_event_battery, NULL);
    bus_env_change =
        bus_register_subscribe(DATA_BUS_ENV_REFRESH, on_event_env_change, NULL);
    bus_record_start = bus_register_subscribe(DATA_BUS_RECORD_START,
                                              on_event_record_start, NULL);
    bus_record_stop = bus_register_subscribe(DATA_BUS_RECORD_STOP,
                                             on_event_record_stop, NULL);
    bus_button =
        bus_register_subscribe(DATA_BUS_BUTTON_EVENT, on_event_button, NULL);
}

ui_data_t dark_dial = {.id = ACTIVITY_ID_DIAL_DARK,
                       .launcher_mode = SINGLE_TASK,  // 栈内复用模式
                       .fun_get_view = fun_get_view,
                       .fun_on_create = on_create_fun,
                       .fun_on_destoy = on_destoy_fun,
                       .fun_on_stop = on_stop_fun,
                       .fun_on_start = on_start_fun};
