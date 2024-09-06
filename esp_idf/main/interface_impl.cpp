#include "interface_impl.h"
#include <Arduino.h>
#include "MAX17048.h"
#include "TinyGPSPlus.h"
#include "Wire.h"
#include "constant.h"
#include "sht4x.h"

static TaskHandle_t task;

static SHT4X sht4x;
static MAX17048 soc_sensor;

static void get_sensor_thread(void* params) {
    while (!sht4x.sht4x_probe()) {
        printf("SHT sensor probing failed....\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    int32_t temperature, humidity;
    while (1) {
        int8_t ret = sht4x.sht4x_measure_blocking_read(&temperature, &humidity);
        if (ret) {
            printf(
                "measured temperature: %0.2f degreeCelsius, "
                "measured humidity: %0.2f percentRH\n",
                (temperature / 1000.0f), humidity / 1000.0f);
        } else {
            printf("EEEEEEEEEEEEE,错误\n");
        }

        // vTaskDelay(pdMS_TO_TICKS(500));

        // printf("SocVCELL: ADC=%d,Voltage=%f,Percent=%d,SOC=%f%%\n",soc_sensor.adc(),soc_sensor.voltage(),
        // soc_sensor.percent(),soc_sensor.accuratePercent());


        // Serial.print("VCELL ADC : ");
        // Serial.println(soc_sensor.adc());
        // Serial.print("VCELL V   : ");
        // Serial.println(soc_sensor.voltage());
        // Serial.print("VCELL SOC : ");
        // Serial.print(soc_sensor.percent());
        // Serial.println(" \%");
        // Serial.print("VCELL SOC : ");
        // Serial.print(soc_sensor.accuratePercent());
        // Serial.println(" \%");
        // Serial.println();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * 初始化接口数据
 */
void interface_initialize(void) {
    Wire.setPins(IO_I2C_SDA, IO_I2C_SCL);
    Wire.begin();
    Serial1.begin(9600, SERIAL_8N1, IO_GPS_RX, IO_GPS_TX);

    sht4x.begin(Wire);
    soc_sensor.attatch(Wire);

    xTaskCreate(get_sensor_thread, "Sensor", 4000, NULL, 1, &task);
}