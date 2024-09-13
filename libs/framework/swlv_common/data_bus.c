#include "data_bus.h"
#include <string.h>
#include "stdlib.h"
#if BUS_TYPE_FREERTOS
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#endif
#if BUS_TYPE_LVGL_ASYNC
#include "lvgl.h"
#endif

#if BUS_TYPE_FREERTOS
QueueHandle_t data_bus_queue;
#endif

typedef struct _bus_desc_t bus_desc_t;

struct _bus_desc_t {
    bus_event id;
    void* user_data;
    bus_msg_subscribe_cb_t callback;
    bus_desc_t* pre;
    bus_desc_t* next;
};

static bus_desc_t* bus_root = NULL;
static uint8_t bus_cnt = 0;

/* 记录当前骑行的行驶数据 */
app_real_record_t global_real_record;

void* bus_register_subscribe(bus_event event,
                             bus_msg_subscribe_cb_t cb,
                             void* user_data) {
    if (cb == NULL) {
        printf("Error: bus_register_subscribe cb is not null\n");
        return NULL;
    }
    bus_desc_t* new_desc = (bus_desc_t*)lv_malloc_zeroed(sizeof(bus_desc_t));
    if (new_desc == NULL) {
        printf("Error: create subscribe NULL\n");
        return NULL;
    }
    memset(new_desc, 0, sizeof(bus_desc_t));
    new_desc->id = event;
    new_desc->callback = cb;
    new_desc->user_data = user_data;
    new_desc->next = NULL;
    new_desc->pre = NULL;
    if (bus_root != NULL) {
        bus_desc_t* last = bus_root;
        while (last != NULL) {
            if (last->next == NULL) {
                break;
            }
            last = last->next;
        }
        last->next = new_desc;
        new_desc->pre = last;
    } else {
        bus_root = new_desc;
    }
    bus_cnt++;
    return new_desc;
}

void bus_unregister_subscribe(void* subscribe) {
    if (subscribe == NULL || bus_root == NULL) {
        return;
    }
    if (bus_root != NULL && bus_root == subscribe && bus_root->next == NULL) {
        lv_free(bus_root);
        bus_root = NULL;
        bus_cnt--;
        return;
    }
    bool is_root = bus_root == subscribe;
    bus_desc_t* temp = bus_root;
    while (temp != NULL) {
        if (temp == subscribe) {
            // Found it
            bus_desc_t* next = temp->next;
            bus_desc_t* pre = temp->pre;
            if (next != NULL) {
                next->pre = pre;
            }
            if (pre != NULL) {
                pre->next = next;
            }
            lv_free(temp);
            temp = NULL;
            if (is_root) {
                bus_root = next;
            }
            break;
        }
        temp = temp->next;
    }
    bus_cnt--;
}

#if BUS_TYPE_LVGL_ASYNC
struct lvgl_call_t {
    bus_msg_subscribe_cb_t callback;
    void* user_data;
    bus_msg_t msg;
};
static void on_lvgl_async_cb(void* params) {
    struct lvgl_call_t* p = (struct lvgl_call_t*)params;
    if (p == NULL) {
        LV_LOG_ERROR("on_lvgl_async_cb bus struct lvgl_call_t* p == NULL");
        return;
    }
    p->callback(p->user_data, &p->msg);
    lv_free(p);
    p = NULL;
}
#endif

/**
 * 通用方法
 */
