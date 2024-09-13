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
#include "./include/interface_impl.h"
#include "data_bus.h"
#include "lv_demos.h"
#include "lv_examples.h"
#include "lvgl.h"

/**
 * 屏幕的尺寸
 */
#define MY_DISP_HOR_RES 240
#define MY_DISP_VER_RES 280

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565))

static lv_obj_t* battery;
static lv_obj_t* time_obj;
static lv_obj_t* gps;
static lv_obj_t* env;

static lv_display_t* hal_init(int32_t w, int32_t h);

static app_battery_t battery_dat;
static app_environment_t env_dat;
static app_gps_t gps_dat;

void refresh_async_cb(void* params) {
    lv_label_set_text_fmt(battery, "SOC:%.2f%%,Charge: %d", battery_dat.level,
                          battery_dat.is_charge);

    lv_label_set_text_fmt(env, "Temp:%.2f,Hum: %.2f", env_dat.temp,
                          env_dat.humidity);

    lv_label_set_text_fmt(time_obj, "%d-%d-%d %d:%d:%d", gps_dat.datetime.year,
                          gps_dat.datetime.month, gps_dat.datetime.day,
                          gps_dat.datetime.hour, gps_dat.datetime.minute,
                          gps_dat.datetime.second);
    lv_label_set_text_fmt(gps, "lat:%.2lf\nlng:%.2lf\nspeed:%.2lf\nstate:%d",
                          gps_dat.latitude, gps_dat.longitude, gps_dat.speed,
                          gps_dat.sats_in_use);
}

void bus_event_cb(void* subscribe, bus_msg_t* msg) {
    if (msg == NULL || msg->payload == NULL) {
        return;
    }
    if (msg->id == DATA_BUS_BATTERY_EVENT) {
        memcpy(&battery_dat, msg->payload, sizeof(app_battery_t));
    } else if (msg->id == DATA_BUS_ENV_REFRESH) {
        memcpy(&env_dat, msg->payload, sizeof(app_environment_t));
    } else if (msg->id == DATA_BUS_GPS_REFRESH) {
        memcpy(&gps_dat, msg->payload, sizeof(app_gps_t));
    }
    lv_async_call(refresh_async_cb, NULL);
}

static void test_ui() {
    lv_disp_t* dispp = lv_disp_get_default();
    lv_theme_t* theme = lv_theme_default_init(
        dispp, lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    lv_obj_t* scr = lv_obj_create(NULL);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);

    gps = lv_label_create(scr);
    lv_obj_set_width(gps, LV_SIZE_CONTENT);
    lv_obj_set_height(gps, LV_SIZE_CONTENT);
    lv_obj_center(gps);
    lv_label_set_text(gps, "-");
    lv_obj_set_style_text_color(gps, lv_color_hex(0xfb8b05), LV_PART_MAIN);

    battery = lv_label_create(scr);
    lv_obj_set_width(battery, LV_SIZE_CONTENT);
    lv_obj_set_height(battery, LV_SIZE_CONTENT);
    lv_obj_align(battery, LV_ALIGN_TOP_RIGHT, -50, 20);
    lv_obj_set_style_text_color(battery, lv_color_hex(0x1ba784), LV_PART_MAIN);

    time_obj = lv_label_create(scr);
    lv_obj_set_width(time_obj, LV_SIZE_CONTENT);
    lv_obj_set_height(time_obj, LV_SIZE_CONTENT);
    lv_obj_align(time_obj, LV_ALIGN_TOP_LEFT, 5, 40);
    lv_obj_set_style_text_color(time_obj, lv_color_hex(0x1ba784), LV_PART_MAIN);

    env = lv_label_create(scr);
    lv_obj_set_width(env, LV_SIZE_CONTENT);
    lv_obj_set_height(env, LV_SIZE_CONTENT);
    lv_obj_align(env, LV_ALIGN_BOTTOM_LEFT, 10, -50);
    lv_obj_set_style_text_color(env, lv_color_hex(0x1ba784), LV_PART_MAIN);

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);

    bus_register_subscribe(
        DATA_BUS_BATTERY_EVENT | DATA_BUS_ENV_REFRESH | DATA_BUS_GPS_REFRESH,
        bus_event_cb, NULL);
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

    printf("====>>Initialize Interface");

    printf("=====>>>> initialize lvgl....\n");
    lv_init();
    hal_init(240, 240);

    printf("=====>>>> launch ui pages\n");
    // ui_init_and_start();
    // lv_demo_widgets();
    // lv_demo_widgets_start_slideshow();

    test_ui();
    interface_initialize();

    while (1) {
        lv_timer_handler();
        delay(10);
    }
}

/**
 * 外部输入事件
 */
static void hal_button_indev_cb(lv_indev_t* indev, lv_indev_data_t* data) {
    data->state = LV_INDEV_STATE_RELEASED;
}

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
        indev, LV_INDEV_TYPE_NONE); /*Touch pad is a pointer-like device*/
    lv_indev_set_read_cb(indev, hal_button_indev_cb);

    lv_indev_set_display(indev, display);
    lv_display_set_default(display);

    // // 初始化屏幕的PWM调光
    pinMode(IO_PWM_PIN, OUTPUT);
    // digitalWrite(IO_PWM_PIN, 1);
    analogWrite(IO_PWM_PIN, 100);
    return display;
}
