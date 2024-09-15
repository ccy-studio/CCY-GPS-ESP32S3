#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "app_config.h"

typedef uint8_t bus_event;  // 事件类型

typedef struct {
    bus_event id;
    void* user_data;
    void* payload;
} bus_msg_t;

typedef void (*bus_msg_subscribe_cb_t)(bus_msg_t msg);

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


#ifdef __cplusplus
}
#endif
