#include "Arduino.h"
#include "app_config.h"
#include "constant.h"
#include "ui.h"

#define _BTN_PR 1
#define _BTN_LONG_PR 2
#define _BTN_RELEASE 0

#define _BTN_TIME_SORT 5        // 短按
#define _BTN_TIME_LONG 2000000  // 长按

static struct _btn_t {
    uint8_t pin;    // IO Pin
    uint8_t state;  // See _BTN_PR
    uint32_t last_ms;
};

static void release_indev_cb(lv_event_t* e);
static void btn_keyboard_read_custom(lv_indev_t* indev, lv_indev_data_t* data);
static void hal_init_button();

/**
 * 初始化输入事件
 */
lv_indev_t* create_btn_key_indev() {
    lv_indev_t* kb = lv_indev_create();
    app_btn_pck* pck = lv_malloc_zeroed(sizeof(app_btn_pck));
    LV_ASSERT_MALLOC(pck);
    if (pck == NULL) {
        return NULL;
    }
    lv_indev_set_driver_data(kb, pck);
    lv_indev_set_type(kb, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_mode(kb, LV_INDEV_MODE_EVENT);
    lv_indev_set_read_cb(kb, btn_keyboard_read_custom);
    lv_indev_add_event_cb(kb, release_indev_cb, LV_EVENT_DELETE, kb);

    // 初始化按键扫描
    hal_init_button();
    return kb;
}

static void btn_keyboard_read_custom(lv_indev_t* indev, lv_indev_data_t* data) {
    app_btn_pck* pck = lv_indev_get_driver_data(indev);
    lv_log("PCK: code = %d,state = %d\n", pck->btn_code, pck->btn_state);
    ui_data_t* curr = ui_get_current();

    if (pck->btn_state == APP_BUTTON_PRESS) {
        data->state = LV_INDEV_STATE_PRESSED;
        // lv_indev_send_event(indev, LV_EVENT_PRESSED, NULL);
    } else if (pck->btn_state == APP_BUTTON_LONG_PRESS) {
        data->state = LV_INDEV_STATE_PRESSED;
        // lv_indev_send_event(indev, LV_EVENT_LONG_PRESSED, NULL);
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
        // lv_indev_send_event(indev, LV_EVENT_RELEASED, NULL);
    }

    if (curr != NULL && curr->fun_read_cb != NULL) {
        curr->fun_read_cb(indev, data);
        return;
    }

    if (pck->btn_code == APP_BUTTON_UP) {
        data->key = LV_KEY_UP;
    } else if (pck->btn_code == APP_BUTTON_DOWN) {
        data->key = LV_KEY_DOWN;
    } else if (pck->btn_code == APP_BUTTON_ENTER) {
        data->key = LV_KEY_ENTER;
    }
}

static void release_indev_cb(lv_event_t* e) {
    lv_indev_t* indev = (lv_indev_t*)lv_event_get_user_data(e);
    app_btn_pck* pck = lv_indev_get_driver_data(indev);
    if (pck) {
        lv_indev_set_driver_data(indev, NULL);
        lv_indev_set_read_cb(indev, NULL);
        lv_free(pck);
        LV_LOG_INFO("done");
    }
}

static void _send_btn_e(struct _btn_t* btn, lv_indev_t* indev) {
    app_btn_pck* pck = lv_indev_get_driver_data(indev);
    if (pck == NULL) {
        return;
    }
    btn->last_ms = micros();
    if (btn->state == _BTN_PR) {
        pck->btn_state = APP_BUTTON_PRESS;
    } else if (btn->state == _BTN_LONG_PR) {
        pck->btn_state = APP_BUTTON_LONG_PRESS;
    } else if (btn->state == _BTN_RELEASE) {
        pck->btn_state = APP_BUTTON_RELEASE;
    } else {
        lv_log("Error: 错误的按键类型");
        return;
    }
    if (btn->pin == IO_BTN_UP) {
        pck->btn_code = APP_BUTTON_UP;
    } else if (btn->pin == IO_BTN_DOWN) {
        pck->btn_code = APP_BUTTON_DOWN;
    } else {
        pck->btn_code = APP_BUTTON_ENTER;
    }
    lv_lock();
    lv_indev_read(indev);
    lv_unlock();
}

/**
 * 按键扫描
 */
static void _scan_button(void* params) {
    struct _btn_t btn_arr[3] = {
        {.pin = IO_BTN_DOWN, .state = _BTN_RELEASE, .last_ms = 0},
        {.pin = IO_BTN_UP, .state = _BTN_RELEASE, .last_ms = 0},
        {.pin = IO_BTN_ENTER, .state = _BTN_RELEASE, .last_ms = 0}};
    uint8_t index = 0;
    while (1) {
        lv_indev_t* indev = lv_indev_get_next(NULL);
        if (indev == NULL) {
            return;
        }
        for (index = 0; index < 3; index++) {
            struct _btn_t* btn = &btn_arr[index];
            vTaskDelay(pdMS_TO_TICKS(_BTN_TIME_SORT));
            uint8_t level = digitalRead(btn->pin);
            if (!level && btn->state == _BTN_RELEASE) {
                // 检测到低电平并且原先状态并不是低电平
                btn->state = _BTN_PR;
                // 发送事件
                _send_btn_e(btn, indev);
                break;
            } else if (level && btn->state != _BTN_RELEASE) {
                // 高电平释放
                btn->state = _BTN_RELEASE;
                // 发送事件
                _send_btn_e(btn, indev);
                break;
            }
            uint32_t tim_diff = micros() - btn->last_ms;
            // 是否满足长按的时长要求
            if (tim_diff > _BTN_TIME_LONG && btn->state == _BTN_PR) {
                btn->state = _BTN_LONG_PR;
                // 发送事件
                _send_btn_e(btn, indev);
            }
        }
    }
    vTaskDelete(NULL);
}

/**
 * 初始化按键
 */
static void hal_init_button() {
    pinMode(IO_BTN_DOWN, INPUT);
    pinMode(IO_BTN_UP, INPUT);
    pinMode(IO_BTN_ENTER, INPUT);
    xTaskCreate(_scan_button, "BS", 4096, NULL, 2, NULL);
}