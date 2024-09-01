/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <inttypes.h>
#include <stdio.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "ui.h"

static lv_display_t* hal_init(int32_t w, int32_t h);

void app_main(void) {
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
    ui_init_and_start();

    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelete(NULL);
}

static lv_display_t* hal_init(int32_t w, int32_t h) {
    // LVGL 需要系统滴答来了解动画和其他任务的经过时间。
    lv_tick_set_cb(xTaskGetTickCount);
    // 设置显示接口
}