void bus_send(bus_event event, void* payload) {
#if BUS_TYPE_FREERTOS
    bus_msg_t msg_q = {.id = event, .payload = payload};
    xQueueSend(data_bus_queue, &msg_q, portMAX_DELAY);
#endif
    if (bus_root == NULL) {
        return;
    }
    bus_desc_t* temp = bus_root;
    bus_desc_t** arr = NULL;
    uint8_t cnt = 0;
    while (temp != NULL) {
        if ((temp->id & event) == event && temp->callback != NULL) {
            cnt++;
        }
        temp = temp->next;
    }
    temp = bus_root;
    arr = (bus_desc_t**)lv_malloc_zeroed(sizeof(bus_desc_t*) * cnt);
    if (arr == NULL) {
        printf("bus_send create point fail\n");
        return;
    }
    memset(arr, 0, sizeof(bus_desc_t*) * cnt);
    uint8_t index = 0;
    while (temp != NULL) {
        if ((temp->id & event) == event && temp->callback != NULL) {
            arr[index++] = temp;
        }
        temp = temp->next;
    }
    for (uint8_t i = 0; i < cnt; i++) {
        if (arr[i] && arr[i] != NULL) {
#if BUS_TYPE_LVGL_ASYNC
            struct lvgl_call_t* ct =
                lv_malloc_zeroed(sizeof(struct lvgl_call_t));
            if (ct == NULL) {
                LV_LOG_ERROR("lv_malloc FAIL ");
                return;
            }
            ct->callback = arr[i]->callback;
            ct->msg.id = event;
            ct->msg.user_data = arr[i]->user_data;
            ct->msg.payload = payload;
            ct->user_data = arr[i]->user_data;
            lv_async_call(on_lvgl_async_cb, ct);
#else
            bus_msg_t msg = {.id = event,
                             .user_data = arr[i]->user_data,
                             .payload = payload};
            arr[i]->callback(arr[i], &msg);
#endif
        }
    }
    lv_free(arr);
    arr = NULL;
}

void app_start_record() {
    memset(&global_real_record, 0, sizeof(global_real_record));
    global_real_record.is_start = true;
    bus_send(DATA_BUS_RECORD_START, &global_real_record);
}

void app_stop_record() {
    global_real_record.is_start = false;
    bus_send(DATA_BUS_RECORD_STOP, &global_real_record);
}

/**
 * @brief 通知-实时运行数据改变
 */
void notify_data_change() {
    bus_send(DATA_BUS_RECORD_CHANGE, &global_real_record);
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

#ifdef APP_TEST
#include "lvgl/lvgl.h"
// 随机数生成测试
#include <time.h>
static void test_timer(lv_timer_t* t) {
    static gps_cnt = 0, env_cnt = 0;
    gps_cnt++;
    env_cnt++;
    if (gps_cnt == 5) {
        // send gps message
        app_gps_t* gps = &global_real_record.curr_gps;
        gps->sats_in_use = !gps->sats_in_use;
        int randomNumber;
        randomNumber = (rand() % 100) + 20;  // 生成1到100之间的随机数
        gps->speed = randomNumber;
        randomNumber = (rand() % 3000) + 1000;
        gps->altitude = randomNumber;
        randomNumber = (rand() % 3000) + 1000;
        gps->latitude = randomNumber;
        randomNumber = (rand() % 3000) + 1000;
        gps->longitude = randomNumber;

        time_t rawtime;
        struct tm* timeinfo;
        time(&rawtime);                  // 获取当前时间
        timeinfo = localtime(&rawtime);  // 将时间转换为本地时间
        global_real_record.curr_gps.datetime.hour = timeinfo->tm_hour;
        global_real_record.curr_gps.datetime.minute = timeinfo->tm_min;

        randomNumber = (rand() % 3600 * 3) + 3600;  // 生成1到100之间的随机数
        global_real_record.curr_log_dat.run_second = randomNumber;

        gps_cnt = 0;
        notify_data_change();
        notify_gps_refresh(gps);
    }
    if (env_cnt == 10) {
        // send env message
        static app_environment_t env_t;
        int randomNumber;
        randomNumber = (rand() % 25) + 1;
        env_t.temp = randomNumber;
        randomNumber = (rand() % 70) + 1;
        env_t.humidity = randomNumber;

        notify_env_refresh(&env_t);

        static app_battery_t battery = {.is_charge = false, .level = 98};
        notify_battery_event(&battery);
        env_cnt = 0;
    }
}

void bus_test_msg() {
    srand(time(NULL));  // 使用当前时间作为随机数种子
    lv_timer_create(test_timer, 100, NULL);
    app_start_record();
}

#endif  // APP_TEST
