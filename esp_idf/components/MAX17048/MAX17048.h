#pragma once
#ifndef MAX17048_H
#define MAX17048_H

#ifdef TEENSYDUINO
#include <i2c_t3.h>
#else
#include <Wire.h>
#endif

namespace EmbeddedDevices {
template <int CELL>
class MAX17048 {
    const uint8_t I2C_ADDR = 0x36;

    enum class REG {
        VCELL = 0x02,
        SOC = 0x04,
        MODE = 0x06,
        VERSION = 0x08,
        HIBRT = 0x0A,
        CONFIG = 0x0C,
        VALRT = 0x14,
        CRATE = 0x16,
        VRESET_ID = 0x18,
        STATUS = 0x1A,
        TABLE = 0x40,
        CMD = 0xFE
    };

    enum class ALERT {
        RI = (1 << 0),  // Reset indicator
        VH = (1 << 1),  // Voltage high alert
        VL = (1 << 2),  // Voltage low alert
        VR = (1 << 3),  // Voltage reset alert
        HD = (1 << 4),  // SOC low alert
        SC = (1 << 5)   // SOC change alert
    };

   public:
    void attatch(TwoWire& w) { wire = &w; }

    uint16_t adc() { return read(REG::VCELL); }
    float voltage() {
        return (float)read(REG::VCELL) * 78.125f * (float)CELL / 1000000.f;
    }

    uint8_t percent() { return (uint8_t)(read(REG::SOC) / 256); }
    float accuratePercent() { return (float)read(REG::SOC) / 256.f; }

    void quickStart() {
        uint16_t v = read(REG::MODE);
        bitSet(v, 14);
        write(REG::MODE, v);
    }

    void enSleep(bool b) {
        uint16_t v = read(REG::MODE);
        bitWrite(v, 13, b);
        write(REG::MODE, v);
    }
    bool isSleepable() { return bitRead(read(REG::MODE), 13); }

    uint16_t mode() { return read(REG::MODE); }

    uint8_t version() { return (uint8_t)read(REG::VERSION); }

    bool isHibernating() { return bitRead(read(REG::MODE), 12); }
    float hibernateActTh() {
        return (float)highByte(read(REG::HIBRT)) * 0.00125;
    }
    float hibernateHibTh() { return (float)lowByte(read(REG::HIBRT)) * 0.208; }
    uint8_t hibernateActTh(float th) {
        uint16_t v = read(REG::HIBRT) & 0xFF00;
        if (th > 0.0) {
            if (th < 0.31875)
                v |= (uint16_t)(th / 0.00125) & 0x00FF;
            else
                v |= 0x00FF;
        }
        write(REG::HIBRT, v);
        return 0;
    }
    uint8_t hibernateHibTh(float th) {
        uint16_t v = read(REG::HIBRT) & 0x00FF;
        if (th > 0.0) {
            if (th < 53.04)
                v |= (uint16_t)(th / 0.208) << 8;
            else
                v |= 0xFF00;
        }
        write(REG::HIBRT, v);
        return 0;
    }

    void tempCompensate(float temp) {
        uint8_t v = 0;
        if (temp > 20.0)
            v = 0x97 + (temp - 20.0) * -0.5;
        else
            v = 0x97 + (temp - 20.0) * -5.0;
        rcomp(v);
    }

    void rcomp(uint8_t rcomp) {
        uint16_t v = (read(REG::CONFIG) & 0x00FF) | (rcomp << 8);
        write(REG::CONFIG, v);
    }

    void sleep(bool b) {
        uint16_t v = read(REG::CONFIG);
        bitWrite(v, 7, b);
        write(REG::CONFIG, v);
    }

    // alert functions are ported & modified from
    // https://github.com/mjbcopland/MAX17048

    bool isAlerting() { return bitRead(read(REG::CONFIG), 5); }

    void clearAlert() {
        uint16_t v = read(REG::CONFIG);
        bitClear(v, 5);
        write(REG::CONFIG, v);
    }

