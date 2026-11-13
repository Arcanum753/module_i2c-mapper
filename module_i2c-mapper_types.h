#ifndef _MODULE_I2C_MAPPER_TYPES_h
#define _MODULE_I2C_MAPPER_TYPES_h

// ============================================================
// module_i2c-mapper_types.h — define'ы модуля сканера I2C.
// Реализация: module_i2c-mapper.cpp (шаблон) и
// module_i2c-mapper_engine.cpp (scanBus).
// ============================================================

#include <Arduino.h>
#include <stdint.h>

#if defined(ESP32)
#define I2C_MAPPER_SDA  21
#define I2C_MAPPER_SCL  22
#endif

#if defined(ESP8266)
#define I2C_MAPPER_SDA  4
#define I2C_MAPPER_SCL  5
#endif

// Диапазон сканирования и размер кэша последнего результата.
#define I2C_MAPPER_ADDR_MIN    0x01
#define I2C_MAPPER_ADDR_MAX    0x7E
#define I2C_MAPPER_MAX_FOUND   126   // 0x01..0x7E

#endif // _MODULE_I2C_MAPPER_TYPES_h
