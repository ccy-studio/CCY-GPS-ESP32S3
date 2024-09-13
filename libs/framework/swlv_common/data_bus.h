#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "app_config.h"

#define BUS_TYPE_FREERTOS 0   /** RTOS队列模式 */
#define BUS_TYPE_LVGL_ASYNC 1 /** LVGL异步通知模式 */

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

typedef uint8_t bus_event;  // 事件类型

typedef struct {
    bus_event id;
    void* user_data;
    void* payload;
} bus_msg_t;

#if BUS_TYPE_FREERTOS
extern QueueHandle_t data_bus_queue;
#endif

typedef void (*bus_msg_subscribe_cb_t)(void* subscribe, bus_msg_t* msg);

/* 记录当前骑行的行驶数据 */
extern app_real_record_t current_real_record;

/**
 * @brief 注册消息订阅
 * @param event 订阅的事件
 * @param cb 回调函数
 * @param user_data 用户数据
 * @return
 */
void* bus_register_subscribe(bus_event event,
                             bus_msg_subscribe_cb_t cb,
                             void* user_data);

/**
 * @brief 解除消息订阅
 * @param subscribe
 */
void bus_unregister_subscribe(void* subscribe);

/**
 * @brief 发送事件
 * @param event 事件类型
 * @param payload 发送的数据
 */
void bus_send(bus_event event, void* payload);

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
void notify_data_change();

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

#ifdef APP_TEST

void bus_test_msg();

#endif  // APP_TEST

#ifdef __cplusplus
}
#endif
