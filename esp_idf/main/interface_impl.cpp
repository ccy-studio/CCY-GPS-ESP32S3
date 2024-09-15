#include "interface_impl.h"
#include <Arduino.h>
#include "MAX17048.h"
#include "TinyGPSPlus.h"
#include "Wire.h"
#include "app_service.h"
#include "constant.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "lvgl.h"
#include "sht4x.h"

#define GPS_TAG "GPS_INT"

#define TIME_ZONE (+8)    // Beijing Time
#define YEAR_BASE (2000)  // date in GPS starts from 2000

static TaskHandle_t sensor_task;
static TaskHandle_t gps_task;
static QueueHandle_t event_gps_queue;

static SHT4X sht4x;
static MAX17048 soc_sensor;
static TinyGPSPlus gps_service;

static void get_sensor_thread(void* params) {
    printf("SHT sensor Number %lX\n", sht4x.sht4x_probe());
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        sht4x.sht4x_measure_blocking_read(&global_env.temp,
                                          &global_env.humidity);
        global_battery.level = soc_sensor.accuratePercent();
        global_battery.is_charge = !digitalRead(IO_BQ_CHARGE);
        // printf("Temp: %.1f,Hum:%.1f\n", global_env.temp,
        // global_env.humidity);
        lv_lock();
        notify_env_refresh(&global_env);
        notify_battery_event(&global_battery);
        lv_unlock();
    }
    vTaskDelete(NULL);
}

static void decode_gps(uint8_t* buffer) {
    static uint32_t last_time = micros();
    int pos = uart_pattern_pop_pos(UART_NUM_1);
    if (pos != -1) {
        bool isUpdate = false;
        int read_len = uart_read_bytes(UART_NUM_1, buffer, pos + 1,
                                       100 / portTICK_PERIOD_MS);
        buffer[read_len] = '\0';
        for (size_t i = 0; i < read_len + 1; i++) {
            gps_service.encode(buffer[i]);
        }
        // printf("%s", buffer);

        if (gps_service.location.isUpdated()) {
            global_gps.latitude = gps_service.location.lat();
            global_gps.longitude = gps_service.location.lng();
            isUpdate = true;
        }
        if (gps_service.speed.isUpdated()) {
            global_gps.speed = gps_service.speed.mps();
            isUpdate = true;
        }
        if (gps_service.altitude.isUpdated()) {
            // 海拔高度
            global_gps.altitude = gps_service.altitude.meters();
            isUpdate = true;
        }
        if (gps_service.time.isUpdated() || gps_service.date.isUpdated()) {
            global_gps.datetime.year = gps_service.date.year() + YEAR_BASE;
            global_gps.datetime.month = gps_service.date.month();
            global_gps.datetime.day = gps_service.date.day();
            global_gps.datetime.hour = gps_service.time.hour() + TIME_ZONE;
            global_gps.datetime.minute = gps_service.time.minute();
            global_gps.datetime.second = gps_service.time.second();

            isUpdate = true;
        }
        if (gps_service.satellites.isUpdated()) {
            global_gps.sats_in_use = gps_service.satellites.value();
            isUpdate = true;
        }

        // 设定xxx毫秒进行更新一次
        uint32_t diff = micros() - last_time;
        if (isUpdate && diff > 400000) {
            lv_lock();
            notify_gps_refresh(&global_gps);
            // bool up_tim =
            //     (global_real_record.curr_gps.latitude != global_gps.latitude)
            //     || (global_real_record.curr_gps.longitude !=
            //     global_gps.longitude);
            notify_data_change(false);
            lv_unlock();
            last_time = micros();
        }
    } else {
        ESP_LOGW(GPS_TAG, "Pattern Queue Size too small");
        uart_flush_input(UART_NUM_1);
    }
}

static void gps_thread(void* params) {
    uint8_t* buffer = (uint8_t*)heap_caps_calloc(1, 512, MALLOC_CAP_SPIRAM);
    if (buffer == NULL) {
        ESP_LOGE(GPS_TAG, "GPS memory allocation failed");
        vTaskDelete(NULL);
        return;
    }
    uart_event_t event;
    while (1) {
        vTaskDelay(1);
        if (xQueueReceive(event_gps_queue, &event, pdMS_TO_TICKS(200))) {
            switch (event.type) {
                case UART_DATA:
                    break;
                case UART_FIFO_OVF:
                    ESP_LOGW(GPS_TAG, "HW FIFO Overflow");
                    uart_flush(UART_NUM_1);
                    xQueueReset(event_gps_queue);
                    break;
                case UART_BUFFER_FULL:
                    ESP_LOGW(GPS_TAG, "Ring Buffer Full");
                    uart_flush(UART_NUM_1);
                    xQueueReset(event_gps_queue);
                    break;
                case UART_BREAK:
                    ESP_LOGW(GPS_TAG, "Rx Break");
                    break;
                case UART_PARITY_ERR:
                    ESP_LOGE(GPS_TAG, "Parity Error");
                    break;
                case UART_FRAME_ERR:
                    ESP_LOGE(GPS_TAG, "Frame Error");
                    break;
                case UART_PATTERN_DET:
                    decode_gps(buffer);
                    break;
                default:
                    ESP_LOGW(GPS_TAG, "unknown uart event type: %d",
                             event.type);
                    break;
            }
        }
    }
    vTaskDelete(NULL);
}

/**
 * 监听关机的事件
 */
static void on_power_close(bus_msg_t msg) {
    ESP_LOGI("SYS", "Power Close......");
    digitalWrite(IO_POWER_EN_PIN, LOW);
}

/**
 * 监听背光事件
 */
static void on_bl_set_cb(bus_msg_t msg) {
    app_bl_pwm* bl = (app_bl_pwm*)msg.payload;
    analogWrite(IO_PWM_PIN, *bl);
}

/**
 * 初始化接口数据
 */
void interface_initialize(void) {
    pinMode(IO_BQ_CHARGE, INPUT);
    Wire.setPins(IO_I2C_SDA, IO_I2C_SCL);
    Wire.begin();
    // Serial1.begin(9600, SERIAL_8N1, IO_GPS_RX, IO_GPS_TX);

    // espidf串口初始化
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    if (uart_driver_install(UART_NUM_1, 1024, 0, 16, &event_gps_queue, 0) !=
        ESP_OK) {
        ESP_LOGE(GPS_TAG, "install uart driver failed");
    }

    if (uart_param_config(UART_NUM_1, &uart_config) != ESP_OK) {
        ESP_LOGE(GPS_TAG, "config uart parameter failed");
    }
    if (uart_set_pin(UART_NUM_1, UART_PIN_NO_CHANGE, IO_GPS_RX,
                     UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
        ESP_LOGE(GPS_TAG, "config uart gpio failed");
    }
    uart_enable_pattern_det_baud_intr(UART_NUM_1, '\n', 1, 9, 0, 0);
    uart_pattern_queue_reset(UART_NUM_1, 16);
    uart_flush(UART_NUM_1);

    sht4x.begin(Wire);
    soc_sensor.attatch(Wire);

    xTaskCreate(get_sensor_thread, "Sensor", 6000, NULL, 2, &sensor_task);
    xTaskCreate(gps_thread, "GSP", 6000, NULL, 1, &gps_task);

    bus_register_subscribe(DATA_BUS_POWER_EVENT, on_power_close, NULL);
    bus_register_subscribe(DATA_BUS_BL_EVENT, on_bl_set_cb, NULL);
}