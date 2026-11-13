#include "core_web/FSWebServerLib.h"

#include "module_i2c-mapper.h"

#include <Wire.h>

#if defined(ESP32)
#include <esp_task_wdt.h>
#endif

// ============================================================
// Конкретная логика модуля
// ============================================================

// Инициализация шины: параметры фиксированы define'ами модуля,
// повторный вызов повторяет поведение прежнего web-скана.
void CLASS_MODULE_I2C_MAPPER::busInit() {
#if defined(ESP32)
    Wire.begin(I2C_MAPPER_SDA, I2C_MAPPER_SCL);
#endif
#if defined(ESP8266)
    Wire.begin();
#endif
    _wireReady = true;
}

uint8_t CLASS_MODULE_I2C_MAPPER::scanBus() {
    busInit();

    uint8_t count = 0;
    for (uint8_t addr = I2C_MAPPER_ADDR_MIN; addr <= I2C_MAPPER_ADDR_MAX; addr++) {
#if defined(ESP32)
        esp_task_wdt_reset();
#endif
#if defined(ESP8266)
        ESP.wdtFeed();
#endif

        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();

        if (error == 0 && count < I2C_MAPPER_MAX_FOUND) {
            _lastAddrs[count++] = addr;
        }
    }

    _lastCount = count;
    _scanDone = true;
    return count;
}

bool CLASS_MODULE_I2C_MAPPER::probeBus(uint8_t addr) {
    busInit();

#if defined(ESP32)
    esp_task_wdt_reset();
#endif
#if defined(ESP8266)
    ESP.wdtFeed();
#endif

    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}
