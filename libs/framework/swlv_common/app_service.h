#ifndef __APP_SER_H
#define __APP_SER_H

#include "data_bus.h"

/* 定义DataBus数据类型 */
#define DATA_BUS_RECORD_STOP 0x01   /*骑行结束*/
#define DATA_BUS_RECORD_START 0x02  /*骑行开始*/
#define DATA_BUS_RECORD_CHANGE 0x03 /*运行数据改变事件*/
#define DATA_BUS_POWER_EVENT 0x04   /*关机触发事件*/
#define DATA_BUS_DIAL_CHANGE 0x05   /*表盘样式改变触发事件*/
#define DATA_BUS_GPS_REFRESH 0x06   /*GPS刷新事件*/
#define DATA_BUS_ENV_REFRESH 0x07   /*环境信息刷新事件*/
#define DATA_BUS_BUTTON_EVENT 0x08  /*触发物理按键事件*/
#define DATA_BUS_BATTERY_EVENT 0x09 /*电池电量改变事件*/
#define DATA_BUS_BL_EVENT 0x10      /*屏幕背光事件*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 开启骑行
 */
void app_start_record();

/**
 * @brief 结束骑行
 */
void app_stop_record();

/**
 * @brief 通知-实时运行数据改变
 */
void notify_data_change(bool is_time);

/**
 * @brief 通知-关机
 */
void notify_power_close();

/**
 * @brief 通知-表盘样式改变
 */
void notify_dial_change();

/**
 * @brief 通知-GPS刷新时间
 * @param gps_dat
 */
void notify_gps_refresh(app_gps_t* gps_dat);

/**
 * @brief 通知-环境信息数据刷新的事件
 * @param evt_dat
 */
void notify_env_refresh(app_environment_t* evt_dat);

/**
 * @brief 物理按键触发事件
 */
void notify_button_event(app_btn_pck* btn);

/**
 * @brief 通知-电池电量刷新事件
 * @param battery_info
 */
void notify_battery_event(app_battery_t* battery_info);


/**
 * 通知屏幕背光开关
 */
void notify_bl_event(app_bl_pwm state);

#ifdef __cplusplus
}
#endif
#endif