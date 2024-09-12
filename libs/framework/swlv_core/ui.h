/*
 * @Description:
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2023-12-22 15:04:07
 * @LastEditTime: 2024-01-05 16:39:02
 */
#ifndef __GUI_H
#define __GUI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "data_bus.h"
#include "lvgl.h"
#include "util.h"
#include <string.h>

#define MAX_PAGE 10

typedef enum {
    ACTIVITY_ID_START = 0,
    ACTIVITY_ID_DIAL_DARK,
    ACTIVITY_ID_SETTING,
} activity_id_t;

typedef enum {
    SINGLE_TOP = 0,  // 栈顶复用模式
    STANDARD,  // 标准模式(最基本的模式，每次启动都会创建一个新的 ACTIVITY)
    SINGLE_TASK,  // 栈内复用模式(如果栈内复用，那么会 Clear Task 中这个
                  // Activity
    // 上面的其他的 Activity)
} launch_mode_t;

typedef struct _ui_data_t ui_data_t;

/**
 * @brief 事件监听回调
 */
typedef void (*handle_event)(bus_event event, void* params);

/**
 * activity的声明周期：
 * on_create -> on_start -> on_stop -> on_destoy
 * 暂停重新启动时： on_stop -> on_restart -> on_start
 */
typedef void (*on_create)(ui_data_t* ui_dat, void* params);
typedef void (*on_start)(void* params);
typedef void (*on_restart)(void* params);
typedef void (*on_stop)(void* params);
typedef void (*on_destoy)(void* params);
typedef lv_obj_t* (*get_view)();

/**
 * Activity的Base基础核心结构体
 */
struct _ui_data_t {
    activity_id_t id;
    bool launcher;  // 是否作为主启动器，只能允许一个存在
    launch_mode_t launcher_mode;  // 启动模式
    get_view fun_get_view;        // 获取View对象
    handle_event fun_handle_event;
    on_create fun_on_create;
    on_start fun_on_start;
    on_restart fun_on_restart;
    on_stop fun_on_stop;
    on_destoy fun_on_destoy;
    bool is_createed;
    lv_group_t* group;  // KeyGroup
    lv_indev_read_cb_t fun_read_cb; //每个页面可以自定义按键的触发类型
};

typedef struct {
    ui_data_t* _this;
    ui_data_t* target;
    void* params;
    bool anim;  // 是否开启切换动画
} ui_intent_t;

void ui_init_and_start();

/*
 获取当前的页面Activity
*/
ui_data_t* ui_get_current();


/**
 * @brief 快速创建简单的Intent意图
 * @param _this 当前Activity
 * @param target 调整的目标Activity的ID
 * @param intent 指向Intent结构体的指针
 */
void ui_fun_fast_create_intent(ui_data_t* _this,
                               activity_id_t target,
                               ui_intent_t* intent);

/**
 * 跳转Activity
 */
void ui_fun_start_activity(ui_intent_t* intent);

/**
 * @brief 关闭并销毁指定的Activity
 * @param _this 指定的ActivityID
 * @param anim 是否开启动画
 */
void ui_fun_finish(ui_data_t* _this, bool anim);

void _ui_load_scr(lv_obj_t* scr);
void _ui_load_scr_opa(lv_obj_t* scr);

void ui_error(char* error);

/**
 * @brief 向栈顶当前活动的Activity发送Event事件
 * @param event 事件类型
 * @param params 用户数据
 */
void ui_send_event_current(bus_event event, void* params);

/**
 * @brief 向指定的Activity发送事件
 * @param activity ActivityID
 * @param event 事件类型
 * @param params 用户数据
 */
void ui_send_event(activity_id_t activity, bus_event event, void* params);

/**
 * @brief 向所有的Activity发送事件
 * @param event 事件类型
 * @param params 用户数据
 */
void ui_send_event_all(bus_event event, void* params);

/**
 * @brief
 * 动态滚动条,在可滚动容器内监听按LV_EVENT_FOCUSED事件并滚动到此对应的组件Y轴位置
 * @param e
 */
void ui_callback_dynamic_scrollbar(lv_event_t* e);

#ifdef __cplusplus
}
#endif
#endif
