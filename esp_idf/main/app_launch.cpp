/*
 * @Description:
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2024-09-02 11:39:24
 * @LastEditTime: 2024-09-04 18:00:04
 */
/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <inttypes.h>
#include <stdio.h>
#include "Arduino.h"
#include "constant.h"
#include "driver/gpio.h"
// #include "drivers/display/tft_espi/lv_tft_espi.h"
#include "lv_tft_espi.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
// #include "ui.h"
#include "lvgl.h"
#include "lv_demos.h"
#include "lv_examples.h"

#define MY_DISP_HOR_RES 240
#define MY_DISP_VER_RES 300

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565))

static lv_display_t* hal_init(int32_t w, int32_t h);

extern "C" void app_main(void) {
    initArduino();
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

    printf("=====>>>> initialize lvgl....\n");
    lv_init();
    hal_init(240, 240);

    printf("=====>>>> launch ui pages\n");
    // ui_init_and_start();
    lv_demo_widgets();


    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelete(NULL);
}

/**
 * 外部输入事件
 */
static void hal_button_indev_cb(lv_indev_t * indev, lv_indev_data_t * data){

}

static lv_display_t* hal_init(int32_t w, int32_t h) {
    // LVGL 需要系统滴答来了解动画和其他任务的经过时间。
    lv_tick_set_cb(xTaskGetTickCount);
    // 创建绘制缓冲区
    // 1/10的缓存
    static uint8_t
        buf1[MY_DISP_HOR_RES * MY_DISP_VER_RES / 10 * BYTE_PER_PIXEL];
    lv_display_t* display = lv_tft_espi_create(MY_DISP_HOR_RES, MY_DISP_VER_RES,
                                               buf1, sizeof(buf1));

    lv_indev_t* indev = lv_indev_create(); /*Create an input device*/
    lv_indev_set_type(
        indev, LV_INDEV_TYPE_BUTTON); /*Touch pad is a pointer-like device*/
        lv_indev_set_read_cb(indev,hal_button_indev_cb);

    lv_indev_set_display(indev, display);
    lv_display_set_default(display);

    // // 初始化屏幕的PWM调光
    // gpio_config_t pwm_io = {
    //     .pin_bit_mask = BIT(IO_PWM_PIN),
    //     .mode = GPIO_MODE_OUTPUT,
    // };
    // gpio_config(&pwm_io);
    // gpio_set_level(IO_PWM_PIN, 1);

    pinMode(IO_PWM_PIN,OUTPUT);
    digitalWrite(IO_PWM_PIN,1);
    return display;
}
