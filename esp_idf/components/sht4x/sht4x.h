#pragma once
#ifndef _SHT4X_H
#define _SHT4X_H

#include "Arduino.h"
#include "Wire.h"

#define SENSIRION_WORD_SIZE 2
#define SENSIRION_NUM_WORDS(x) (sizeof(x) / SENSIRION_WORD_SIZE)

class SHT4X {
    enum class REG {
        SHT4X_CMD_MEASURE_HPM = 0xFD,
        SHT4X_CMD_MEASURE_LPM = 0xE0,
        SHT4X_CMD_READ_SERIAL = 0x89,
        SHT4X_CMD_DURATION_USEC = 1000,
        SHT4X_ADDRESS = 0x44,
        SHT4X_MEASUREMENT_DURATION_USEC = 10000,
        SHT4X_MEASUREMENT_DURATION_LPM_USEC = 2500
    };

    uint8_t sht4x_cmd_measure = (uint8_t)REG::SHT4X_CMD_MEASURE_HPM;
    uint16_t sht4x_cmd_measure_delay_us =
        (uint16_t)REG::SHT4X_MEASUREMENT_DURATION_USEC;

   private:
    TwoWire* wire;
    int8_t sensirion_i2c_read(uint8_t address, uint8_t* data, uint16_t count) {
        uint8_t readData[count];
        uint8_t rxByteCount = 0;

        // 2 bytes RH, 1 CRC, 2 bytes T, 1 CRC
        wire->requestFrom(address, count);

        while (wire->available()) {  // wait till all arrive
            readData[rxByteCount++] = wire->read();
            if (rxByteCount >= count)
                break;
        }

        memcpy(data, readData, count);

        return 0;
    }

    int8_t sensirion_i2c_write(uint8_t address, uint8_t* data, uint16_t count) {
        wire->beginTransmission(address);
        wire->write(data, count);
        wire->endTransmission(1);
        return 0;
    }

    void sensirion_sleep_usec(uint32_t useconds) {
        delay((useconds / 1000) + 1);
    }

    int16_t sensirion_i2c_read_words(uint8_t address,
                                     uint16_t* data_words,
                                     uint16_t num_words) {
        // 检查数据指针是否有效
        if (data_words == NULL || num_words == 0) {
            return -1;  // 返回错误代码
        }

        // 请求读取 num_words * 2 字节的数据
        wire->beginTransmission(address);
        wire->endTransmission();  // 结束传输以准备读取

        // 请求数据
        wire->requestFrom(address, num_words * 2);  // 每个 uint16_t 需要 2 字节

        // 检查接收到的数据字节数
        if (wire->available() < num_words * 2) {
            return -2;  // 返回错误代码，表示接收到的数据不足
        }

        // 读取数据
        for (uint16_t i = 0; i < num_words; i++) {
            // 读取低字节
            uint8_t lowByte = wire->read();
            // 读取高字节
            uint8_t highByte = wire->read();

            // 合并低字节和高字节
            data_words[i] =
                (highByte << 8) | lowByte;  // 高字节在前，低字节在后
        }

        return num_words;  // 返回成功读取的字数
    }

   public:
    void begin(TwoWire& w) { wire = &w; }

    int16_t sht4x_measure_blocking_read(int32_t* temperature,
                                        int32_t* humidity) {
        int16_t ret;

        ret = sht4x_measure();
        if (ret)
            return ret;
        // sensirion_sleep_usec(sht4x_cmd_measure_delay_us);
        delay(10);
        return sht4x_read(temperature, humidity);
    }

    int16_t sht4x_measure(void) {
        uint8_t buf[1] = {sht4x_cmd_measure};
        return sensirion_i2c_write((uint8_t)REG::SHT4X_ADDRESS, buf, 1);
    }

    int16_t sht4x_read(int32_t* temperature, int32_t* humidity) {
        uint16_t words[2];
        int16_t ret = sensirion_i2c_read_words(
            (uint8_t)REG::SHT4X_ADDRESS, words, SENSIRION_NUM_WORDS(words));

        /**
         * formulas for conversion of the sensor signals, optimized for fixed
         * point algebra: Temperature = 175 * S_T / 65535 - 45 Relative
         Humidity
         * = 125 * (S_RH / 65535) - 6
         */
        *temperature = ((21875 * (int32_t)words[0]) >> 13) - 45000;
        *humidity = ((15625 * (int32_t)words[1]) >> 13) - 6000;

        return ret;
    }

    int16_t sht4x_probe() {
        uint32_t serial;
        uint16_t ret = sht4x_read_serial(&serial);
        printf("SHT4X Serial = %lX\n", serial);
        return ret;
    }

    void sht4x_enable_low_power_mode(uint8_t enable_low_power_mode) {
        if (enable_low_power_mode) {
            sht4x_cmd_measure = (uint8_t)REG::SHT4X_CMD_MEASURE_LPM;
            sht4x_cmd_measure_delay_us =
                (uint16_t)REG::SHT4X_MEASUREMENT_DURATION_LPM_USEC;
        } else {
            sht4x_cmd_measure = (uint8_t)REG::SHT4X_CMD_MEASURE_HPM;
            sht4x_cmd_measure_delay_us =
                (uint16_t)REG::SHT4X_MEASUREMENT_DURATION_USEC;
        }
    }

    int16_t sht4x_read_serial(uint32_t* serial) {
        uint8_t cmd[1] = {(uint8_t)REG::SHT4X_CMD_READ_SERIAL};
        int16_t ret;
        uint16_t serial_words[SENSIRION_NUM_WORDS(*serial)];

        ret = sensirion_i2c_write((uint8_t)REG::SHT4X_ADDRESS, cmd, 1);
        if (ret)
            return ret;

        sensirion_sleep_usec((uint8_t)REG::SHT4X_CMD_DURATION_USEC);
        ret =
            sensirion_i2c_read_words((uint8_t)REG::SHT4X_ADDRESS, serial_words,
                                     SENSIRION_NUM_WORDS(serial_words));
        *serial = ((uint32_t)serial_words[0] << 16) | serial_words[1];

        return ret;
    }

    uint8_t sht4x_get_configured_address(void) {
        return (uint8_t)REG::SHT4X_ADDRESS;
    }
};

#endif