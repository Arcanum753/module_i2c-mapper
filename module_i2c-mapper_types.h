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

#endif // _MODULE_I2C_MAPPER_TYPES_h
