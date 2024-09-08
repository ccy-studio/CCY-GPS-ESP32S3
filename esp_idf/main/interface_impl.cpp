#include "interface_impl.h"
#include <Arduino.h>
#include "MAX17048.h"
#include "TinyGPSPlus.h"
#include "Wire.h"
#include "constant.h"
#include "data_bus.h"
#include "sht4x.h"

#define GPS_RAW_DEBUG 1

static TaskHandle_t sensor_task;
static TaskHandle_t gps_task;

static SHT4X sht4x;
static MAX17048 soc_sensor;
static TinyGPSPlus gps_service;

static app_environment_t env;
static app_battery_t battery;
static app_gps_t gps_info;

static void get_sensor_thread(void* params) {
    printf("SHT sensor Number %lX\n", sht4x.sht4x_probe());
    while (1) {
        sht4x.sht4x_measure_blocking_read(&env.temp, &env.humidity);
        notify_env_refresh(&env);
        battery.level = soc_sensor.accuratePercent();
        battery.is_charge = !digitalRead(IO_BQ_CHARGE);
        notify_battery_event(&battery);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);
}

static void gps_thread(void* params) {
#if GPS_RAW_DEBUG == 1
    uint8_t* buf = (uint8_t*)calloc(sizeof(uint8_t) * 50, MALLOC_CAP_DMA);
    uint8_t* start = buf;
#endif
    bool ok;
    while (1) {
#if GPS_RAW_DEBUG == 1
        buf = start;
        memset(buf, 0, sizeof(sizeof(uint8_t) * 50));
#endif
        ok = false;
        vTaskDelay(1);
        while (Serial1.available() > 0) {
            int b = Serial1.read();
            gps_service.encode(b);
            ok = true;
#if GPS_RAW_DEBUG == 1
            *buf = b;
            buf++;
#endif
        }
#if GPS_RAW_DEBUG == 1
        buf = start;
        printf((char*)buf);
#endif
        if (gps_service.location.isUpdated()) {
            gps_info.latitude = gps_service.location.lat();
            gps_info.longitude = gps_service.location.lng();
        }
        if (gps_service.speed.isUpdated()) {
            gps_info.speed = gps_service.speed.kmph();
        }
        if (gps_service.altitude.isUpdated()) {
            // 海拔高度
            gps_info.altitude = gps_service.altitude.meters();
        }
        if (gps_service.time.isUpdated()) {
            gps_info.datetime.year = gps_service.date.year();
            gps_info.datetime.month = gps_service.date.month();
            gps_info.datetime.day = gps_service.date.day();
            gps_info.datetime.hour = gps_service.time.hour();
            gps_info.datetime.minute = gps_service.time.minute();
            gps_info.datetime.second = gps_service.time.second();
        }
        if (gps_service.satellites.isUpdated()) {
            gps_info.sats_in_use = gps_service.satellites.value();
        }
        if (ok) {
            notify_gps_refresh(&gps_info);
        }
    }
    vTaskDelete(NULL);
}

/**
 * 初始化接口数据
 */
void interface_initialize(void) {
    pinMode(IO_BQ_CHARGE, INPUT);
    Wire.setPins(IO_I2C_SDA, IO_I2C_SCL);
    Wire.begin();
    Serial1.begin(9600, SERIAL_8N1, IO_GPS_RX, IO_GPS_TX);

    sht4x.begin(Wire);
    soc_sensor.attatch(Wire);

    xTaskCreate(get_sensor_thread, "Sensor", 4000, NULL, 2, &sensor_task);
    xTaskCreate(gps_thread, "gps_service", 4000, NULL, 1, &gps_task);
}