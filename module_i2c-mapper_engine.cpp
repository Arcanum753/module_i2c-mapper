#include "core_web/FSWebServerLib.h"

#include "module_i2c-mapper.h"

#include "core_terminal/core_terminal.h"
#include "core_terminal/ErriezSerialTerminal.h"

#include <Wire.h>

#if defined(ESP32)
#include <esp_task_wdt.h>
#endif

// ============================================================
// Конкретная логика модуля
// ============================================================
uint8_t CLASS_MODULE_I2C_MAPPER::scanBus(uint8_t *found, uint8_t maxCount) {
#if defined(ESP32)
    int sda = I2C_MAPPER_SDA;
    int scl = I2C_MAPPER_SCL;
    Wire.begin(sda, scl);
#endif
#if defined(ESP8266)
    Wire.begin();
#endif

    uint8_t count = 0;
    for (uint8_t addr = 0x01; addr < 0x7F && count < maxCount; addr++) {
#if defined(ESP32)
        esp_task_wdt_reset();
#endif
#if defined(ESP8266)
        ESP.wdtFeed();
#endif

        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            if (count < maxCount) { found[count++] = addr; }
        }
    }
    return count;
}

void i2cMapperCmdScan() {
    uint8_t found[126];
    uint8_t count = module_i2c_mapper.scanBus(found, sizeof(found));

    Serial.printf("\r\nI2C Bus Map (0x01-0x7E)\r\n");
    Serial.printf("    0 1 2 3 4 5 6 7 8 9 a b c d e f\r\n");
    for (uint8_t r = 0; r < 8; r++) {
        Serial.printf("%02X: ", r * 0x10);
        for (uint8_t c = 0; c < 16; c++) {
            uint8_t addr = (r << 4) | c;
            bool foundAddr = false;
            for (uint8_t i = 0; i < count; i++) {
                if (found[i] == addr) { foundAddr = true; break; }
            }
            if (foundAddr) {
                Serial.printf("%02X ", addr);      // полный адрес на перекрестье строки и столбца
            } else {
                Serial.printf("-- ");
            }
        }
        Serial.printf("\r\n");
    }

    if (count == 0) {
        Serial.printf("No devices found.\r\n");
    } else {
        Serial.printf("Found %d device(s):", count);
        for (uint8_t i = 0; i < count; i++) { Serial.printf(" 0x%02X", found[i]); }
        Serial.printf("\r\n");
    }
}