    uint8_t emptyAlertThreshold() { return 32 - (read(REG::CONFIG) & 0x001F); }
    void emptyAlertThreshold(uint8_t th) {
        uint16_t v = read(REG::CONFIG);
        th = constrain(th, 1, 32);
        v &= 0xFFE0;
        v |= 32 - th;
        write(REG::CONFIG, v);
    }

    float vAlertMinThreshold() { return highByte(read(REG::VALRT)) * 0.02; }
    void vAlertMinThreshold(float th) {
        uint16_t v = read(REG::VALRT) & 0x00FF;
        if (th > 0.0) {
            if (th < 5.1)
                v |= (uint16_t)(th / 0.02) << 8;
            else
                v |= 0xFF00;
        }
        write(REG::VALRT, v);
    }

    float vAlertMaxThreshold() { return (read(REG::VALRT) & 0x00FF) * 0.02; }
    void vAlertMaxThreshold(float th) {
        uint16_t v = read(REG::VALRT) & 0xFF00;
        if (th > 0.0) {
            if (th < 5.1)
                v |= (uint8_t)(th / 0.02);
            else
                v |= 0x00FF;
        }
        write(REG::VALRT, v);
    }

    float vResetThreshold() { return read(REG::VRESET_ID >> 9) * 0.04; }
    void vResetThreshold(float th) {
        uint16_t v = read(REG::VRESET_ID) & 0x01FF;
        if (th > 0.0) {
            if (th < 5.08)
                v |= (uint16_t)(th / 0.04) << 9;
            else
                v |= 0xFE00;
        }
        write(REG::VRESET_ID, v);
    }

    bool comparatorEnabled() { return bitRead(read(REG::VRESET_ID), 8); }
    void comparatorEnabled(bool b) {
        uint16_t v = read(REG::VRESET_ID);
        bitWrite(v, 8, b);
        write(REG::VRESET_ID, v);
    }

    uint8_t id() { return lowByte(read(REG::VRESET_ID)); }

    bool vResetAlertEnabled() { return bitRead(read(REG::STATUS), 14); }
    void vResetAlertEnabled(bool b) {
        uint16_t v = read(REG::STATUS);
        bitWrite(v, 14, b);
        write(REG::STATUS, v);
    }

    uint8_t alertFlags() { return highByte(read(REG::STATUS)) & 0x3F; }
    void clearAlert(uint8_t flags) {
        uint16_t v = read(REG::STATUS);
        v &= ~((flags & 0x3F) << 8);
        write(REG::STATUS, v);
    }

    float crate() { return (float)read(REG::CRATE) * 0.208f; }  // % per hour

    uint8_t status() { return read(REG::STATUS); }
    bool highVoltage() { return bitRead(alertFlags(), 1); }
    bool lowVoltage() { return bitRead(alertFlags(), 2); }
    bool resetVoltage() { return bitRead(alertFlags(), 3); }
    bool lowSOC() { return bitRead(alertFlags(), 4); }
    bool chnageSOC() { return bitRead(alertFlags(), 5); }

    void reset() { write(REG::CMD, 0x5400); }

   private:
    void write(const REG reg, const bool stop = true) {
        wire->beginTransmission(I2C_ADDR);
        wire->write((uint8_t)reg);
        wire->endTransmission(stop);
    }

    void write(const REG reg, const uint16_t data, const bool stop = true) {
        wire->beginTransmission(I2C_ADDR);
        wire->write((uint8_t)reg);
        wire->write((data & 0xFF00) >> 8);
        wire->write((data & 0x00FF) >> 0);
        wire->endTransmission(stop);
    }

    uint16_t read(const REG reg) {
        write(reg, false);
        wire->requestFrom((uint8_t)I2C_ADDR, (uint8_t)2);  // 2byte R/W only
        uint16_t data = (uint16_t)((wire->read() << 8) & 0xFF00);
        data |= (uint16_t)(wire->read() & 0x00FF);
        return data;
    }

    bool b_quick_start{false};
    bool b_sleep{false};

    TwoWire* wire;
};
}  // namespace EmbeddedDevices

using MAX17048 = EmbeddedDevices::MAX17048<1>;
using MAX17049 = EmbeddedDevices::MAX17048<2>;

#endif  // MAX17048_H
