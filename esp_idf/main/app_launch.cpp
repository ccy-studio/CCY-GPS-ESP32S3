/*
 * @Description:
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2024-09-02 11:39:24
 * @LastEditTime: 2024-09-05 17:38:54
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
#include "drivers/display/tft_espi/lv_tft_espi.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
// #include "ui.h"
#include "lv_demos.h"
#include "lv_examples.h"
#include "lvgl.h"

/**
 * 屏幕的尺寸
 */
#define MY_DISP_HOR_RES 240
#define MY_DISP_VER_RES 280

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565))

static lv_display_t* hal_init(int32_t w, int32_t h);

static void test_ui() {
    lv_disp_t* dispp = lv_disp_get_default();
    lv_theme_t* theme = lv_theme_default_init(
        dispp, lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    lv_obj_t* scr = lv_obj_create(NULL);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);

    lv_obj_t* product = lv_label_create(scr);
    lv_obj_set_width(product, LV_SIZE_CONTENT);
    lv_obj_set_height(product, LV_SIZE_CONTENT);
    lv_obj_center(product);
    lv_label_set_text(product, "Saisaiwa");
    lv_obj_set_style_text_color(product, lv_color_hex(0xfb8b05), LV_PART_MAIN);

    lv_obj_t* text = lv_label_create(scr);
    lv_obj_set_width(text, LV_SIZE_CONTENT);
    lv_obj_set_height(text, LV_SIZE_CONTENT);
    lv_obj_set_pos(text, 0, 0);
    lv_label_set_text_fmt(text, "LeftPoint");
    lv_obj_set_style_text_color(text, lv_color_hex(0x1ba784), LV_PART_MAIN);

    text = lv_label_create(scr);
    lv_obj_set_width(text, LV_SIZE_CONTENT);
    lv_obj_set_height(text, LV_SIZE_CONTENT);
    lv_obj_set_pos(text, lv_disp_get_hor_res(dispp) - 28,
                   lv_disp_get_ver_res(dispp) / 2);
    lv_label_set_text_fmt(text, "RightPoint");
    lv_obj_set_style_text_color(text, lv_color_hex(0x1ba784), LV_PART_MAIN);

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
}


/**
 * https://github.com/mikalhart/TinyGPSPlus/blob/master/examples/FullExample/FullExample.ino
 */
static void gps_thread(void* params) {
    Serial1.begin(9600, SERIAL_8N1, IO_GPS_RX, IO_GPS_TX);
    delay(1000);
    size_t len = sizeof(char) * 50;
    char* buf = (char*)heap_caps_malloc(len, MALLOC_CAP_SPIRAM);
    size_t i = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
        memset(buf, 0, len);
        i = 0;
        if (Serial1.available()) {
            Serial1.read(buf + i, 1);
            i++;
        }
        printf(buf);
    }
    heap_caps_free(buf);
}

extern "C" void app_main(void) {
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

    pinMode(IO_POWER_EN_PIN, OUTPUT);
    digitalWrite(IO_POWER_EN_PIN, 1);

    printf("=====>>>> initialize lvgl....\n");
    // lv_init();
    // hal_init(240, 240);

    printf("=====>>>> launch ui pages\n");
    // ui_init_and_start();
    // lv_demo_widgets();
    // lv_demo_widgets_start_slideshow();

    // test_ui();

    xTaskCreate(gps_thread, "GPS", 4096, NULL, 1, NULL);

    while (1) {
        lv_timer_handler();
        delay(10);
    }
}

/**
 * 外部输入事件
 */
static void hal_button_indev_cb(lv_indev_t* indev, lv_indev_data_t* data) {}

static lv_display_t* hal_init(int32_t w, int32_t h) {
    // LVGL 需要系统滴答来了解动画和其他任务的经过时间。
    lv_tick_set_cb(xTaskGetTickCount);
    // 创建绘制缓冲区 - 双缓冲区
    size_t len = MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL;
    void* buf1 = heap_caps_malloc(len, MALLOC_CAP_SPIRAM);
    void* buf2 = heap_caps_malloc(len, MALLOC_CAP_SPIRAM);

    lv_display_t* display =
        lv_tft_espi_create(MY_DISP_HOR_RES, MY_DISP_VER_RES, buf1, buf2, len);

    lv_indev_t* indev = lv_indev_create(); /*Create an input device*/
    lv_indev_set_type(
        indev, LV_INDEV_TYPE_BUTTON); /*Touch pad is a pointer-like device*/
    lv_indev_set_read_cb(indev, hal_button_indev_cb);

    lv_indev_set_display(indev, display);
    lv_display_set_default(display);

    // // 初始化屏幕的PWM调光
    pinMode(IO_PWM_PIN, OUTPUT);
    // digitalWrite(IO_PWM_PIN, 1);
    analogWrite(IO_PWM_PIN, 100);
    return display;
}
