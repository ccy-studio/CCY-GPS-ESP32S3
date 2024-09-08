#pragma once
#ifndef _SHT4X_H
#define _SHT4X_H

#include "Arduino.h"
#include "Wire.h"

class SHT4X {
    enum class REG {
        SHT4X_CMD_MEASURE_HPM = 0xFD,
        SHT4X_CMD_MEASURE_LPM = 0xE0,
        SHT4X_CMD_READ_SERIAL = 0x89,
        SHT4X_ADDRESS = 0x44
    };

    uint8_t sht4x_cmd_measure = (uint8_t)REG::SHT4X_CMD_MEASURE_HPM;
    uint16_t sht4x_cmd_measure_delay_us = 10;

   private:
    TwoWire* wire;

    int8_t sensirion_i2c_write(uint8_t* data,
                               uint16_t count,
                               bool stop = true) {
        wire->beginTransmission((uint8_t)REG::SHT4X_ADDRESS);
        wire->write(data, count);
        wire->endTransmission(stop);
        return 0;
    }

    bool sensirion_i2c_read_words(uint8_t* buffer,
                                  size_t len,
                                  bool stop = true) {
        // 检查数据指针是否有效
        if (buffer == NULL || len == 0) {
            return -1;  // 返回错误代码
        }
        static uint8_t max_buf_size = 128;
        size_t pos = 0;
        while (pos < len) {
            size_t read_len =
                ((len - pos) > max_buf_size) ? max_buf_size : (len - pos);
            bool read_stop = (pos < (len - read_len)) ? false : stop;
            if (!_read(buffer + pos, read_len, read_stop))
                return false;
            pos += read_len;
        }
        return true;
    }

    bool _read(uint8_t* buffer, size_t len, bool stop) {
        size_t recv = wire->requestFrom((uint8_t)REG::SHT4X_ADDRESS,
                                        (uint8_t)len, (uint8_t)stop);
        if (recv != len) {
            return false;
        }
        for (uint16_t i = 0; i < len; i++) {
            buffer[i] = wire->read();
        }
        return true;
    }

    uint8_t crc8(const uint8_t* data, int len) {
        /*
         *
         * CRC-8 formula from page 14 of SHT spec pdf
         *
         * Test data 0xBE, 0xEF should yield 0x92
         *
         * Initialization data 0xFF
         * Polynomial 0x31 (x8 + x5 +x4 +1)
         * Final XOR 0x00
         */

        const uint8_t POLYNOMIAL(0x31);
        uint8_t crc(0xFF);

        for (int j = len; j; --j) {
            crc ^= *data++;

            for (int i = 8; i; --i) {
                crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
            }
        }
        return crc;
    }

   public:
    void begin(TwoWire& w) { wire = &w; }

    bool sht4x_measure_blocking_read(float* temperature, float* humidity) {
        sht4x_measure();
        delay(sht4x_cmd_measure_delay_us);
        return sht4x_read(temperature, humidity);
    }

    int16_t sht4x_measure(void) {
        return sensirion_i2c_write(&sht4x_cmd_measure, 1);
    }

    bool sht4x_read(float* temperature, float* humidity) {
        uint8_t readbuffer[6];
        if (!sensirion_i2c_read_words(readbuffer, 6)) {
            return false;
        }

        if (readbuffer[2] != crc8(readbuffer, 2) ||
            readbuffer[5] != crc8(readbuffer + 3, 2)) {
            return false;
        }

        float t_ticks = (uint16_t)readbuffer[0] * 256 + (uint16_t)readbuffer[1];
        float rh_ticks =
            (uint16_t)readbuffer[3] * 256 + (uint16_t)readbuffer[4];
        *temperature = -45 + 175 * t_ticks / 65535;
        *humidity = -6 + 125 * rh_ticks / 65535;
        *humidity = min(max(*humidity, (float)0.0), (float)100.0);

        return true;
    }

    uint32_t sht4x_probe() {
        uint32_t serial;
        sht4x_read_serial(&serial);
        return serial;
    }

    void sht4x_enable_low_power_mode(uint8_t enable_low_power_mode) {
        if (enable_low_power_mode) {
            sht4x_cmd_measure = (uint8_t)REG::SHT4X_CMD_MEASURE_LPM;
            sht4x_cmd_measure_delay_us = 2;
        } else {
            sht4x_cmd_measure = (uint8_t)REG::SHT4X_CMD_MEASURE_HPM;
            sht4x_cmd_measure_delay_us = 10;
        }
    }

    void sht4x_read_serial(uint32_t* serial) {
        *serial = 0;
        uint8_t cmd = (uint8_t)REG::SHT4X_CMD_READ_SERIAL;
        uint8_t reply[6];

        sensirion_i2c_write(&cmd, 1);

        delay(10);

        if (!sensirion_i2c_read_words(reply, 6)) {
            return;
        }

        if ((crc8(reply, 2) != reply[2]) || (crc8(reply + 3, 2) != reply[5])) {
            return;
        }
        *serial = reply[0];
        *serial <<= 8;
        *serial |= reply[1];
        *serial <<= 8;
        *serial |= reply[3];
        *serial <<= 8;
        *serial |= reply[4];
    }

    uint8_t sht4x_get_configured_address(void) {
        return (uint8_t)REG::SHT4X_ADDRESS;
    }
};

#endif