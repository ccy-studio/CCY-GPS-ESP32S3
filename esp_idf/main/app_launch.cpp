/*
 * @Description:
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2024-09-02 11:39:24
 * @LastEditTime: 2024-09-06 15:39:21
 */
/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <inttypes.h>
#include <stdio.h>
#include "./include/interface_impl.h"
#include "Arduino.h"
#include "constant.h"
#include "driver/gpio.h"
#include "drivers/display/tft_espi/lv_tft_espi.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "ui.h"
// #include "lv_demos.h"
// #include "lv_examples.h"
#include "lvgl.h"

/**
 * 屏幕的尺寸
 */
#define MY_DISP_HOR_RES 240
#define MY_DISP_VER_RES 280

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565))

extern "C" {

app_environment_t global_env;
app_gps_t global_gps;
app_battery_t global_battery;

lv_indev_t* create_btn_key_indev();
static lv_display_t* hal_init(int32_t w, int32_t h);

void app_main(void) {
    initArduino();
    Serial.begin(115200);
    printf("Hello Saisaiwa GPS ESP32Project!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ", CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154)
               ? ", 802.15.4 (Zigbee/Thread)"
               : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded"
                                                         : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n",
           esp_get_minimum_free_heap_size());

    // 判读开机时间是否满足最小长按时间限制
    pinMode(IO_POWER_EN_PIN, INPUT);
    vTaskDelay(pdMS_TO_TICKS(2000));
    if (!digitalRead(IO_POWER_EN_PIN)) {
        // 满足
        pinMode(IO_POWER_EN_PIN, OUTPUT);
        digitalWrite(IO_POWER_EN_PIN, 1);
    } else {
        while (1) {
            vTaskDelay(1);
        }
    }

    lv_init();
    hal_init(240, 240);

    ui_init_and_start();
    // lv_demo_widgets();
    // lv_demo_widgets_start_slideshow();

    interface_initialize();
    uint32_t time_till_next;
    while (1) {
        time_till_next = lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(time_till_next));
    }
}

static lv_display_t* hal_init(int32_t w, int32_t h) {
    // LVGL 需要系统滴答来了解动画和其他任务的经过时间。
    lv_tick_set_cb(xTaskGetTickCount);
    // 创建绘制缓冲区 - 双缓冲区
    size_t len = MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL;
    void* buf1 = heap_caps_malloc(len, MALLOC_CAP_SPIRAM);
    // void* buf2 = heap_caps_malloc(len, MALLOC_CAP_SPIRAM);

    lv_group_set_default(lv_group_create());
    lv_display_t* display =
        lv_tft_espi_create(MY_DISP_HOR_RES, MY_DISP_VER_RES, buf1, NULL, len);

    lv_indev_t* indev = (lv_indev_t*)create_btn_key_indev();
    lv_indev_set_group(indev, lv_group_get_default());
    lv_indev_set_display(indev, display);
    lv_display_set_default(display);

    // 初始化屏幕的PWM调光
    pinMode(IO_PWM_PIN, OUTPUT);
    return display;
}
}