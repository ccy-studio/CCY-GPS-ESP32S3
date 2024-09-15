#include "app_service.h"
#include <stdlib.h>
#include <string.h>
#include "store.h"

app_real_record_t global_real_record;  // 骑行日志记录

void app_start_record() {
    if (global_real_record.is_start) {
        return;
    }
    memset(&global_real_record, 0, sizeof(global_real_record));
    global_real_record.is_start = true;
    sprintf(&global_real_record.curr_log_dat.start_time, "%d:%d:%d",
            global_gps.datetime.hour, global_gps.datetime.minute,
            global_gps.datetime.second);
    bus_send(DATA_BUS_RECORD_START, &global_real_record);
}

void app_stop_record() {
    if (!global_real_record.is_start) {
        return;
    }
    global_real_record.is_start = false;
    uint32_t* cnt = &global_real_record.tick_cnt;
    app_run_log_t* log = &global_real_record.curr_log_dat;
    sprintf(&log->date, "%d-%d-%d", global_gps.datetime.year,
            global_gps.datetime.month, global_gps.datetime.day);
    sprintf(&log->end_time, "%d:%d:%d", global_gps.datetime.hour,
            global_gps.datetime.minute, global_gps.datetime.second);

    log->avg_speed = global_real_record.sum_speed / *cnt;
    log->avg_temp = global_real_record.sum_temp / *cnt;
    log->avg_speed = global_real_record.sum_speed / *cnt;
    bus_send(DATA_BUS_RECORD_STOP, &global_real_record);
    app_store_save_run_log(&global_real_record);
}
/**
 * @brief 通知-实时运行数据改变
 */
void notify_data_change(bool is_time) {
    if (!global_real_record.is_start) {
        return;
    }
    if (global_gps.datetime.second != global_real_record.last_sencond) {
        global_real_record.last_sencond = global_gps.datetime.second;
        global_real_record.curr_log_dat.run_second++;
    }
    if (is_time) {
        return;
    }
    global_real_record.tick_cnt++;
    memcpy(&global_real_record.curr_gps, &global_gps, sizeof(app_gps_t));
    memcpy(&global_real_record.env_dat, &global_env, sizeof(app_environment_t));
    app_run_log_t* log = &global_real_record.curr_log_dat;
    // 最大骑行速度
    if (log->max_speed < global_gps.speed) {
        log->max_speed = global_gps.speed;
    }
    // 计算平均速度
    if (global_gps.speed > 2) {
        global_real_record.sum_speed += global_gps.speed;
    }
    // 计算小阶段的里程，里程=速度x时间;
    log->mileage += global_gps.distance;

    // 平均温湿度的累加
    global_real_record.sum_hum += global_env.humidity;
    global_real_record.sum_temp += global_env.temp;

    bus_send(DATA_BUS_RECORD_CHANGE, &global_real_record);
    app_store_save_run_log(&global_real_record);
}

/**
 * @brief 通知-关机
 */
void notify_power_close() {
    bus_send(DATA_BUS_POWER_EVENT, NULL);
}

/**
 * @brief 通知-表盘样式改变
 */
void notify_dial_change() {
    bus_send(DATA_BUS_DIAL_CHANGE, NULL);
}

/**
 * @brief 通知-GPS刷新时间
 * @param gps_dat
 */
void notify_gps_refresh(app_gps_t* gps_dat) {
    bus_send(DATA_BUS_GPS_REFRESH, gps_dat);
}

/**
 * @brief 通知-环境信息数据刷新的事件
 * @param evt_dat
 */
void notify_env_refresh(app_environment_t* evt_dat) {
    bus_send(DATA_BUS_ENV_REFRESH, evt_dat);
}

void notify_button_event(app_btn_pck* btn) {
    bus_send(DATA_BUS_BUTTON_EVENT, btn);
}

/**
 * @brief 通知-电池电量刷新事件
 * @param battery_info
 */
void notify_battery_event(app_battery_t* battery_info) {
    bus_send(DATA_BUS_BATTERY_EVENT, battery_info);
}

void notify_bl_event(app_bl_pwm state) {
    bus_send(DATA_BUS_BL_EVENT, &state);
}