#include "data_bus.h"
#include <string.h>
#include "stdlib.h"

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

void* bus_register_subscribe(bus_event event,
                             bus_msg_subscribe_cb_t cb,
                             void* user_data) {
    if (cb == NULL) {
        printf("Error: bus_register_subscribe cb is not null\n");
        return NULL;
    }
    bus_desc_t* new_desc = (bus_desc_t*)malloc(sizeof(bus_desc_t));
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
        free(bus_root);
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
            free(temp);
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

void bus_send(bus_event event, void* payload) {
    if (bus_root == NULL) {
        return;
    }
    bus_desc_t* next = bus_root;
    while (next != NULL) {
        // if (next->callback != NULL && (next->id & event) == event) {
        if (next->callback != NULL && next->id == event) {
            bus_msg_t msg = {
                .id = event, .user_data = next->user_data, .payload = payload};
            next->callback(msg);
        }
        next = next->next;
    }
}

/**
 * 通用方法
 */
// void bus_send(bus_event event, void* payload) {
// #if BUS_TYPE_FREERTOS
//     bus_msg_t msg_q = {.id = event, .payload = payload};
//     xQueueSend(data_bus_queue, &msg_q, portMAX_DELAY);
// #endif

// #if BUS_TYPE_LVGL_ASYNC
//     if (ev_timer == NULL) {
//         ev_timer = lv_timer_create(on_lvgl_update_ui_timer, 300, NULL);
//     }
// #endif
//     if (bus_root == NULL) {
//         return;
//     }
//     bus_desc_t* temp = bus_root;
//     bus_desc_t** arr = NULL;
//     uint8_t cnt = 0;
//     while (temp != NULL) {
//         if ((temp->id & event) == event && temp->callback != NULL) {
//             cnt++;
//         }
//         temp = temp->next;
//     }
//     temp = bus_root;
//     arr = (bus_desc_t**)malloc(sizeof(bus_desc_t*) * cnt);
//     if (arr == NULL) {
//         printf("bus_send create point fail\n");
//         goto fail;
//     }
//     memset(arr, 0, sizeof(bus_desc_t*) * cnt);
//     uint8_t index = 0;
//     while (temp != NULL) {
//         if ((temp->id & event) == event && temp->callback != NULL) {
//             arr[index++] = temp;
//         }
//         temp = temp->next;
//     }

//     for (uint8_t i = 0; i < cnt; i++) {
//         if (arr[i] && arr[i] != NULL) {
// #if BUS_TYPE_LVGL_ASYNC
//             struct lvgl_call_t* ct = malloc(sizeof(struct lvgl_call_t));
//             if (ct == NULL) {
//                 LV_LOG_ERROR("lv_malloc FAIL ");
//                 goto fail;
//             }
//             ct->callback = arr[i]->callback;
//             ct->msg.id = event;
//             ct->msg.user_data = arr[i]->user_data;
//             ct->msg.payload = payload;
//             ct->user_data = arr[i]->user_data;

//             while (ev_task_arr != NULL) {
//             }

//             ev_task_arr = ct;
// #else
//             bus_msg_t msg = {.id = event,
//                              .user_data = arr[i]->user_data,
//                              .payload = payload};
//             arr[i]->callback(arr[i], &msg);
// #endif
//         }
//     }
// fail:
//     free(arr);
//     arr = NULL;
// }